#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

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
#ifdef WEBGPU_BACKEND_DAWN
    {WGPUFeatureName_PipelineStatisticsQuery, "PipelineStatisticsQuery"},
    {WGPUFeatureName_DawnShaderFloat16, "DawnShaderFloat16"},
    {WGPUFeatureName_DawnInternalUsages, "DawnInternalUsages"},
    {WGPUFeatureName_DawnMultiPlanarFormats, "DawnMultiPlanarFormats"},
    {WGPUFeatureName_DawnNative, "DawnNative"},
    {WGPUFeatureName_ChromiumExperimentalDp4a, "ChromiumExperimentalDp4a"},
    {WGPUFeatureName_TimestampQueryInsidePasses, "TimestampQueryInsidePasses"},
    {WGPUFeatureName_ImplicitDeviceSynchronization, "ImplicitDeviceSynchronization"},
    {WGPUFeatureName_SurfaceCapabilities, "SurfaceCapabilities"},
    {WGPUFeatureName_TransientAttachments, "TransientAttachments"},
    {WGPUFeatureName_MSAARenderToSingleSampled, "MSAARenderToSingleSampled"},
#endif
};

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char* WINDOW_TITLE = "Clear Screen";

SDL_Window* window = nullptr;

WGPUInstance instance = nullptr;
WGPUDevice device = nullptr;
WGPUSurface surface = nullptr;

bool isRunning = true;

WGPUAdapter requestAdapter(WGPUInstance intance, const WGPURequestAdapterOptions* options)
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
        std::cout << std::hex << "  - 0x" << feature << ": " << featureNames[feature] << std::endl;
    }

}

// Initialize the application.
void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

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
    instance = wgpuCreateInstance(&instanceDescriptor);
#endif

    if (!instance)
    {
        std::cerr << "Failed to create WebGPU instance." << std::endl;
        return;
    }

    // Request the adapter.
    WGPURequestAdapterOptions requestAdapaterOptions{};
    WGPUAdapter adapter = requestAdapter(instance, &requestAdapaterOptions);

    if (!adapter)
    {
        std::cerr << "Failed to get adapter." << std::endl;
        return;
    }

    inspectAdapter(adapter);

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor{};
    deviceDescriptor.label = "LearnWebGPU"; // You can use anything here.
#ifdef __EMSCRIPTEN__
    deviceDescriptor.requiredFeatureCount = 0; // We don't require any extra features.
#else
    deviceDescriptor.requiredFeaturesCount = 0; // We don't require any extra features.
#endif
    deviceDescriptor.requiredFeatures = nullptr;
    deviceDescriptor.requiredLimits = nullptr; // We don't require any specific limits.
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.defaultQueue.label = "Default Queue"; // You can use anything here.
    deviceDescriptor.deviceLostCallback = nullptr;
    device = requestDevice(adapter, &deviceDescriptor);

    // We are done with the adapter, it is safe to release it.
    wgpuAdapterRelease(adapter);

    if (!device)
    {
        std::cerr << "Failed to create device." << std::endl;
        return;
    }




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
        default:;
        }
    }
}

void destroy()
{
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

