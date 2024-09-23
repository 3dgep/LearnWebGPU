#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <sdl2webgpu.h>
#include <stb_image.h>
#include <webgpu/webgpu.h>

#ifdef WEBGPU_BACKEND_WGPU
    #include <webgpu/wgpu.h>  // Include non-standard functions.
#endif

#include <Timer.hpp>

#include <glm/gtc/matrix_transform.hpp>  // For matrix transformations.
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifndef WEBGPU_BACKEND_DAWN
WGPUBool WGPUStatus_Success = 1;
#endif

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

constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char*   WINDOW_TITLE  = "Cube";

const char* SHADER_MODULE = {
#include "shader.wgsl"
};

const char* GENERATE_MIPS_MODULE = {
#include "generateMips.wgsl"
};

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoord;
};

static Vertex vertices[] = {
    { { -1, 1, 1 }, { 0, 0 } },    // 0
    { { -1, -1, 1 }, { 0, 1 } },   // 1
    { { 1, -1, 1 }, { 1, 1 } },    // 2
    { { 1, 1, 1 }, { 1, 0 } },     // 3
    { { 1, 1, 1 }, { 0, 0 } },     // 4
    { { 1, -1, 1 }, { 0, 1 } },    // 5
    { { 1, -1, -1 }, { 1, 1 } },   // 6
    { { 1, 1, -1 }, { 1, 0 } },    // 7
    { { 1, 1, -1 }, { 1, 1 } },    // 8
    { { 1, -1, -1 }, { 1, 0 } },   // 9
    { { -1, -1, -1 }, { 0, 0 } },  // 10
    { { -1, 1, -1 }, { 0, 1 } },   // 11
    { { -1, 1, -1 }, { 0, 0 } },   // 12
    { { -1, -1, -1 }, { 0, 1 } },  // 13
    { { -1, -1, 1 }, { 1, 1 } },   // 14
    { { -1, 1, 1 }, { 1, 0 } },    // 15
    { { -1, 1, -1 }, { 0, 0 } },   // 16
    { { -1, 1, 1 }, { 0, 1 } },    // 17
    { { 1, 1, 1 }, { 1, 1 } },     // 18
    { { 1, 1, -1 }, { 1, 0 } },    // 19
    { { -1, -1, 1 }, { 0, 0 } },   // 20
    { { -1, -1, -1 }, { 0, 1 } },  // 21
    { { 1, -1, -1 }, { 1, 1 } },   // 22
    { { 1, -1, 1 }, { 1, 0 } },    // 23
};

static uint16_t indices[] = {
    0,  1,  2,  2,  3,  0,   // Front face
    4,  5,  6,  6,  7,  4,   // Right face
    8,  9,  10, 10, 11, 8,   // Back face
    12, 13, 14, 14, 15, 12,  // Left face
    16, 17, 18, 18, 19, 16,  // Top face
    20, 21, 22, 22, 23, 20,  // Bottom face
};

struct Mip
{
    uint32_t  srcMipLevel = 0;
    uint32_t  numMips     = 0;
    uint32_t  dimensions  = 0;
    uint32_t  isSRGB      = 0;
    glm::vec2 texelSize { 0 };
};

SDL_Window* window = nullptr;

WGPUInstance             instance = nullptr;
WGPUDevice               device   = nullptr;
WGPULimits               deviceLimits {};
WGPUQueue                queue            = nullptr;
WGPUSurface              surface          = nullptr;
WGPUTexture              depthTexture     = nullptr;
WGPUTextureView          depthTextureView = nullptr;
WGPUSurfaceConfiguration surfaceConfiguration {};
WGPURenderPipeline       pipeline                    = nullptr;
WGPUComputePipeline      generateMipsPipeline        = nullptr;
WGPUBindGroupLayout      generateMipsBindGroupLayout = nullptr;
WGPUBindGroupLayout      bindGroupLayout             = nullptr;
WGPUBuffer               vertexBuffer                = nullptr;
WGPUBuffer               indexBuffer                 = nullptr;
WGPUBuffer               mvpBuffer                   = nullptr;
WGPUBuffer               generateMipsBuffer          = nullptr;
WGPUTexture              texture                     = nullptr;
WGPUTexture              dummyTexture = nullptr;  // Dummy texture used to pad unused textures during mipmap generation.
WGPUTextureView          textureView  = nullptr;
WGPUSampler              linearClampSampler  = nullptr;
WGPUSampler              linearRepeatSampler = nullptr;

Timer timer;
bool  isRunning = true;

/***************************************************************************
 * These functions were taken from the MiniEngine.
 * Source code available here:
 * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
 * Retrieved: January 13, 2016
 **************************************************************************/
template<typename T>
constexpr T AlignUpWithMask( T value, size_t mask )
{
    return (T)( ( (size_t)value + mask ) & ~mask );
}

template<typename T>
constexpr T AlignDownWithMask( T value, size_t mask )
{
    return (T)( (size_t)value & ~mask );
}

template<typename T>
constexpr T AlignUp( T value, size_t alignment )
{
    return AlignUpWithMask( value, alignment - 1 );
}

template<typename T>
constexpr T AlignDown( T value, size_t alignment )
{
    return AlignDownWithMask( value, alignment - 1 );
}
template<typename T>
constexpr bool IsAligned( T value, size_t alignment )
{
    return 0 == ( (size_t)value & ( alignment - 1 ) );
}

template<typename T>
constexpr T DivideByMultiple( T value, size_t alignment )
{
    return (T)( ( value + alignment - 1 ) / alignment );
}
/***************************************************************************/

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

void inspectAdapter( WGPUAdapter adapter )
{
    // List the adapter properties.
    WGPUAdapterProperties adapterProperties {};
    wgpuAdapterGetProperties( adapter, &adapterProperties );

    std::cout << "Adapter name: " << adapterProperties.name << std::endl;
    std::cout << "Adapter vendor: " << adapterProperties.vendorName << std::endl;

    // List adapter limits.
    WGPUSupportedLimits supportedLimits {};
    if ( wgpuAdapterGetLimits( adapter, &supportedLimits ) == WGPUStatus_Success )
    {
        deviceLimits = supportedLimits.limits;
        std::cout << "Limits: " << std::endl;
        std::cout << "  maxTextureDimension1D:                     " << deviceLimits.maxTextureDimension1D << std::endl;
        std::cout << "  maxTextureDimension2D:                     " << deviceLimits.maxTextureDimension2D << std::endl;
        std::cout << "  maxTextureDimension3D:                     " << deviceLimits.maxTextureDimension3D << std::endl;
        std::cout << "  maxTextureArrayLayers:                     " << deviceLimits.maxTextureArrayLayers << std::endl;
        std::cout << "  maxBindGroups:                             " << deviceLimits.maxBindGroups << std::endl;
        std::cout << "  maxBindGroupsPlusVertexBuffers:            " << deviceLimits.maxBindGroupsPlusVertexBuffers << std::endl;
        std::cout << "  maxBindingsPerBindGroup:                   " << deviceLimits.maxBindingsPerBindGroup << std::endl;
        std::cout << "  maxDynamicUniformBuffersPerPipelineLayout: "
                  << deviceLimits.maxDynamicUniformBuffersPerPipelineLayout << std::endl;
        std::cout << "  maxDynamicStorageBuffersPerPipelineLayout: "
                  << deviceLimits.maxDynamicStorageBuffersPerPipelineLayout << std::endl;
        std::cout << "  maxSampledTexturesPerShaderStage:          " << deviceLimits.maxSampledTexturesPerShaderStage
                  << std::endl;
        std::cout << "  maxSamplersPerShaderStage:                 " << deviceLimits.maxSamplersPerShaderStage << std::endl;
        std::cout << "  maxStorageBuffersPerShaderStage:           " << deviceLimits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << "  maxStorageTexturesPerShaderStage:          " << deviceLimits.maxStorageTexturesPerShaderStage
                  << std::endl;
        std::cout << "  maxUniformBuffersPerShaderStage:           " << deviceLimits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << "  maxUniformBufferBindingSize:               " << deviceLimits.maxUniformBufferBindingSize << std::endl;
        std::cout << "  maxStorageBufferBindingSize:               " << deviceLimits.maxStorageBufferBindingSize << std::endl;
        std::cout << "  minUniformBufferOffsetAlignment:           " << deviceLimits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << "  minStorageBufferOffsetAlignment:           " << deviceLimits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << "  maxVertexBuffers:                          " << deviceLimits.maxVertexBuffers << std::endl;
        std::cout << "  maxBufferSize:                             " << deviceLimits.maxBufferSize << std::endl;
        std::cout << "  maxVertexAttributes:                       " << deviceLimits.maxVertexAttributes << std::endl;
        std::cout << "  maxVertexBufferArrayStride:                " << deviceLimits.maxVertexBufferArrayStride << std::endl;
        std::cout << "  maxInterStageShaderComponents:             " << deviceLimits.maxInterStageShaderComponents << std::endl;
        std::cout << "  maxInterStageShaderVariables:              " << deviceLimits.maxInterStageShaderVariables << std::endl;
        std::cout << "  maxColorAttachments:                       " << deviceLimits.maxColorAttachments << std::endl;
        std::cout << "  maxComputeWorkgroupStorageSize:            " << deviceLimits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << "  maxComputeInvocationsPerWorkgroup:         " << deviceLimits.maxComputeInvocationsPerWorkgroup
                  << std::endl;
        std::cout << "  maxComputeWorkgroupSizeX:                  " << deviceLimits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << "  maxComputeWorkgroupSizeY:                  " << deviceLimits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << "  maxComputeWorkgroupSizeZ:                  " << deviceLimits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << "  maxComputeWorkgroupsPerDimension:          " << deviceLimits.maxComputeWorkgroupsPerDimension
                  << std::endl;
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
    //    std::cout << "Queue work done [" << std::hex << status << std::dec << "]: " <<
    //    queueWorkDoneStatusNames[status] << std::endl;
}

// Poll the GPU to allow work to be done on the device queue.
void pollDevice( WGPUDevice _device, bool sleep = false )
{
#if defined( WEBGPU_BACKEND_DAWN )
    wgpuDeviceTick( _device );
#elif defined( WEBGPU_BACKEND_WGPU )
    wgpuDevicePoll( _device, false, nullptr );
#elif defined( WEBGPU_BACKEND_EMSCRIPTEN )
    if ( sleep )
    {
        emscripten_sleep( 100 );
    }
#endif
}

// Poll the queue until all work is done.
void flushQueue()
{
    bool done = false;
    wgpuQueueOnSubmittedWorkDone(
        queue,
        []( WGPUQueueWorkDoneStatus status, void* pDone ) {
            std::cout << "Queue work done [" << std::hex << status << std::dec
                      << "]: " << queueWorkDoneStatusNames[status] << std::endl;
            *static_cast<bool*>( pDone ) = true;
        },
        &done );

    while ( !done )
    {
        pollDevice( device, true );
    }
}

void onResize( int width, int height )
{
    if ( depthTextureView )
        wgpuTextureViewRelease( depthTextureView );

    if ( depthTexture )
        wgpuTextureRelease( depthTexture );

    surfaceConfiguration.width  = width;
    surfaceConfiguration.height = height;
    wgpuSurfaceConfigure( surface, &surfaceConfiguration );

    // Create the depth texture.
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth32Float;

    WGPUTextureDescriptor depthTextureDescriptor {};
    depthTextureDescriptor.label                   = "Depth Texture";
    depthTextureDescriptor.usage                   = WGPUTextureUsage_RenderAttachment;
    depthTextureDescriptor.dimension               = WGPUTextureDimension_2D;
    depthTextureDescriptor.size.width              = width;
    depthTextureDescriptor.size.height             = height;
    depthTextureDescriptor.size.depthOrArrayLayers = 1;
    depthTextureDescriptor.format                  = depthTextureFormat;
    depthTextureDescriptor.mipLevelCount           = 1;
    depthTextureDescriptor.sampleCount             = 1;
    depthTextureDescriptor.viewFormatCount         = 1;
    depthTextureDescriptor.viewFormats             = &depthTextureFormat;
    depthTexture                                   = wgpuDeviceCreateTexture( device, &depthTextureDescriptor );

    // And create the depth texture view.
    WGPUTextureViewDescriptor depthTextureViewDescriptor {};
    depthTextureViewDescriptor.label           = "Depth Texture View";
    depthTextureViewDescriptor.format          = depthTextureFormat;
    depthTextureViewDescriptor.dimension       = WGPUTextureViewDimension_2D;
    depthTextureViewDescriptor.baseMipLevel    = 0;
    depthTextureViewDescriptor.mipLevelCount   = 1;
    depthTextureViewDescriptor.baseArrayLayer  = 0;
    depthTextureViewDescriptor.arrayLayerCount = 1;
    depthTextureViewDescriptor.aspect          = WGPUTextureAspect_DepthOnly;
    depthTextureView                           = wgpuTextureCreateView( depthTexture, &depthTextureViewDescriptor );
}

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 * @source: https://www.chessprogramming.org/BitScan
 * @date: July 18th, 2024
 */
int bitScanForward( uint64_t bb )
{
    static const int index64[64] = { 0,  47, 1,  56, 48, 27, 2,  60, 57, 49, 41, 37, 28, 16, 3,  61,
                                     54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11, 4,  62,
                                     46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
                                     25, 39, 14, 33, 19, 30, 9,  24, 13, 18, 8,  12, 7,  6,  5,  63 };

    const uint64_t debruijn64 = 0x03f79d71b4cb0a89;
    assert( bb != 0 );
    return index64[( ( bb ^ ( bb - 1 ) ) * debruijn64 ) >> 58];
}

void generateMips( const WGPUTextureDescriptor& desc, WGPUTexture texture )
{
    // Create a command encoder.
    WGPUCommandEncoderDescriptor commandEncoderDesc {};
    commandEncoderDesc.label          = "Generate Mips Command Encoder";
    WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder( device, &commandEncoderDesc );

    // Create a compute pass
    WGPUComputePassDescriptor computePassDesc {};
    computePassDesc.label                     = "Generate Mips Compute Pass";
    computePassDesc.timestampWrites           = nullptr;
    WGPUComputePassEncoder computePassEncoder = wgpuCommandEncoderBeginComputePass( commandEncoder, &computePassDesc );

    wgpuComputePassEncoderSetPipeline( computePassEncoder, generateMipsPipeline );

    // Compute the stride between dynamic uniform buffer offsets.
    uint32_t uniformStride = AlignUp<uint32_t>( sizeof( Mip ), 256 /*deviceLimits.minUniformBufferOffsetAlignment*/ );

    WGPUBindGroupEntry bindGroupEntries[7] {};
    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].buffer  = generateMipsBuffer;
    bindGroupEntries[0].offset  = 0;
    bindGroupEntries[0].size    = sizeof( Mip );

    // Bindings 1 .. 5 are bound in the loop below.

    bindGroupEntries[6].binding = 6;
    bindGroupEntries[6].sampler = linearClampSampler;

    for ( uint32_t srcMip = 0, pass = 0; srcMip < desc.mipLevelCount - 1; ++pass )
    {
        uint32_t srcWidth  = desc.size.width >> srcMip;
        uint32_t srcHeight = desc.size.height >> srcMip;
        uint32_t dstWidth  = srcWidth >> 1u;
        uint32_t dstHeight = srcHeight >> 1u;

        Mip mip {};
        // 0b00(0): Both width and height are even.
        // 0b01(1): Width is odd, height is even.
        // 0b10(2): Width is even, height is odd.
        // 0b11(3): Both width and height are odd.
        mip.dimensions = ( srcHeight & 1 ) << 1 | ( srcWidth & 1 );

        // The number of times we can half the size of the texture and get
        // exactly a 50% reduction in size.
        // A 1 bit in the width or height indicates an odd dimension.
        // The case where either the width or the height is exactly 1 is handled
        // as a special case (as the dimension does not require reduction).
        int mipCount =
            bitScanForward( ( dstWidth == 1 ? dstHeight : dstWidth ) | ( dstHeight == 1 ? dstWidth : dstHeight ) );

        // Maximum number of mips to generate is 4.
        mipCount = std::min( mipCount + 1, 4 );

        // Clamp to total number of mips left over.
        mipCount = ( srcMip + mipCount ) >= desc.mipLevelCount ? static_cast<int>( desc.mipLevelCount - srcMip ) - 1 :
                                                                 mipCount;

        // Dimensions should not reduce to 0.
        // This can happen if the width and height are not the same.
        dstWidth  = std::max( 1u, dstWidth );
        dstHeight = std::max( 1u, dstHeight );

        mip.srcMipLevel = srcMip;
        mip.numMips     = mipCount;
        mip.texelSize   = { 1.0f / static_cast<float>( dstWidth ), 1.0f / static_cast<float>( dstHeight ) };

        // Write the mip info to the buffer.
        uint32_t bufferOffset = uniformStride * pass;
        wgpuQueueWriteBuffer( queue, generateMipsBuffer, bufferOffset, &mip, sizeof( Mip ) );

        bindGroupEntries[0].offset = bufferOffset;

        // Setup a texture view for the source texture.
        WGPUTextureViewDescriptor srcTextureViewDesc {};
        srcTextureViewDesc.label           = "Generate Mip Source Texture";
        srcTextureViewDesc.format          = desc.format;
        srcTextureViewDesc.dimension       = WGPUTextureViewDimension_2D;
        srcTextureViewDesc.baseMipLevel    = srcMip;
        srcTextureViewDesc.mipLevelCount   = 1;
        srcTextureViewDesc.baseArrayLayer  = 0;
        srcTextureViewDesc.arrayLayerCount = 1;
        srcTextureViewDesc.aspect          = WGPUTextureAspect_All;
        WGPUTextureView srcTextureView     = wgpuTextureCreateView( texture, &srcTextureViewDesc );

        bindGroupEntries[1].binding     = 1;
        bindGroupEntries[1].textureView = srcTextureView;

        uint32_t dstMip = 0;
        for ( ; dstMip < mipCount; ++dstMip )
        {
            WGPUTextureViewDescriptor dstMipViewDesc {};
            dstMipViewDesc.label           = "Generate Mip Destination Texture";
            dstMipViewDesc.format          = desc.format;
            dstMipViewDesc.dimension       = WGPUTextureViewDimension_2D;
            dstMipViewDesc.baseMipLevel    = srcMip + dstMip + 1;
            dstMipViewDesc.mipLevelCount   = 1;
            dstMipViewDesc.baseArrayLayer  = 0;
            dstMipViewDesc.arrayLayerCount = 1;
            dstMipViewDesc.aspect          = WGPUTextureAspect_All;
            WGPUTextureView dstMipView     = wgpuTextureCreateView( texture, &dstMipViewDesc );

            bindGroupEntries[2 + dstMip].binding     = 2 + dstMip;
            bindGroupEntries[2 + dstMip].textureView = dstMipView;
        }

        // Pad any unused mips with a dummy texture view.
        for ( ; dstMip < 4; ++dstMip )
        {
            WGPUTextureViewDescriptor dstMipViewDesc {};
            dstMipViewDesc.label           = "Generate Mip Dummy Texture";
            dstMipViewDesc.format          = desc.format;
            dstMipViewDesc.dimension       = WGPUTextureViewDimension_2D;
            dstMipViewDesc.baseMipLevel    = dstMip;
            dstMipViewDesc.mipLevelCount   = 1;
            dstMipViewDesc.baseArrayLayer  = 0;
            dstMipViewDesc.arrayLayerCount = 1;
            dstMipViewDesc.aspect          = WGPUTextureAspect_All;
            WGPUTextureView dstMipView     = wgpuTextureCreateView( dummyTexture, &dstMipViewDesc );

            bindGroupEntries[2 + dstMip].binding     = 2 + dstMip;
            bindGroupEntries[2 + dstMip].textureView = dstMipView;
        }

        // Setup the bind group.
        WGPUBindGroupDescriptor bindGroupDesc {};
        bindGroupDesc.layout     = generateMipsBindGroupLayout;
        bindGroupDesc.entryCount = std::size( bindGroupEntries );
        bindGroupDesc.entries    = bindGroupEntries;
        WGPUBindGroup bindGroup  = wgpuDeviceCreateBindGroup( device, &bindGroupDesc );

        wgpuComputePassEncoderSetBindGroup( computePassEncoder, 0, bindGroup, 0, nullptr );

        wgpuComputePassEncoderDispatchWorkgroups( computePassEncoder, DivideByMultiple( dstWidth, 8 ),
                                                  DivideByMultiple( dstHeight, 8 ), 1 );

        srcMip += mipCount;
    }

    // End and submit the command encoder.
    wgpuComputePassEncoderEnd( computePassEncoder );

    // Create a command buffer.
    WGPUCommandBufferDescriptor commandBufferDesc {};
    commandBufferDesc.label         = "Generate Mips Command Buffer";
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish( commandEncoder, &commandBufferDesc );

    // Submit the command buffer to the queue.
    wgpuQueueSubmit( queue, 1, &commandBuffer );

    wgpuCommandBufferRelease( commandBuffer );
    wgpuComputePassEncoderRelease( computePassEncoder );
    wgpuCommandEncoderRelease( commandEncoder );
}

WGPUTexture loadTexture( const std::filesystem::path& filePath )
{
    // Load the texture
    int            width, height, channels;
    unsigned char* data = stbi_load( filePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha );

    if ( !data )
    {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return nullptr;
    }

    const WGPUExtent3D textureSize { static_cast<uint32_t>( width ), static_cast<uint32_t>( height ), 1u };

    // Create the texture object.
    WGPUTextureDescriptor textureDesc {};
    textureDesc.label       = filePath.filename().string().c_str();
    textureDesc.dimension   = WGPUTextureDimension_2D;
    textureDesc.format      = WGPUTextureFormat_RGBA8Unorm;
    textureDesc.size        = textureSize;
    textureDesc.sampleCount = 1;
    textureDesc.mipLevelCount =
        static_cast<uint32_t>(
            std::floor( std::log2( std::max( static_cast<float>( width ), static_cast<float>( height ) ) ) ) ) +
        1u;
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopyDst;

    WGPUTexture texture = wgpuDeviceCreateTexture( device, &textureDesc );

    // Copy mip level 0.
    WGPUImageCopyTexture dst {};
    dst.texture  = texture;
    dst.mipLevel = 0;
    dst.origin   = { 0, 0, 0 };
    dst.aspect   = WGPUTextureAspect_All;

    WGPUTextureDataLayout src {};
    src.offset       = 0;
    src.bytesPerRow  = 4 * width;
    src.rowsPerImage = height;

    wgpuQueueWriteTexture( queue, &dst, data, static_cast<size_t>( 4 * width * height ), &src, &textureSize );

    stbi_image_free( data );

    generateMips( textureDesc, texture );

    return texture;
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
    WGPURequestAdapterOptions requestAdapaterOptions {};
    requestAdapaterOptions.compatibleSurface = surface;
    WGPUAdapter adapter                      = requestAdapter( &requestAdapaterOptions );

    if ( !adapter )
    {
        std::cerr << "Failed to get adapter." << std::endl;
        return;
    }

    inspectAdapter( adapter );

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor {};
    deviceDescriptor.label                    = "LearnWebGPU";  // Used for debugging.
    deviceDescriptor.requiredFeatureCount     = 0;              // We don't require any extra features.
    deviceDescriptor.requiredFeatures         = nullptr;
    deviceDescriptor.requiredLimits           = nullptr;  // We don't require any specific limits.
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.defaultQueue.label       = "Default Queue";  // You can use anything here.
    deviceDescriptor.deviceLostCallback       = onDeviceLost;
    device                                    = requestDevice( adapter, &deviceDescriptor );

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

    // Create the vertex buffer.
    WGPUBufferDescriptor vertexBufferDescriptor {};
    vertexBufferDescriptor.label            = "Vertex Buffer";
    vertexBufferDescriptor.size             = sizeof( vertices );
    vertexBufferDescriptor.usage            = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    vertexBufferDescriptor.mappedAtCreation = false;
    vertexBuffer                            = wgpuDeviceCreateBuffer( device, &vertexBufferDescriptor );

    // Upload vertex data to the vertex buffer.
    wgpuQueueWriteBuffer( queue, vertexBuffer, 0, vertices, vertexBufferDescriptor.size );

    // Create the index buffer
    WGPUBufferDescriptor indexBufferDescriptor {};
    indexBufferDescriptor.label            = "Index Buffer";
    indexBufferDescriptor.size             = sizeof( indices );
    indexBufferDescriptor.usage            = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    indexBufferDescriptor.mappedAtCreation = false;
    indexBuffer                            = wgpuDeviceCreateBuffer( device, &indexBufferDescriptor );

    // Upload index data to the index buffer.
    wgpuQueueWriteBuffer( queue, indexBuffer, 0, indices, indexBufferDescriptor.size );

    // Create a uniform buffer to store the MVP matrix.
    WGPUBufferDescriptor mvpBufferDescriptor {};
    mvpBufferDescriptor.label            = "MVP Buffer";
    mvpBufferDescriptor.size             = sizeof( glm::mat4 );
    mvpBufferDescriptor.usage            = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    mvpBufferDescriptor.mappedAtCreation = false;
    mvpBuffer                            = wgpuDeviceCreateBuffer( device, &mvpBufferDescriptor );
    // We will write to the buffer in the render loop.

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

    surfaceConfiguration.device          = device;
    surfaceConfiguration.format          = preferredFormat;
    surfaceConfiguration.usage           = WGPUTextureUsage_RenderAttachment;
    surfaceConfiguration.viewFormatCount = 0;
    surfaceConfiguration.viewFormats     = nullptr;
    surfaceConfiguration.alphaMode       = WGPUCompositeAlphaMode_Auto;
    surfaceConfiguration.presentMode     = supportsMailbox ? WGPUPresentMode_Mailbox : WGPUPresentMode_Fifo;

    // Resize & configure the surface and depth buffer.
    onResize( WINDOW_WIDTH, WINDOW_HEIGHT );

    // Load the shader module.
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc {};
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.chain.next  = nullptr;
    shaderCodeDesc.code        = SHADER_MODULE;

    WGPUShaderModuleDescriptor shaderModuleDescriptor {};
    shaderModuleDescriptor.nextInChain = &shaderCodeDesc.chain;

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule( device, &shaderModuleDescriptor );

    // Set up the render pipeline.

    // Setup the color targets.
    WGPUColorTargetState colorTargetState {};
    colorTargetState.format    = preferredFormat;
    colorTargetState.blend     = nullptr;  // &blendState;
    colorTargetState.writeMask = WGPUColorWriteMask_All;

    // Setup the binding layout for the renderer.
    // @group(0) @binding(0) var<uniform> mvp : mat4x4f;
    // @group(0) @binding(1) var albedoTexture : texture_2d<f32>;
    // @group(0) @binding(2) var linearRepeatSampler : sampler;

    // First, define the binding entry in the group:
    WGPUBindGroupLayoutEntry bindGroupLayoutEntries[] = {
        {
            .binding = 0,
            .visibility = WGPUShaderStage_Vertex,
            .buffer = {
                .type = WGPUBufferBindingType_Uniform,
                .minBindingSize = sizeof(glm::mat4)
            },
        },
        {
            .binding = 1,
            .visibility = WGPUShaderStage_Fragment,
            .texture = {
                .sampleType = WGPUTextureSampleType_Float,
                .viewDimension = WGPUTextureViewDimension_2D,
            },
        },
        {
            .binding = 2,
            .visibility = WGPUShaderStage_Fragment,
            .sampler = {
                .type = WGPUSamplerBindingType_Filtering
            },
        }
    };

    // Setup the binding group.
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor {};
    bindGroupLayoutDescriptor.label      = "Binding Group";
    bindGroupLayoutDescriptor.entryCount = std::size( bindGroupLayoutEntries );
    bindGroupLayoutDescriptor.entries    = bindGroupLayoutEntries;
    bindGroupLayout                      = wgpuDeviceCreateBindGroupLayout( device, &bindGroupLayoutDescriptor );

    // Create the pipeline layout.
    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor {};
    pipelineLayoutDescriptor.label                = "Pipeline Layout";
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts     = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout             = wgpuDeviceCreatePipelineLayout( device, &pipelineLayoutDescriptor );

    WGPURenderPipelineDescriptor pipelineDescriptor {};
    pipelineDescriptor.label  = "Render Pipeline";
    pipelineDescriptor.layout = pipelineLayout;

    // Primitive assembly.
    pipelineDescriptor.primitive.topology         = WGPUPrimitiveTopology_TriangleList;
    pipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDescriptor.primitive.frontFace        = WGPUFrontFace_CCW;
    pipelineDescriptor.primitive.cullMode         = WGPUCullMode_None;

    // Describe the vertex layout.
    WGPUVertexAttribute attributes[] = { {
                                             // glm::vec3 position;
                                             .format         = WGPUVertexFormat_Float32x3,
                                             .offset         = 0,
                                             .shaderLocation = 0,
                                         },
                                         {
                                             // glm::vec2 texCoord;
                                             .format         = WGPUVertexFormat_Float32x2,
                                             .offset         = sizeof( glm::vec3 ),
                                             .shaderLocation = 1,
                                         } };

    WGPUVertexBufferLayout vertexBufferLayout {};
    vertexBufferLayout.arrayStride    = sizeof( Vertex );
    vertexBufferLayout.stepMode       = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = std::size( attributes );
    vertexBufferLayout.attributes     = attributes;

    // Vertex shader stage.
    pipelineDescriptor.vertex.module        = shaderModule;
    pipelineDescriptor.vertex.entryPoint    = "vs_main";
    pipelineDescriptor.vertex.constantCount = 0;
    pipelineDescriptor.vertex.constants     = nullptr;
    pipelineDescriptor.vertex.bufferCount   = 1;
    pipelineDescriptor.vertex.buffers       = &vertexBufferLayout;

    // Fragment shader stage.
    WGPUFragmentState fragmentState {};
    fragmentState.module        = shaderModule;
    fragmentState.entryPoint    = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants     = nullptr;
    fragmentState.targetCount   = 1;
    fragmentState.targets       = &colorTargetState;
    pipelineDescriptor.fragment = &fragmentState;

    // Setup stencil face state.
    WGPUStencilFaceState stencilFaceState {};
    stencilFaceState.compare     = WGPUCompareFunction_Always;
    stencilFaceState.failOp      = WGPUStencilOperation_Keep;
    stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
    stencilFaceState.passOp      = WGPUStencilOperation_Keep;

    // Depth/Stencil state.
    WGPUDepthStencilState depthStencilState {};
    depthStencilState.format              = WGPUTextureFormat_Depth32Float;
    depthStencilState.depthWriteEnabled   = true;
    depthStencilState.depthCompare        = WGPUCompareFunction_Less;
    depthStencilState.stencilFront        = stencilFaceState;
    depthStencilState.stencilBack         = stencilFaceState;
    depthStencilState.stencilReadMask     = ~0u;
    depthStencilState.stencilWriteMask    = ~0u;
    depthStencilState.depthBias           = 0;
    depthStencilState.depthBiasSlopeScale = 0.0f;
    depthStencilState.depthBiasClamp      = 0.0f;

    pipelineDescriptor.depthStencil = &depthStencilState;

    // Multisampling.
    pipelineDescriptor.multisample.count                  = 1;
    pipelineDescriptor.multisample.mask                   = ~0u;  // All bits on.
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;

    pipeline = wgpuDeviceCreateRenderPipeline( device, &pipelineDescriptor );

    // Setup the texture sampler.
    WGPUSamplerDescriptor linearRepeatSamplerDesc {};
    linearRepeatSamplerDesc.label         = "Linear Repeat Sampler";
    linearRepeatSamplerDesc.addressModeU  = WGPUAddressMode_Repeat;
    linearRepeatSamplerDesc.addressModeV  = WGPUAddressMode_Repeat;
    linearRepeatSamplerDesc.addressModeW  = WGPUAddressMode_Repeat;
    linearRepeatSamplerDesc.magFilter     = WGPUFilterMode_Linear;
    linearRepeatSamplerDesc.minFilter     = WGPUFilterMode_Linear;
    linearRepeatSamplerDesc.mipmapFilter  = WGPUMipmapFilterMode_Linear;
    linearRepeatSamplerDesc.lodMinClamp   = 0.0f;
    linearRepeatSamplerDesc.lodMaxClamp   = FLT_MAX;
    linearRepeatSamplerDesc.compare       = WGPUCompareFunction_Undefined;
    linearRepeatSamplerDesc.maxAnisotropy = 8;
    linearRepeatSampler                   = wgpuDeviceCreateSampler( device, &linearRepeatSamplerDesc );

    WGPUSamplerDescriptor linearClampSamplerDesc {};
    linearClampSamplerDesc.label         = "Linear Clamp Sampler";
    linearClampSamplerDesc.addressModeU  = WGPUAddressMode_ClampToEdge;
    linearClampSamplerDesc.addressModeV  = WGPUAddressMode_ClampToEdge;
    linearClampSamplerDesc.addressModeW  = WGPUAddressMode_ClampToEdge;
    linearClampSamplerDesc.magFilter     = WGPUFilterMode_Linear;
    linearClampSamplerDesc.minFilter     = WGPUFilterMode_Linear;
    linearClampSamplerDesc.mipmapFilter  = WGPUMipmapFilterMode_Linear;
    linearClampSamplerDesc.lodMinClamp   = 0.0f;
    linearClampSamplerDesc.lodMaxClamp   = FLT_MAX;
    linearClampSamplerDesc.compare       = WGPUCompareFunction_Undefined;
    linearClampSamplerDesc.maxAnisotropy = 1;
    linearClampSampler                   = wgpuDeviceCreateSampler( device, &linearClampSamplerDesc );

    // Release the shader module.
    wgpuShaderModuleRelease( shaderModule );

    // Release the pipeline layout.
    wgpuPipelineLayoutRelease( pipelineLayout );

    // Setup the binding layout for the generate mips compute shader.
    //@group(0) @binding(0) var<uniform> mip : Mip;
    //@group(0) @binding(1) var srcMip : texture_2d<f32>;
    //@group(0) @binding(2) var dstMip1 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(3) var dstMip2 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(4) var dstMip3 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(5) var dstMip4 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(6) var linearClampSampler : sampler;
    WGPUBindGroupLayoutEntry generateMipsBindGroupLayoutEntries[] = {
        {
            .binding = 0,
            .visibility = WGPUShaderStage_Compute,
            .buffer = {
                .type = WGPUBufferBindingType_Uniform,
                .minBindingSize = sizeof(Mip),
            },
        },
        {
            .binding = 1,
            .visibility = WGPUShaderStage_Compute,
            .texture = {
                .sampleType = WGPUTextureSampleType_Float,
                .viewDimension = WGPUTextureViewDimension_2D,
            },
        },
        {
            .binding = 2,
            .visibility = WGPUShaderStage_Compute,
            .storageTexture = {
                .access = WGPUStorageTextureAccess_WriteOnly,
                .format = WGPUTextureFormat_RGBA8Unorm,
                .viewDimension = WGPUTextureViewDimension_2D,
            },
        },
        {
            .binding = 3,
            .visibility = WGPUShaderStage_Compute,
            .storageTexture = {
                .access = WGPUStorageTextureAccess_WriteOnly,
                .format = WGPUTextureFormat_RGBA8Unorm,
                .viewDimension = WGPUTextureViewDimension_2D,
            },
        },
        {
            .binding = 4,
            .visibility = WGPUShaderStage_Compute,
            .storageTexture = {
                .access = WGPUStorageTextureAccess_WriteOnly,
                .format = WGPUTextureFormat_RGBA8Unorm,
                .viewDimension = WGPUTextureViewDimension_2D,
            },
        },
        {
            .binding = 5,
            .visibility = WGPUShaderStage_Compute,
            .storageTexture = {
                .access = WGPUStorageTextureAccess_WriteOnly,
                .format = WGPUTextureFormat_RGBA8Unorm,
                .viewDimension = WGPUTextureViewDimension_2D,
            },
        },
        {
            .binding = 6,
            .visibility = WGPUShaderStage_Compute,
            .sampler = {
                .type = WGPUSamplerBindingType_Filtering
            },
        },
    };

    // Setup the binding group layout.
    WGPUBindGroupLayoutDescriptor generateMipsBindGroupLayoutDescriptor {};
    generateMipsBindGroupLayoutDescriptor.label      = "Generate Mips Bind Group Layout";
    generateMipsBindGroupLayoutDescriptor.entryCount = std::size( generateMipsBindGroupLayoutEntries );
    generateMipsBindGroupLayoutDescriptor.entries    = generateMipsBindGroupLayoutEntries;
    generateMipsBindGroupLayout = wgpuDeviceCreateBindGroupLayout( device, &generateMipsBindGroupLayoutDescriptor );

    // And the pipeline layout for the generate mips compute shader.
    WGPUPipelineLayoutDescriptor generateMipsPipelineLayoutDescriptor {};
    generateMipsPipelineLayoutDescriptor.label                = "Generate Mips Pipeline Layout";
    generateMipsPipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    generateMipsPipelineLayoutDescriptor.bindGroupLayouts     = &generateMipsBindGroupLayout;
    WGPUPipelineLayout generateMipsPipelineLayout =
        wgpuDeviceCreatePipelineLayout( device, &generateMipsPipelineLayoutDescriptor );

    // Load the compute shader module.
    WGPUShaderModuleWGSLDescriptor generateMipsShaderCodeDesc {};
    generateMipsShaderCodeDesc.chain.next  = nullptr;
    generateMipsShaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    generateMipsShaderCodeDesc.code        = GENERATE_MIPS_MODULE;

    WGPUShaderModuleDescriptor generateMipsShaderModuleDescriptor {};
    generateMipsShaderModuleDescriptor.nextInChain = &generateMipsShaderCodeDesc.chain;
    generateMipsShaderModuleDescriptor.label       = "Generate Mips Shader Module";
    WGPUShaderModule generateMipsShaderModule =
        wgpuDeviceCreateShaderModule( device, &generateMipsShaderModuleDescriptor );

    // Now setup the compute pipeline for mipmap generation.
    WGPUComputePipelineDescriptor generateMipsPipelineDescriptor {};
    generateMipsPipelineDescriptor.label              = "Generate Mips Pipeline";
    generateMipsPipelineDescriptor.layout             = generateMipsPipelineLayout;
    generateMipsPipelineDescriptor.compute.module     = generateMipsShaderModule;
    generateMipsPipelineDescriptor.compute.entryPoint = "main";
    generateMipsPipeline = wgpuDeviceCreateComputePipeline( device, &generateMipsPipelineDescriptor );

    // We are now done with the shader module.
    wgpuShaderModuleRelease( generateMipsShaderModule );
    // And the pipeline layout
    wgpuPipelineLayoutRelease( generateMipsPipelineLayout );

    // Setup a dynamic uniform buffer for generating mipmaps.
    WGPUBufferDescriptor generateMipsBufferDescriptor {};
    generateMipsBufferDescriptor.label = "Generate Mips Buffer";
    generateMipsBufferDescriptor.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    generateMipsBufferDescriptor.size  = 4ull * 1024ull;  // 4KB should be sufficient.
    generateMipsBuffer                 = wgpuDeviceCreateBuffer( device, &generateMipsBufferDescriptor );

    // Create a dummy texture to use during mipmap generation.
    WGPUTextureDescriptor dummyTextureDesc {};
    dummyTextureDesc.label         = "Dummy Texture";
    dummyTextureDesc.usage         = WGPUTextureUsage_StorageBinding;
    dummyTextureDesc.dimension     = WGPUTextureDimension_2D;
    dummyTextureDesc.size          = { 16, 16, 1 };
    dummyTextureDesc.format        = WGPUTextureFormat_RGBA8Unorm;
    dummyTextureDesc.mipLevelCount = 4;
    dummyTextureDesc.sampleCount   = 1;
    dummyTexture                   = wgpuDeviceCreateTexture( device, &dummyTextureDesc );

    // Load the texture
    texture = loadTexture( "assets/textures/webgpu.png" );
    // Create a default view of the texture.
    textureView = wgpuTextureCreateView( texture, nullptr );

    // We are done with the adapter, it is safe to release it.
    wgpuAdapterRelease( adapter );
}

WGPUTexture getNextSurfaceTexture( WGPUSurface s )
{
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture( s, &surfaceTexture );

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

        int width, height;
        SDL_GetWindowSize( window, &width, &height );
        if ( width > 0 && height > 0 )
        {
            surfaceConfiguration.width  = width;
            surfaceConfiguration.height = height;

            wgpuSurfaceConfigure( surface, &surfaceConfiguration );
        }
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
    viewDescriptor.label               = "Surface texture view";
    viewDescriptor.format              = wgpuTextureGetFormat( surfaceTexture );
    viewDescriptor.dimension           = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel        = 0;
    viewDescriptor.mipLevelCount       = 1;
    viewDescriptor.baseArrayLayer      = 0;
    viewDescriptor.arrayLayerCount     = 1;
    viewDescriptor.aspect              = WGPUTextureAspect_All;
    WGPUTextureView surfaceTextureView = wgpuTextureCreateView( surfaceTexture, &viewDescriptor );

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

    WGPURenderPassDepthStencilAttachment depthStencilAttachment {};
    depthStencilAttachment.view              = depthTextureView;
    depthStencilAttachment.depthLoadOp       = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp      = WGPUStoreOp_Store;
    depthStencilAttachment.depthClearValue   = 1.0f;
    depthStencilAttachment.depthReadOnly     = false;
    depthStencilAttachment.stencilLoadOp     = WGPULoadOp_Undefined;
    depthStencilAttachment.stencilStoreOp    = WGPUStoreOp_Undefined;
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilReadOnly   = true;

    // Create the render pass.
    WGPURenderPassDescriptor renderPassDescriptor {};
    renderPassDescriptor.label                  = "Render Pass";
    renderPassDescriptor.colorAttachmentCount   = 1;
    renderPassDescriptor.colorAttachments       = &colorAttachment;
    renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    renderPassDescriptor.timestampWrites        = nullptr;
    WGPURenderPassEncoder renderPass            = wgpuCommandEncoderBeginRenderPass( encoder, &renderPassDescriptor );

    wgpuRenderPassEncoderSetPipeline( renderPass, pipeline );
    wgpuRenderPassEncoderSetVertexBuffer( renderPass, 0, vertexBuffer, 0, sizeof( vertices ) );
    wgpuRenderPassEncoderSetIndexBuffer( renderPass, indexBuffer, WGPUIndexFormat_Uint16, 0, sizeof( indices ) );

    // Bind the MVP uniform buffer.
    WGPUBindGroupEntry binding[] = { {
                                         .binding = 0,
                                         .buffer  = mvpBuffer,
                                         .offset  = 0,
                                         .size    = sizeof( glm::mat4 ),
                                     },
                                     {
                                         .binding     = 1,
                                         .textureView = textureView,
                                     },
                                     {
                                         .binding = 2,
                                         .sampler = linearRepeatSampler,
                                     } };

    WGPUBindGroupDescriptor bindGroupDescriptor {};
    bindGroupDescriptor.label      = "Bind Group";
    bindGroupDescriptor.layout     = bindGroupLayout;
    bindGroupDescriptor.entryCount = std::size( binding );
    bindGroupDescriptor.entries    = binding;
    WGPUBindGroup bindGroup        = wgpuDeviceCreateBindGroup( device, &bindGroupDescriptor );

    wgpuRenderPassEncoderSetBindGroup( renderPass, 0, bindGroup, 0, nullptr );

    wgpuRenderPassEncoderDrawIndexed( renderPass, std::size( indices ), 1, 0, 0, 0 );

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
    pollDevice( device );

    // Cleanup frame resources.
    wgpuCommandBufferRelease( commandBuffer );
    wgpuCommandEncoderRelease( encoder );
    wgpuTextureViewRelease( surfaceTextureView );
    wgpuTextureRelease( surfaceTexture );
    wgpuBindGroupRelease( bindGroup );
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
        case SDL_WINDOWEVENT:
            if ( event.window.event == SDL_WINDOWEVENT_RESIZED )
            {
                std::cout << "Window resized: " << event.window.data1 << "x" << event.window.data2 << std::endl;
                onResize( event.window.data1, event.window.data2 );
            }
            break;
        default:;
        }
    }

    timer.tick();

    // Update the model-view-projection matrix.
    int width, height;
    SDL_GetWindowSize( window, &width, &height );
    float     angle            = static_cast<float>( timer.totalSeconds() * 90.0 );
    glm::vec3 axis             = glm::vec3( 1.0f, 1.0f, 1.0f );
    glm::mat4 modelMatrix      = glm::rotate( glm::mat4 { 1 }, glm::radians( angle ), axis );
    glm::mat4 viewMatrix       = glm::lookAt( glm::vec3 { 0, 0, 10 }, glm::vec3 { 0, 0, 0 }, glm::vec3 { 0, 1, 0 } );
    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians( 45.0f ), static_cast<float>( width ) / static_cast<float>( height ), 0.1f, 100.0f );
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

    // Update the MVP matrix in the uniform buffer.
    wgpuQueueWriteBuffer( queue, mvpBuffer, 0, &mvpMatrix, sizeof( mvpMatrix ) );

    render();
}

void destroy()
{
    wgpuSamplerRelease( linearClampSampler );
    wgpuSamplerRelease( linearRepeatSampler );
    wgpuTextureViewRelease( textureView );
    wgpuTextureRelease( dummyTexture );
    wgpuTextureRelease( texture );
    wgpuBufferRelease( vertexBuffer );
    wgpuBufferRelease( indexBuffer );
    wgpuBufferRelease( mvpBuffer );
    wgpuBufferRelease( generateMipsBuffer );
    wgpuBindGroupLayoutRelease( bindGroupLayout );
    wgpuBindGroupLayoutRelease( generateMipsBindGroupLayout );
    wgpuRenderPipelineRelease( pipeline );
    wgpuComputePipelineRelease( generateMipsPipeline );
    wgpuTextureRelease( depthTexture );
    wgpuTextureViewRelease( depthTextureView );
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
