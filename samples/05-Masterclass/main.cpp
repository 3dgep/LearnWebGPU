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
const char*   WINDOW_TITLE  = "Masterclass";

SDL_Window* window = nullptr;

WGPUInstance             instance = nullptr;
WGPUDevice               device   = nullptr;
WGPUQueue                queue    = nullptr;
WGPUSurface              surface  = nullptr;
WGPUSurfaceConfiguration surfaceConfiguration {};

bool isRunning = true;

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
}

void resize()
{
    int width, height;
    SDL_GetWindowSize( window, &width, &height );
}

void render()
{}

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
