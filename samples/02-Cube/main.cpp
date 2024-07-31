#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>
#include <sdl2webgpu.h>

#ifdef WEBGPU_BACKEND_WGPU
#include <webgpu/wgpu.h> // Include non-standard functions.
#endif

#include <Timer.hpp>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // For matrix transformations.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <map>

std::map<WGPUFeatureName, std::string> featureNames = {
    {WGPUFeatureName_DepthClipControl, "DepthClipControl"},
    {WGPUFeatureName_Depth32FloatStencil8, "Depth32FloatStencil8"},
    {WGPUFeatureName_TimestampQuery, "TimestampQuery"},
    {WGPUFeatureName_TextureCompressionBC, "TextureCompressionBC"},
    {WGPUFeatureName_TextureCompressionETC2, "TextureCompressionETC2"},
    {WGPUFeatureName_TextureCompressionASTC, "TextureCompressionASTC"},
    {WGPUFeatureName_IndirectFirstInstance, "IndirectFirstInstance"},
    {WGPUFeatureName_ShaderF16, "ShaderF16"},
    {WGPUFeatureName_RG11B10UfloatRenderable, "RG11B10UfloatRenderable"},
    {WGPUFeatureName_BGRA8UnormStorage, "BGRA8UnormStorage"},
    {WGPUFeatureName_Float32Filterable, "Float32Filterable"},
};

std::map<WGPUQueueWorkDoneStatus, std::string> queueWorkDoneStatusNames = {
    {WGPUQueueWorkDoneStatus_Success, "Success"},
    {WGPUQueueWorkDoneStatus_Error, "Error"},
    {WGPUQueueWorkDoneStatus_Unknown, "Unknown"},
    {WGPUQueueWorkDoneStatus_DeviceLost, "DeviceLost"},
};

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char* WINDOW_TITLE = "Cube";

const char* SHADER_MODULE = {
#include "shader.wgsl"
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

static Vertex vertices[8] = {
    { {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} },  // 0
    { {-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },   // 1
    { {1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f} },    // 2
    { {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} },   // 3
    { {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },   // 4
    { {-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} },    // 5
    { {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} },     // 6
    { {1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} }     // 7
};

static uint16_t indices[36] = {
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

SDL_Window* window = nullptr;

WGPUInstance instance = nullptr;
WGPUDevice device = nullptr;
WGPUQueue queue = nullptr;
WGPUSurface surface = nullptr;
WGPUTexture depthTexture = nullptr;
WGPUTextureView depthTextureView = nullptr;
WGPUSurfaceConfiguration surfaceConfiguration{};
WGPURenderPipeline pipeline = nullptr;
WGPUBindGroupLayout bindGroupLayout = nullptr;
WGPUBuffer vertexBuffer = nullptr;
WGPUBuffer indexBuffer = nullptr;
WGPUBuffer mvpBuffer = nullptr;

Timer timer;
bool isRunning = true;

WGPUAdapter requestAdapter(const WGPURequestAdapterOptions* options)
{
    // Used by the requestAdapterCallback function to store the adapter and to notify
    // us when the request is complete.
    struct UserData
    {
        WGPUAdapter adapter = nullptr;
        bool done = false;
    } userData;

    // Callback function that is called by wgpuInstanceRequestAdapter when the request is complete.
    auto requestAdapterCallback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userData)
        {
            auto& data = *static_cast<UserData*>(userData);
            if (status == WGPURequestAdapterStatus_Success)
            {
                data.adapter = adapter;
            }
            else
            {
                std::cerr << "Failed to request adapter: " << message << std::endl;
            }
            data.done = true;
        };

    wgpuInstanceRequestAdapter(instance, options, requestAdapterCallback, &userData);

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while (!userData.done)
    {
        emscripten_sleep(100);
    }
#endif

    // The request must complete.
    assert(userData.done);

    return userData.adapter;
}

void inspectAdapter(WGPUAdapter adapter)
{
    // List the adapter properties.
    WGPUAdapterProperties adapterProperties{};
    wgpuAdapterGetProperties(adapter, &adapterProperties);

    std::cout << "Adapter name: " << adapterProperties.name << std::endl;
    std::cout << "Adapter vendor: " << adapterProperties.vendorName << std::endl;

    // List adapter limits.
    WGPUSupportedLimits supportedLimits{};
    if (wgpuAdapterGetLimits(adapter, &supportedLimits))
    {
        WGPULimits limits = supportedLimits.limits;
        std::cout << "Limits: " << std::endl;
        std::cout << "  maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
        std::cout << "  maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
        std::cout << "  maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
        std::cout << "  maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
        std::cout << "  maxBindGroups: " << limits.maxBindGroups << std::endl;
        std::cout << "  maxBindGroupsPlusVertexBuffers: " << limits.maxBindGroupsPlusVertexBuffers << std::endl;
        std::cout << "  maxBindingsPerBindGroup: " << limits.maxBindingsPerBindGroup << std::endl;
        std::cout << "  maxDynamicUniformBuffersPerPipelineLayout: " << limits.maxDynamicUniformBuffersPerPipelineLayout << std::endl;
        std::cout << "  maxDynamicStorageBuffersPerPipelineLayout: " << limits.maxDynamicStorageBuffersPerPipelineLayout << std::endl;
        std::cout << "  maxSampledTexturesPerShaderStage: " << limits.maxSampledTexturesPerShaderStage << std::endl;
        std::cout << "  maxSamplersPerShaderStage: " << limits.maxSamplersPerShaderStage << std::endl;
        std::cout << "  maxStorageBuffersPerShaderStage: " << limits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << "  maxStorageTexturesPerShaderStage: " << limits.maxStorageTexturesPerShaderStage << std::endl;
        std::cout << "  maxUniformBuffersPerShaderStage: " << limits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << "  maxUniformBufferBindingSize: " << limits.maxUniformBufferBindingSize << std::endl;
        std::cout << "  maxStorageBufferBindingSize: " << limits.maxStorageBufferBindingSize << std::endl;
        std::cout << "  minUniformBufferOffsetAlignment: " << limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << "  minStorageBufferOffsetAlignment: " << limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << "  maxVertexBuffers: " << limits.maxVertexBuffers << std::endl;
        std::cout << "  maxBufferSize: " << limits.maxBufferSize << std::endl;
        std::cout << "  maxVertexAttributes: " << limits.maxVertexAttributes << std::endl;
        std::cout << "  maxVertexBufferArrayStride: " << limits.maxVertexBufferArrayStride << std::endl;
        std::cout << "  maxInterStageShaderComponents: " << limits.maxInterStageShaderComponents << std::endl;
        std::cout << "  maxInterStageShaderVariables: " << limits.maxInterStageShaderVariables << std::endl;
        std::cout << "  maxComputeWorkgroupStorageSize: " << limits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << "  maxComputeInvocationsPerWorkgroup: " << limits.maxComputeInvocationsPerWorkgroup << std::endl;
        std::cout << "  maxComputeWorkgroupSizeX: " << limits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << "  maxComputeWorkgroupSizeY: " << limits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << "  maxComputeWorkgroupSizeZ: " << limits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << "  maxComputeWorkgroupsPerDimension: " << limits.maxComputeWorkgroupsPerDimension << std::endl;
    }

    // List the adapter features.
    std::vector<WGPUFeatureName> features;

    // Query the number of features.
    size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

    // Allocate memory to store the resulting features.
    features.resize(featureCount);

    // Enumerate again, now with the allocated to store the features.
    wgpuAdapterEnumerateFeatures(adapter, features.data());

    std::cout << "Adapter features: " << std::endl;
    for (auto feature : features)
    {
        // Print features in hexadecimal format to make it easier to compare with the WebGPU specification.
        std::cout << "  - 0x" << std::hex << feature << std::dec << ": " << featureNames[feature] << std::endl;
    }
}

WGPUDevice requestDevice(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor)
{
    struct UserData
    {
        WGPUDevice device = nullptr;
        bool done = false;
    } userData;

    auto requestDeviceCallback = [](WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* userData)
        {
            auto& data = *static_cast<UserData*>(userData);
            if (status == WGPURequestDeviceStatus_Success)
            {
                data.device = device;
            }
            else
            {
                std::cerr << "Failed to request device: " << message << std::endl;
            }
            data.done = true;
        };

    wgpuAdapterRequestDevice(adapter, descriptor, requestDeviceCallback, &userData);

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while (!userData.done)
    {
        emscripten_sleep(100);
    }
#endif

    // The request must complete.
    assert(userData.done);

    return userData.device;
}

// A callback function that is called when the GPU device is no longer available for some reason.
void onDeviceLost(WGPUDeviceLostReason reason, char const* message, void*)
{
    std::cerr << "Device lost: " << std::hex << reason << std::dec;
    if (message)
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

// A callback function that is called when we do something wrong with the device.
// For example, we run out of memory or we do something wrong with the API.
void onUncapturedErrorCallback(WGPUErrorType type, char const* message, void*)
{
    std::cerr << "Uncaptured device error: " << std::hex << type << std::dec;
    if (message)
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

// A callback function that is called when submitted work is done.
void onQueueWorkDone(WGPUQueueWorkDoneStatus status, void*)
{
//    std::cout << "Queue work done [" << std::hex << status << std::dec << "]: " << queueWorkDoneStatusNames[status] << std::endl;
}

// Poll the GPU to allow work to be done on the device queue.
void pollDevice(WGPUDevice _device, bool sleep = false)
{
#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(_device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(_device, false, nullptr);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
    if (sleep)
    {
        emscripten_sleep(100);
    }
#endif
}

// Poll the queue until all work is done.
void flushQueue()
{
    bool done = false;
    wgpuQueueOnSubmittedWorkDone(queue, [](WGPUQueueWorkDoneStatus status, void* pDone)
        {
            std::cout << "Queue work done [" << std::hex << status << std::dec << "]: " << queueWorkDoneStatusNames[status] << std::endl;
            *static_cast<bool*>(pDone) = true;
        }, &done
    );

    while (!done)
    {
        pollDevice(device, true);
    }
}

void onResize(int width, int height)
{
    if (depthTextureView)
        wgpuTextureViewRelease(depthTextureView);

    if (depthTexture)
        wgpuTextureRelease(depthTexture);

    surfaceConfiguration.width = width;
    surfaceConfiguration.height = height;
    wgpuSurfaceConfigure(surface, &surfaceConfiguration);

    // Create the depth texture.
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth32Float;

    WGPUTextureDescriptor depthTextureDescriptor{};
    depthTextureDescriptor.label = "Depth Texture";
    depthTextureDescriptor.usage = WGPUTextureUsage_RenderAttachment;
    depthTextureDescriptor.dimension = WGPUTextureDimension_2D;
    depthTextureDescriptor.size.width = width;
    depthTextureDescriptor.size.height = height;
    depthTextureDescriptor.size.depthOrArrayLayers = 1;
    depthTextureDescriptor.format = depthTextureFormat;
    depthTextureDescriptor.mipLevelCount = 1;
    depthTextureDescriptor.sampleCount = 1;
    depthTextureDescriptor.viewFormatCount = 1;
    depthTextureDescriptor.viewFormats = &depthTextureFormat;
    depthTexture = wgpuDeviceCreateTexture(device, &depthTextureDescriptor);

    // And create the depth texture view.
    WGPUTextureViewDescriptor depthTextureViewDescriptor{};
    depthTextureViewDescriptor.label = "Depth Texture View";
    depthTextureViewDescriptor.format = depthTextureFormat;
    depthTextureViewDescriptor.dimension = WGPUTextureViewDimension_2D;
    depthTextureViewDescriptor.baseMipLevel = 0;
    depthTextureViewDescriptor.mipLevelCount = 1;
    depthTextureViewDescriptor.baseArrayLayer = 0;
    depthTextureViewDescriptor.arrayLayerCount = 1;
    depthTextureViewDescriptor.aspect = WGPUTextureAspect_DepthOnly;
    depthTextureView = wgpuTextureCreateView(depthTexture, &depthTextureViewDescriptor);
}

// Initialize the application.
void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "Failed to create window." << std::endl;
        return;
    }

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    // For some reason, the instance descriptor must be null when using emscripten.
    instance = wgpuCreateInstance(nullptr);
#else
    WGPUInstanceDescriptor instanceDescriptor{};
#ifdef WEBGPU_BACKEND_DAWN
    // Make sure the uncaptured error callback is called as soon as an error
    // occurs, rather than waiting for the next Tick. This enables using the 
    // stack trace in which the uncaptured error occurred when breaking into the
    // uncaptured error callback.
    const char* enabledToggles[] = {
        "enable_immediate_error_handling"
    };
    WGPUDawnTogglesDescriptor toggles{};
    toggles.chain.next = nullptr;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount = 0;
    toggles.enabledToggleCount = std::size(enabledToggles);
    toggles.enabledToggles = enabledToggles;
    instanceDescriptor.nextInChain = &toggles.chain;
#endif
    instance = wgpuCreateInstance(&instanceDescriptor);
#endif

    if (!instance)
    {
        std::cerr << "Failed to create WebGPU instance." << std::endl;
        return;
    }

    surface = SDL_GetWGPUSurface(instance, window);

    if (!surface)
    {
        std::cerr << "Failed to get surface." << std::endl;
        return;
    }

    // Request the adapter.
    WGPURequestAdapterOptions requestAdapterOptions{};
    requestAdapterOptions.compatibleSurface = surface;
    WGPUAdapter adapter = requestAdapter(&requestAdapterOptions);

    if (!adapter)
    {
        std::cerr << "Failed to get adapter." << std::endl;
        return;
    }

    inspectAdapter(adapter);

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor{};
    deviceDescriptor.label = "LearnWebGPU"; // You can use anything here.
    deviceDescriptor.requiredFeatureCount = 0; // We don't require any extra features.
    deviceDescriptor.requiredFeatures = nullptr;
    deviceDescriptor.requiredLimits = nullptr; // We don't require any specific limits.
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.defaultQueue.label = "Default Queue"; // You can use anything here.
    deviceDescriptor.deviceLostCallback = onDeviceLost;
    device = requestDevice(adapter, &deviceDescriptor);

    if (!device)
    {
        std::cerr << "Failed to create device." << std::endl;
        return;
    }

    // Set the uncaptured error callback.
    wgpuDeviceSetUncapturedErrorCallback(device, onUncapturedErrorCallback, nullptr);

    // Get the device queue.
    queue = wgpuDeviceGetQueue(device);

    if (!queue)
    {
        std::cerr << "Failed to get device queue." << std::endl;
        return;
    }

    // Create the vertex buffer.
    WGPUBufferDescriptor vertexBufferDescriptor{};
    vertexBufferDescriptor.label = "Vertex Buffer";
    vertexBufferDescriptor.size = sizeof(vertices);
    vertexBufferDescriptor.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    vertexBufferDescriptor.mappedAtCreation = false;
    vertexBuffer = wgpuDeviceCreateBuffer(device, &vertexBufferDescriptor);

    // Upload vertex data to the vertex buffer.
    wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertices, vertexBufferDescriptor.size);

    // Create the index buffer
    WGPUBufferDescriptor indexBufferDescriptor{};
    indexBufferDescriptor.label = "Index Buffer";
    indexBufferDescriptor.size = sizeof(indices);
    indexBufferDescriptor.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    indexBufferDescriptor.mappedAtCreation = false;
    indexBuffer = wgpuDeviceCreateBuffer(device, &indexBufferDescriptor);

    // Upload index data to the index buffer.
    wgpuQueueWriteBuffer(queue, indexBuffer, 0, indices, indexBufferDescriptor.size);

    // Create a uniform buffer to store the MVP matrix.
    WGPUBufferDescriptor mvpBufferDescriptor{};
    mvpBufferDescriptor.label = "MVP Buffer";
    mvpBufferDescriptor.size = sizeof(glm::mat4);
    mvpBufferDescriptor.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    mvpBufferDescriptor.mappedAtCreation = false;
    mvpBuffer = wgpuDeviceCreateBuffer(device, &mvpBufferDescriptor);
    // We will write to the MVP buffer in the render loop.

    // Configure the render surface.
    WGPUSurfaceCapabilities surfaceCapabilities{};
    wgpuSurfaceGetCapabilities(surface, adapter, &surfaceCapabilities);
    WGPUTextureFormat surfaceFormat = surfaceCapabilities.formats[0];

    surfaceConfiguration.device = device;
    surfaceConfiguration.format = surfaceFormat;
    surfaceConfiguration.usage = WGPUTextureUsage_RenderAttachment;
    surfaceConfiguration.viewFormatCount = 0;
    surfaceConfiguration.viewFormats = nullptr;
    surfaceConfiguration.alphaMode = WGPUCompositeAlphaMode_Auto;
    surfaceConfiguration.presentMode = WGPUPresentMode_Fifo; // This must be Fifo on Emscripten.

    // Resize & configure the surface and depth buffer.
    onResize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Load the shader module.
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.code = SHADER_MODULE;

    WGPUShaderModuleDescriptor shaderModuleDescriptor{};
    shaderModuleDescriptor.nextInChain = &shaderCodeDesc.chain;

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDescriptor);

    // Set up the render pipeline.

    // Setup the color targets.
    WGPUColorTargetState colorTargetState{};
    colorTargetState.format = surfaceFormat;
    colorTargetState.blend = nullptr; // &blendState;
    colorTargetState.writeMask = WGPUColorWriteMask_All;

    // Setup the binding layout.
    // The binding group only requires a single entry:
    // @group(0) @binding(0) var<uniform> mvp : mat4x4f;
    // First, define the binding entry in the group:
    WGPUBindGroupLayoutEntry bindGroupLayoutEntry{};
    bindGroupLayoutEntry.binding = 0;
    bindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex;
    bindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
    bindGroupLayoutEntry.buffer.minBindingSize = sizeof(glm::mat4);

    // Setup the binding group.
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor{};
    bindGroupLayoutDescriptor.label = "Binding Group";
    bindGroupLayoutDescriptor.entryCount = 1;
    bindGroupLayoutDescriptor.entries = &bindGroupLayoutEntry;
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDescriptor);

    // Create the pipeline layout.
    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor{};
    pipelineLayoutDescriptor.label = "Pipeline Layout";
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDescriptor);

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.label = "Render Pipeline";
    pipelineDescriptor.layout = pipelineLayout;

    // Primitive assembly.
    pipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDescriptor.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDescriptor.primitive.cullMode = WGPUCullMode_Back;

    // Describe the vertex layout.
    WGPUVertexAttribute attributes[] = {
        {
            // glm::vec3 position;
            .format = WGPUVertexFormat_Float32x3,
            .offset = 0,
            .shaderLocation = 0,
        },
        {
            // glm::vec3 color;
            .format = WGPUVertexFormat_Float32x3,
            .offset = sizeof(glm::vec3),
            .shaderLocation = 1,
        }
    };
    WGPUVertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(Vertex);
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = std::size(attributes);
    vertexBufferLayout.attributes = attributes;

    // Vertex shader stage.
    pipelineDescriptor.vertex.module = shaderModule;
    pipelineDescriptor.vertex.entryPoint = "vs_main";
    pipelineDescriptor.vertex.constantCount = 0;
    pipelineDescriptor.vertex.constants = nullptr;
    pipelineDescriptor.vertex.bufferCount = 1;
    pipelineDescriptor.vertex.buffers = &vertexBufferLayout;

    // Fragment shader stage.
    WGPUFragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;
    pipelineDescriptor.fragment = &fragmentState;

    // Setup stencil face state.
    WGPUStencilFaceState stencilFaceState{};
    stencilFaceState.compare = WGPUCompareFunction_Always;
    stencilFaceState.failOp = WGPUStencilOperation_Keep;
    stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
    stencilFaceState.passOp = WGPUStencilOperation_Keep;

    // Depth/Stencil state.
    WGPUDepthStencilState depthStencilState{};
    depthStencilState.format = WGPUTextureFormat_Depth32Float;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
    depthStencilState.stencilFront = stencilFaceState;
    depthStencilState.stencilBack = stencilFaceState;
    depthStencilState.stencilReadMask = ~0u;
    depthStencilState.stencilWriteMask = ~0u;
    depthStencilState.depthBias = 0;
    depthStencilState.depthBiasSlopeScale = 0.0f;
    depthStencilState.depthBiasClamp = 0.0f;

    pipelineDescriptor.depthStencil = &depthStencilState;

    // Multisampling.
    pipelineDescriptor.multisample.count = 1;
    pipelineDescriptor.multisample.mask = ~0u; // All bits on.
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;

    pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDescriptor);

    // Release the shader module.
    wgpuShaderModuleRelease(shaderModule);

    // Release the pipeline layout.
    wgpuPipelineLayoutRelease(pipelineLayout);

    // We are done with the adapter, it is safe to release it.
    wgpuAdapterRelease(adapter);
}

WGPUTextureView getNextSurfaceTextureView(WGPUSurface s)
{
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(s, &surfaceTexture);

    switch (surfaceTexture.status)
    {
    case WGPUSurfaceGetCurrentTextureStatus_Success:
        // All good. Just continue.
        break;
    case WGPUSurfaceGetCurrentTextureStatus_Timeout:
    case WGPUSurfaceGetCurrentTextureStatus_Outdated:
    case WGPUSurfaceGetCurrentTextureStatus_Lost:
    {
        // Reconfigure the texture and skip this frame.
        if (surfaceTexture.texture)
            wgpuTextureRelease(surfaceTexture.texture);

        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        if (width > 0 && height > 0)
        {
            surfaceConfiguration.width = width;
            surfaceConfiguration.height = height;

            wgpuSurfaceConfigure(surface, &surfaceConfiguration);
        }
        return nullptr;
    }
    default:
        // Handle the error.
        std::cerr << "Error getting surface texture: " << surfaceTexture.status << std::endl;
        return nullptr;
    }

    WGPUTextureViewDescriptor viewDescriptor{};
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    wgpuTextureRelease( surfaceTexture.texture );

    return targetView;
}

void render()
{
    // Get the next texture view from the surface.
    WGPUTextureView view = getNextSurfaceTextureView(surface);
    if (!view)
        return;

    // Create a command encoder.
    WGPUCommandEncoderDescriptor encoderDescriptor{};
    encoderDescriptor.label = "Command Encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDescriptor);

    // Create a render pass.
    WGPURenderPassColorAttachment colorAttachment{};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = WGPUColor{ 0.4f, 0.6f, 0.9f, 1.0f };
#ifndef WEBGPU_BACKEND_WGPU
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

    WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
    depthStencilAttachment.view = depthTextureView;
    depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
    depthStencilAttachment.depthClearValue = 1.0f;
    depthStencilAttachment.depthReadOnly = false;
    depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
    depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilReadOnly = true;

    // Create the render pass.
    WGPURenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.label = "Render Pass";
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;
    renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    renderPassDescriptor.timestampWrites = nullptr;
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDescriptor);

    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, sizeof(vertices));
    wgpuRenderPassEncoderSetIndexBuffer(renderPass, indexBuffer, WGPUIndexFormat_Uint16, 0, sizeof(indices));

    // Bind the MVP uniform buffer.
    WGPUBindGroupEntry binding{};
    binding.binding = 0;
    binding.buffer = mvpBuffer;
    binding.offset = 0;
    binding.size = sizeof(glm::mat4);

    WGPUBindGroupDescriptor bindGroupDescriptor{};
    bindGroupDescriptor.label = "Bind Group";
    bindGroupDescriptor.layout = bindGroupLayout;
    bindGroupDescriptor.entryCount = 1;
    bindGroupDescriptor.entries = &binding;
    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDescriptor);

    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroup, 0, nullptr);

    wgpuRenderPassEncoderDrawIndexed(renderPass, std::size(indices), 1, 0, 0, 0);

    // End the render pass.
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    // Create a command buffer from the encoder.
    WGPUCommandBufferDescriptor commandBufferDescriptor{};
    commandBufferDescriptor.label = "Command Buffer";
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &commandBufferDescriptor);

    // Submit the command buffer to command queue.
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr);
    wgpuQueueSubmit(queue, 1, &commandBuffer);

    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(encoder);
    wgpuTextureViewRelease(view);
#ifndef __EMSCRIPTEN__
    // Do not present when using emscripten. This happens automatically.
    wgpuSurfacePresent(surface);
#endif

    // Poll the device to check if the work is done.
    pollDevice(device);
}

void update(void* userdata = nullptr)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                std::cout << "Window resized: " << event.window.data1 << "x" << event.window.data2 << std::endl;
                onResize(event.window.data1, event.window.data2);
            }
            break;
        default:;
        }
    }

    timer.tick();

    // Update the model-view-projection matrix.
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    float angle = static_cast<float>(timer.totalSeconds() * 90.0);
    glm::vec3 axis = glm::vec3(0.0f, 1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::rotate(glm::mat4{ 1 }, glm::radians(angle), axis);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3{ 0, 0, -10 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 1, 0 });
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

    // Update the MVP matrix in the uniform buffer.
    wgpuQueueWriteBuffer(queue, mvpBuffer, 0, &mvpMatrix, sizeof(mvpMatrix));

    render();
}

void destroy()
{
    wgpuBufferRelease(vertexBuffer);
    wgpuBufferRelease(indexBuffer);
    wgpuBufferRelease(mvpBuffer);
    wgpuBindGroupLayoutRelease(bindGroupLayout);
    wgpuRenderPipelineRelease(pipeline);
    wgpuTextureRelease(depthTexture);
    wgpuTextureViewRelease(depthTextureView);
    wgpuSurfaceUnconfigure(surface);
    wgpuSurfaceRelease(surface);
    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuInstanceRelease(instance);

    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int, char**) {
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(update, nullptr, 0, 1);
#else

    while (isRunning)
    {
        update();
    }

    destroy();

#endif

    return 0;
}

