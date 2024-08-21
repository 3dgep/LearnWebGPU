#include <WebGPUlib/BindGroup.hpp>
#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/IndexBuffer.hpp>
#include <WebGPUlib/Mesh.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/Sampler.hpp>
#include <WebGPUlib/Surface.hpp>
#include <WebGPUlib/Texture.hpp>
#include <WebGPUlib/UniformBuffer.hpp>
#include <WebGPUlib/Vertex.hpp>
#include <WebGPUlib/VertexBuffer.hpp>

#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
#endif
#ifdef WEBGPU_BACKEND_WGPU
    #include <webgpu/wgpu.h>  // Include non-standard functions.
#endif

#include <glm/vec3.hpp>
#include <sdl2webgpu.h>
#include <stb_image.h>

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace WebGPUlib;

std::unique_ptr<Device> pDevice { nullptr };

struct MakeQueue : Queue
{
    MakeQueue( WGPUQueue&& queue )
    : Queue( std::move( queue ) )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeSurface : Surface
{
    MakeSurface( WGPUSurface&& surface, const WGPUSurfaceConfiguration& config, SDL_Window* window )
    : Surface( std::move( surface ), config, window )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeVertexBuffer : VertexBuffer
{
    MakeVertexBuffer( WGPUBuffer&& buffer, std::size_t vertexCount, std::size_t vertexStride )
    : VertexBuffer( std::move( buffer ), vertexCount, vertexStride )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeIndexBuffer : IndexBuffer
{
    MakeIndexBuffer( WGPUBuffer&& buffer, std::size_t indexCount, std::size_t indexStride )
    : IndexBuffer( std::move( buffer ), indexCount, indexStride )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeUniformBuffer : UniformBuffer
{
    MakeUniformBuffer( WGPUBuffer&& buffer, std::size_t size )
    : UniformBuffer( std::move( buffer ), size )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeSampler : Sampler
{
    MakeSampler( WGPUSampler&& sampler, const WGPUSamplerDescriptor& samplerDescriptor )
    : Sampler( std::move( sampler ), samplerDescriptor )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeTexture : Texture
{
    MakeTexture( WGPUTexture&& texture, const WGPUTextureDescriptor& textureDescriptor )
    : Texture( std::move( texture ), textureDescriptor )
    {}
};

void Device::create( SDL_Window* window )
{
    assert( !pDevice );
    pDevice = std::unique_ptr<Device>( new Device( window ) );
}

void Device::destroy()
{
    pDevice.reset();
}

Device& Device::get()
{
    assert( pDevice );
    return *pDevice;
}

Device::Device( SDL_Window* window )
{
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

    // Used by the requestAdapterCallback function to store the adapter and to notify
    // us when the request is complete.
    struct AdapterData
    {
        WGPUAdapter adapter = nullptr;
        bool        done    = false;
    } adapterData;

    WGPURequestAdapterOptions requestAdapterOptions {};
    requestAdapterOptions.backendType     = WGPUBackendType_Undefined;  // WGPUBackendType_Vulkan;
    requestAdapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;

    wgpuInstanceRequestAdapter(
        instance, &requestAdapterOptions,
        []( WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userData ) {
            auto& data = *static_cast<AdapterData*>( userData );
            if ( status == WGPURequestAdapterStatus_Success )
            {
                data.adapter = adapter;
            }
            else
            {
                std::cerr << "Failed to request adapter: " << message << std::endl;
            }
            data.done = true;
        },
        &adapterData );

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while ( !adapterData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( adapterData.done );
    adapter = adapterData.adapter;

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor {};
    deviceDescriptor.label                    = "WebGPUlib";  // You can use anything here.
    deviceDescriptor.requiredFeatureCount     = 0;            // We don't require any extra features.
    deviceDescriptor.requiredFeatures         = nullptr;
    deviceDescriptor.requiredLimits           = nullptr;  // We don't require any specific limits.
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.defaultQueue.label       = "Queue";  // You can use anything here.
    deviceDescriptor.deviceLostCallback       = onDeviceLostCallback;

    struct DeviceData
    {
        WGPUDevice device = nullptr;
        bool       done   = false;
    } deviceData;

    wgpuAdapterRequestDevice(
        adapter, &deviceDescriptor,
        []( WGPURequestDeviceStatus _status, WGPUDevice _device, const char* _message, void* _userData ) {
            auto& data = *static_cast<DeviceData*>( _userData );
            if ( _status == WGPURequestDeviceStatus_Success )
            {
                data.device = _device;
            }
            else
            {
                std::cerr << "Failed to request device: " << _message << std::endl;
            }
            data.done = true;
        },
        &deviceData );

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while ( !deviceData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( deviceData.done );
    device = deviceData.device;

    if ( !device )
    {
        std::cerr << "Failed to create device." << std::endl;
        return;
    }

    // Set the uncaptured error callback.
    wgpuDeviceSetUncapturedErrorCallback( device, onUncapturedErrorCallback, nullptr );

    // Configure the surface.
    WGPUSurface _surface = SDL_GetWGPUSurface( instance, window );

    if ( !_surface )
    {
        std::cerr << "Failed to get the surface." << std::endl;
        return;
    }

    int windowWidth, windowHeight;
    SDL_GetWindowSize( window, &windowWidth, &windowHeight );

    WGPUSurfaceCapabilities surfaceCapabilities {};
    wgpuSurfaceGetCapabilities( _surface, adapter, &surfaceCapabilities );
    WGPUTextureFormat surfaceFormat = surfaceCapabilities.formats[0];

    // Set the surface configuration.
    WGPUSurfaceConfiguration surfaceConfiguration {};
    surfaceConfiguration.device          = device;
    surfaceConfiguration.format          = surfaceFormat;
    surfaceConfiguration.usage           = WGPUTextureUsage_RenderAttachment;
    surfaceConfiguration.viewFormatCount = 0;
    surfaceConfiguration.viewFormats     = nullptr;
    surfaceConfiguration.alphaMode       = WGPUCompositeAlphaMode_Auto;
    surfaceConfiguration.width           = windowWidth;
    surfaceConfiguration.height          = windowHeight;
    surfaceConfiguration.presentMode     = WGPUPresentMode_Fifo;  // This must be Fifo on Emscripten.

    wgpuSurfaceConfigure( _surface, &surfaceConfiguration );

    surface = std::make_shared<MakeSurface>( std::move( _surface ),  // NOLINT(performance-move-const-arg)
                                             surfaceConfiguration, window );

    // Get the device queue.
    WGPUQueue _queue = wgpuDeviceGetQueue( device );

    if ( !_queue )
    {
        std::cerr << "Failed to get device queue." << std::endl;
        return;
    }

    queue = std::make_shared<MakeQueue>( std::move( _queue ) );  // NOLINT(performance-move-const-arg)
}

Device::~Device()
{
    surface.reset();
    queue.reset();

    if ( device )
        wgpuDeviceRelease( device );

    if ( adapter )
        wgpuAdapterRelease( adapter );

    if ( instance )
        wgpuInstanceRelease( instance );
}

std::shared_ptr<Queue> Device::getQueue() const
{
    return queue;
}

std::shared_ptr<Surface> Device::getSurface() const
{
    return surface;
}

static void reverseWinding( std::vector<VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices )
{
    assert( ( indices.size() % 3 ) == 0 );
    for ( auto it = indices.begin(); it != indices.end(); it += 3 )
    {
        std::swap( *it, *( it + 2 ) );
    }

    for ( auto& vertex: vertices )
    {
        vertex.texCoord.x = ( 1.0f - vertex.texCoord.x );
    }
}

std::shared_ptr<Mesh> Device::createCube( float size, bool _reverseWinding ) const
{
    float s = size * 0.5f;

    // 8 cube vertices.
    const glm::vec3 p[] = { { s, s, -s }, { s, s, s },   { s, -s, s },   { s, -s, -s },
                            { -s, s, s }, { -s, s, -s }, { -s, -s, -s }, { -s, -s, s } };

    // 6 face normals.
    constexpr glm::vec3 n[] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };

    // 4 unique texture coordinates.
    constexpr glm::vec3 t[] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 } };

    // Indices for the vertex positions.
    uint16_t i[24] = {
        0, 1, 2, 3,  // +X
        4, 5, 6, 7,  // -X
        4, 1, 0, 5,  // +Y
        2, 7, 6, 3,  // -Y
        1, 4, 7, 2,  // +Z
        5, 0, 3, 6   // -Z
    };

    std::vector<VertexPositionNormalTexture> vertices;
    std::vector<uint16_t>                    indices;

    for ( uint16_t f = 0; f < 6; ++f )  // For each face of the cube.
    {
        // Four vertices per face.
        vertices.emplace_back( p[i[f * 4 + 0]], n[f], t[0] );
        vertices.emplace_back( p[i[f * 4 + 1]], n[f], t[1] );
        vertices.emplace_back( p[i[f * 4 + 2]], n[f], t[2] );
        vertices.emplace_back( p[i[f * 4 + 3]], n[f], t[3] );

        // First triangle.
        indices.emplace_back( f * 4 + 0 );
        indices.emplace_back( f * 4 + 1 );
        indices.emplace_back( f * 4 + 2 );

        // Second triangle
        indices.emplace_back( f * 4 + 2 );
        indices.emplace_back( f * 4 + 3 );
        indices.emplace_back( f * 4 + 0 );
    }

    if ( _reverseWinding )
        reverseWinding( vertices, indices );

    auto vertexBuffer = createVertexBuffer( vertices );
    auto indexBuffer  = createIndexBuffer( indices );

    return std::make_shared<Mesh>( vertexBuffer, indexBuffer );
}

std::shared_ptr<Texture> Device::createTexture( const WGPUTextureDescriptor& textureDescriptor )
{
    WGPUTexture texture = wgpuDeviceCreateTexture( device, &textureDescriptor );

    return std::make_shared<MakeTexture>( std::move( texture ),
                                          textureDescriptor );  // NOLINT(performance-move-const-arg)
}

std::shared_ptr<Texture> Device::loadTexture( const std::filesystem::path& filePath )
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

    auto tex = std::make_shared<MakeTexture>( std::move( texture ), textureDesc );

    queue->writeTexture( *tex, 0, data, ( static_cast<std::size_t>( width ) * height * 4u ) );

    stbi_image_free( data );

    generateMips( *tex );

    return tex;
}

void Device::generateMips( const Texture& texture )
{
    // TODO:
}

std::shared_ptr<VertexBuffer> Device::createVertexBuffer( const void* vertexData, std::size_t vertexCount,
                                                          std::size_t vertexStride ) const
{
    std::size_t          size = vertexCount * vertexStride;
    WGPUBufferDescriptor bufferDescriptor {};
    bufferDescriptor.size  = size;
    bufferDescriptor.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    WGPUBuffer buffer      = wgpuDeviceCreateBuffer( device, &bufferDescriptor );

    auto vertexBuffer = std::make_shared<MakeVertexBuffer>( std::move( buffer ),  // NOLINT(performance-move-const-arg)
                                                            vertexCount, vertexStride );

    if ( vertexData )
        queue->writeBuffer( *vertexBuffer, vertexData, size );

    return vertexBuffer;
}

std::shared_ptr<IndexBuffer> Device::createIndexBuffer( const void* indexData, std::size_t indexCount,
                                                        std::size_t indexStride ) const
{
    std::size_t          size = indexCount * indexStride;
    WGPUBufferDescriptor bufferDescriptor {};
    bufferDescriptor.size  = size;
    bufferDescriptor.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    WGPUBuffer buffer      = wgpuDeviceCreateBuffer( device, &bufferDescriptor );

    auto indexBuffer = std::make_shared<MakeIndexBuffer>( std::move( buffer ),  // NOLINT(performance-move-const-arg)
                                                          indexCount, indexStride );

    if ( indexData )
        queue->writeBuffer( *indexBuffer, indexData, size );

    return indexBuffer;
}

std::shared_ptr<UniformBuffer> Device::createUniformBuffer( const void* data, std::size_t size ) const
{
    WGPUBufferDescriptor bufferDescriptor {};
    bufferDescriptor.size             = size;
    bufferDescriptor.usage            = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    bufferDescriptor.mappedAtCreation = false;
    WGPUBuffer buffer                 = wgpuDeviceCreateBuffer( device, &bufferDescriptor );

    auto uniformBuffer =
        std::make_shared<MakeUniformBuffer>( std::move( buffer ), size );  // NOLINT(performance-move-const-arg)

    if ( data )
        queue->writeBuffer( *uniformBuffer, data, size );

    return uniformBuffer;
}

std::shared_ptr<Sampler> Device::createSampler( const WGPUSamplerDescriptor& samplerDescriptor ) const
{
    WGPUSampler sampler = wgpuDeviceCreateSampler( device, &samplerDescriptor );

    return std::make_shared<MakeSampler>( std::move( sampler ),  // NOLINT(performance-move-const-arg)
                                          samplerDescriptor );
}

void Device::poll( bool sleep )
{
#if defined( WEBGPU_BACKEND_DAWN )
    wgpuDeviceTick( device );
#elif defined( WEBGPU_BACKEND_WGPU )
    wgpuDevicePoll( device, false, nullptr );
#elif defined( WEBGPU_BACKEND_EMSCRIPTEN )
    if ( sleep )
    {
        emscripten_sleep( 100 );
    }
#endif
}

void Device::onDeviceLostCallback( WGPUDeviceLostReason reason, char const* message, void* userdata )
{
    std::cerr << "Device lost: " << std::hex << reason << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

void Device::onUncapturedErrorCallback( WGPUErrorType type, const char* message, void* userdata )
{
    std::cerr << "Uncaptured device error: " << std::hex << type << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}