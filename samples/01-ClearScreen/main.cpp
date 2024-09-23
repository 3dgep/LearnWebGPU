#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <sdl2webgpu.h>
#include <webgpu/webgpu.h>

#ifdef WEBGPU_BACKEND_WGPU
    #include <webgpu/wgpu.h>  // Include non-standard functions.
#endif

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

std::map<WGPUFeatureName, std::string> featureNames = {
    { WGPUFeatureName_DepthClipControl, "DepthClipControl" },
    { WGPUFeatureName_Depth32FloatStencil8, "Depth32FloatStencil8" },
    { WGPUFeatureName_TimestampQuery, "TimestampQuery" },
    { WGPUFeatureName_TextureCompressionBC, "TextureCompressionBC" },
    { WGPUFeatureName_TextureCompressionETC2, "TextureCompressionETC2" },
    { WGPUFeatureName_TextureCompressionASTC, "TextureCompressionASTC" },
    { WGPUFeatureName_IndirectFirstInstance, "IndirectFirstInstance" },
    { WGPUFeatureName_ShaderF16, "ShaderF16" },
    { WGPUFeatureName_RG11B10UfloatRenderable, "RG11B10UfloatRenderable" },
    { WGPUFeatureName_BGRA8UnormStorage, "BGRA8UnormStorage" },
    { WGPUFeatureName_Float32Filterable, "Float32Filterable" },
};

std::map<WGPUQueueWorkDoneStatus, std::string> queueWorkDoneStatusNames = {
    { WGPUQueueWorkDoneStatus_Success, "Success" },
    { WGPUQueueWorkDoneStatus_Error, "Error" },
    { WGPUQueueWorkDoneStatus_Unknown, "Unknown" },
    { WGPUQueueWorkDoneStatus_DeviceLost, "DeviceLost" },
};

std::map<WGPUBackendType, std::string> backendTypes = {
    { WGPUBackendType_Undefined, "Undefined" }, { WGPUBackendType_Null, "Null" },
    { WGPUBackendType_WebGPU, "WebGPU" },       { WGPUBackendType_D3D11, "D3D11" },
    { WGPUBackendType_D3D12, "D3D12" },         { WGPUBackendType_Metal, "Metal" },
    { WGPUBackendType_Vulkan, "Vulkan" },       { WGPUBackendType_OpenGL, "OpenGL" },
    { WGPUBackendType_OpenGLES, "OpenGLES" },
};

constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char*   WINDOW_TITLE  = "Clear Screen";

SDL_Window* window = nullptr;

WGPUInstance             instance = nullptr;
WGPUDevice               device   = nullptr;
WGPUQueue                queue    = nullptr;
WGPUSurface              surface  = nullptr;
WGPUSurfaceConfiguration surfaceConfiguration {};

bool isRunning = true;

WGPUAdapter requestAdapter( const WGPURequestAdapterOptions* options )
{
    // Used by the requestAdapterCallback function to store the adapter and to notify
    // us when the request is complete.
    struct UserData
    {
        WGPUAdapter adapter = nullptr;
        bool        done    = false;
    } userData;

    // Callback function that is called by wgpuInstanceRequestAdapter when the request is complete.
    auto requestAdapterCallback = []( WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message,
                                      void* userData ) {
        auto& data = *static_cast<UserData*>( userData );
        if ( status == WGPURequestAdapterStatus_Success )
        {
            data.adapter = adapter;
        }
        else
        {
            std::cerr << "Failed to request adapter: " << message << std::endl;
        }
        data.done = true;
    };

    wgpuInstanceRequestAdapter( instance, options, requestAdapterCallback, &userData );

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while ( !userData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( userData.done );

    return userData.adapter;
}

bool getAdapterLimits( WGPUAdapter adapter, WGPUSupportedLimits& supportedLimits )
{
#ifdef WEBGPU_BACKEND_DAWN
    // Dawn returns a status flag instead of WGPUBool.
    return wgpuAdapterGetLimits( adapter, &supportedLimits ) == WGPUStatus_Success;
#else
    return wgpuAdapterGetLimits( adapter, &supportedLimits );
#endif
}

void inspectAdapter( WGPUAdapter adapter )
{
    // List the adapter properties.
    // Note: Deprecated, use WGPUAdapterInfo instead (when it's supported on all backends).
    WGPUAdapterProperties adapterProperties {};
    wgpuAdapterGetProperties( adapter, &adapterProperties );

    std::cout << "Adapter name:   " << adapterProperties.name << std::endl;
    std::cout << "Adapter vendor: " << adapterProperties.vendorName << std::endl;
    std::cout << "Backend type:   " << backendTypes[adapterProperties.backendType] << std::endl;

    // List adapter limits.
    WGPUSupportedLimits supportedLimits {};
    if ( getAdapterLimits( adapter, supportedLimits ) )
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
        std::cout << "  maxDynamicUniformBuffersPerPipelineLayout: " << limits.maxDynamicUniformBuffersPerPipelineLayout
                  << std::endl;
        std::cout << "  maxDynamicStorageBuffersPerPipelineLayout: " << limits.maxDynamicStorageBuffersPerPipelineLayout
                  << std::endl;
        std::cout << "  maxSampledTexturesPerShaderStage: " << limits.maxSampledTexturesPerShaderStage << std::endl;
        std::cout << "  maxSamplersPerShaderStage:        " << limits.maxSamplersPerShaderStage << std::endl;
        std::cout << "  maxStorageBuffersPerShaderStage:  " << limits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << "  maxStorageTexturesPerShaderStage: " << limits.maxStorageTexturesPerShaderStage << std::endl;
        std::cout << "  maxUniformBuffersPerShaderStage:  " << limits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << "  maxUniformBufferBindingSize:      " << limits.maxUniformBufferBindingSize << std::endl;
        std::cout << "  maxStorageBufferBindingSize:      " << limits.maxStorageBufferBindingSize << std::endl;
        std::cout << "  minUniformBufferOffsetAlignment:  " << limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << "  minStorageBufferOffsetAlignment:  " << limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << "  maxVertexBuffers:                 " << limits.maxVertexBuffers << std::endl;
        std::cout << "  maxBufferSize:                    " << limits.maxBufferSize << std::endl;
        std::cout << "  maxVertexAttributes:              " << limits.maxVertexAttributes << std::endl;
        std::cout << "  maxVertexBufferArrayStride:       " << limits.maxVertexBufferArrayStride << std::endl;
        std::cout << "  maxInterStageShaderComponents:    " << limits.maxInterStageShaderComponents << std::endl;
        std::cout << "  maxInterStageShaderVariables:     " << limits.maxInterStageShaderVariables << std::endl;
        std::cout << "  maxColorAttachments:              " << limits.maxColorAttachments << std::endl;
        std::cout << "  maxComputeWorkgroupStorageSize:   " << limits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << "  maxComputeInvocationsPerWorkgroup:" << limits.maxComputeInvocationsPerWorkgroup << std::endl;
        std::cout << "  maxComputeWorkgroupSizeX:         " << limits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << "  maxComputeWorkgroupSizeY:         " << limits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << "  maxComputeWorkgroupSizeZ:         " << limits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << "  maxComputeWorkgroupsPerDimension: " << limits.maxComputeWorkgroupsPerDimension << std::endl;
    }

    // List the adapter features.
    std::vector<WGPUFeatureName> features;

    // Query the number of features.
    size_t featureCount = wgpuAdapterEnumerateFeatures( adapter, nullptr );

    // Allocate memory to store the resulting features.
    features.resize( featureCount );

    // Enumerate again, now with the allocated to store the features.
    wgpuAdapterEnumerateFeatures( adapter, features.data() );

    std::cout << "Adapter features: " << std::endl;
    for ( auto feature: features )
    {
        // Print features in hexadecimal format to make it easier to compare with the WebGPU specification.
        std::cout << "  - 0x" << std::hex << feature << std::dec << ": " << featureNames[feature] << std::endl;
    }
}

WGPUDevice requestDevice( WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor )
{
    struct UserData
    {
        WGPUDevice device = nullptr;
        bool       done   = false;
    } userData;

    auto requestDeviceCallback = []( WGPURequestDeviceStatus status, WGPUDevice device, const char* message,
                                     void* userData ) {
        auto& data = *static_cast<UserData*>( userData );
        if ( status == WGPURequestDeviceStatus_Success )
        {
            data.device = device;
        }
        else
        {
            std::cerr << "Failed to request device: " << message << std::endl;
        }
        data.done = true;
    };

    wgpuAdapterRequestDevice( adapter, descriptor, requestDeviceCallback, &userData );

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while ( !userData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( userData.done );

    return userData.device;
}

// A callback function that is called when the GPU device is no longer available for some reason.
void onDeviceLost( WGPUDeviceLostReason reason, char const* message, void* )
{
    std::cerr << "Device lost: " << std::hex << reason << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

// A callback function that is called when we do something wrong with the device.
// For example, we run out of memory or we do something wrong with the API.
void onUncapturedErrorCallback( WGPUErrorType type, char const* message, void* )
{
    std::cerr << "Uncaptured device error: " << std::hex << type << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

// A callback function that is called when submitted work is done.
void onQueueWorkDone( WGPUQueueWorkDoneStatus status, void* )
{
    // std::cout << "Queue work done [" << std::hex << status << std::dec << "]: " << queueWorkDoneStatusNames[status]
    //           << std::endl;
}

// Initialize the application.
void init()
{
    SDL_Init( SDL_INIT_VIDEO );
    window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                               WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE );

    if ( !window )
    {
        std::cerr << "Failed to create window." << std::endl;
        return;
    }

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    // For some reason, the instance descriptor must be null when using emscripten.
    instance = wgpuCreateInstance( nullptr );
#else
    WGPUInstanceDescriptor instanceDescriptor {};
    #ifdef WEBGPU_BACKEND_DAWN
    // Make sure the uncaptured error callback is called as soon as an error
    // occurs, rather than waiting for the next Tick. This enables using the
    // stack trace in which the uncaptured error occurred when breaking into the
    // uncaptured error callback.
    const char*               enabledToggles[] = { "enable_immediate_error_handling" };
    WGPUDawnTogglesDescriptor toggles {};
    toggles.chain.next             = nullptr;
    toggles.chain.sType            = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount    = 0;
    toggles.enabledToggleCount     = std::size( enabledToggles );
    toggles.enabledToggles         = enabledToggles;
    instanceDescriptor.nextInChain = &toggles.chain;
    #endif
    instance = wgpuCreateInstance( &instanceDescriptor );
#endif

    if ( !instance )
    {
        std::cerr << "Failed to create WebGPU instance." << std::endl;
        return;
    }

    surface = SDL_GetWGPUSurface( instance, window );

    if ( !surface )
    {
        std::cerr << "Failed to get surface." << std::endl;
        return;
    }

    // Request the adapter.
    WGPURequestAdapterOptions requestAdapterOptions {};
    requestAdapterOptions.compatibleSurface = surface;
    WGPUAdapter adapter                     = requestAdapter( &requestAdapterOptions );

    if ( !adapter )
    {
        std::cerr << "Failed to get adapter." << std::endl;
        return;
    }

    inspectAdapter( adapter );

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor {};
    deviceDescriptor.label                = "WebGPU with C++";  // You can use anything here.
    deviceDescriptor.requiredFeatureCount = 0;                  // We don't require any extra features.
    deviceDescriptor.requiredFeatures     = nullptr;
    deviceDescriptor.requiredLimits       = nullptr;          // We don't require any specific limits.
    deviceDescriptor.defaultQueue.label   = "Default Queue";  // You can use anything here.
    deviceDescriptor.deviceLostCallback   = onDeviceLost;
    device                                = requestDevice( adapter, &deviceDescriptor );

    if ( !device )
    {
        std::cerr << "Failed to create device." << std::endl;
        return;
    }

    // Set the uncaptured error callback.
    wgpuDeviceSetUncapturedErrorCallback( device, onUncapturedErrorCallback, nullptr );

    // Get the device queue.
    queue = wgpuDeviceGetQueue( device );

    if ( !queue )
    {
        std::cerr << "Failed to get device queue." << std::endl;
        return;
    }

    wgpuQueueOnSubmittedWorkDone( queue, onQueueWorkDone, nullptr );

    WGPUSurfaceCapabilities surfaceCaps {};
    wgpuSurfaceGetCapabilities( surface, adapter, &surfaceCaps );

    WGPUTextureFormat preferredFormat = surfaceCaps.formats[0];
    bool              supportsMailbox = false;

    for ( size_t i = 0; i < surfaceCaps.presentModeCount; ++i )
    {
        if ( surfaceCaps.presentModes[i] == WGPUPresentMode_Mailbox )
            supportsMailbox = true;
    }

    // Cleanup allocations for surface capabilities.
    wgpuSurfaceCapabilitiesFreeMembers( surfaceCaps );

    // Configure the render surface.
    surfaceConfiguration.device          = device;
    surfaceConfiguration.format          = preferredFormat;
    surfaceConfiguration.usage           = WGPUTextureUsage_RenderAttachment;
    surfaceConfiguration.viewFormatCount = 0;
    surfaceConfiguration.viewFormats     = nullptr;
    surfaceConfiguration.alphaMode       = WGPUCompositeAlphaMode_Auto;
    surfaceConfiguration.width           = WINDOW_WIDTH;
    surfaceConfiguration.height          = WINDOW_HEIGHT;
    surfaceConfiguration.presentMode     = supportsMailbox ? WGPUPresentMode_Mailbox : WGPUPresentMode_Fifo;
    wgpuSurfaceConfigure( surface, &surfaceConfiguration );

    // We are done with the adapter, it is safe to release it.
    wgpuAdapterRelease( adapter );
}

void resize()
{
    int width, height;
    SDL_GetWindowSize( window, &width, &height );

    surfaceConfiguration.width  = std::max( width, 1 );
    surfaceConfiguration.height = std::max( height, 1 );

    wgpuSurfaceConfigure( surface, &surfaceConfiguration );
}

WGPUTexture getNextSurfaceTexture( WGPUSurface s )
{
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture( s, &surfaceTexture );

    if ( surfaceTexture.suboptimal )
    {
        if ( surfaceTexture.texture )
            wgpuTextureRelease( surfaceTexture.texture );

        resize();

        return nullptr;
    }

    switch ( surfaceTexture.status )
    {
    case WGPUSurfaceGetCurrentTextureStatus_Success:
        // All good. Just continue.
        break;
    case WGPUSurfaceGetCurrentTextureStatus_Timeout:
    case WGPUSurfaceGetCurrentTextureStatus_Outdated:
    case WGPUSurfaceGetCurrentTextureStatus_Lost:
    {
        // Reconfigure the texture and skip this frame.
        if ( surfaceTexture.texture )
            wgpuTextureRelease( surfaceTexture.texture );

        resize();

        return nullptr;
    }
    default:
        // Handle the error.
        std::cerr << "Error getting surface texture: " << surfaceTexture.status << std::endl;
        return nullptr;
    }

    return surfaceTexture.texture;
}

void render()
{
    // Get the next texture view from the surface.
    WGPUTexture surfaceTexture = getNextSurfaceTexture( surface );
    if ( !surfaceTexture )
        return;

    WGPUTextureViewDescriptor viewDescriptor {};
    viewDescriptor.label           = "Surface texture view";
    viewDescriptor.format          = wgpuTextureGetFormat( surfaceTexture );
    viewDescriptor.dimension       = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel    = 0;
    viewDescriptor.mipLevelCount   = 1;
    viewDescriptor.baseArrayLayer  = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect          = WGPUTextureAspect_All;
    WGPUTextureView surfaceTextureView     = wgpuTextureCreateView( surfaceTexture, &viewDescriptor );

    // Create a command encoder.
    WGPUCommandEncoderDescriptor encoderDescriptor {};
    encoderDescriptor.label    = "Command Encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder( device, &encoderDescriptor );

    // Create a render pass.
    WGPURenderPassColorAttachment colorAttachment {};
    colorAttachment.view          = surfaceTextureView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp        = WGPULoadOp_Clear;
    colorAttachment.storeOp       = WGPUStoreOp_Store;
    colorAttachment.clearValue    = WGPUColor { 0.4f, 0.6f, 0.9f, 1.0f };
#ifndef WEBGPU_BACKEND_WGPU
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

    // Create the render pass.
    WGPURenderPassDescriptor renderPassDescriptor {};
    renderPassDescriptor.colorAttachmentCount   = 1;
    renderPassDescriptor.colorAttachments       = &colorAttachment;
    renderPassDescriptor.depthStencilAttachment = nullptr;
    renderPassDescriptor.timestampWrites        = nullptr;
    WGPURenderPassEncoder renderPass            = wgpuCommandEncoderBeginRenderPass( encoder, &renderPassDescriptor );

    // End the render pass.
    wgpuRenderPassEncoderEnd( renderPass );
    wgpuRenderPassEncoderRelease( renderPass );

    // Create a command buffer from the encoder.
    WGPUCommandBufferDescriptor commandBufferDescriptor {};
    commandBufferDescriptor.label   = "Command Buffer";
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish( encoder, &commandBufferDescriptor );

    // Submit the command buffer to command queue.
    wgpuQueueOnSubmittedWorkDone( queue, onQueueWorkDone, nullptr );
    wgpuQueueSubmit( queue, 1, &commandBuffer );

#ifndef __EMSCRIPTEN__
    // Do not present when using emscripten. This happens automatically.
    wgpuSurfacePresent( surface );
#endif

    // Poll the device to check if the work is done.
#if defined( WEBGPU_BACKEND_DAWN )
    wgpuDeviceTick( device );
#elif defined( WEBGPU_BACKEND_WGPU )
    wgpuDevicePoll( device, false, nullptr );
#endif

    // Cleanup
    wgpuCommandBufferRelease( commandBuffer );
    wgpuCommandEncoderRelease( encoder );
    wgpuTextureViewRelease( surfaceTextureView );
    wgpuTextureRelease( surfaceTexture );
}

void update( void* userdata = nullptr )
{
    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        switch ( event.type )
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            std::cout << "Key pressed: " << SDL_GetKeyName( event.key.keysym.sym ) << std::endl;
            if ( event.key.keysym.sym == SDLK_ESCAPE )
            {
                isRunning = false;
            }
            break;
        default:;
        }
    }

    render();
}

void destroy()
{
    wgpuSurfaceUnconfigure( surface );
    wgpuSurfaceRelease( surface );
    wgpuQueueRelease( queue );
    wgpuDeviceRelease( device );
    wgpuInstanceRelease( instance );

    SDL_DestroyWindow( window );
    SDL_Quit();
}

int main( int, char** )
{
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg( update, nullptr, 0, 1 );
#else

    while ( isRunning )
    {
        update();
    }

    destroy();

#endif

    return 0;
}
