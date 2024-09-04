#include <WebGPUlib/BindGroup.hpp>
#include <WebGPUlib/ComputeCommandBuffer.hpp>
#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/GenerateMipsPipelineState.hpp>
#include <WebGPUlib/IndexBuffer.hpp>
#include <WebGPUlib/Mesh.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/Sampler.hpp>
#include <WebGPUlib/Scene.hpp>
#include <WebGPUlib/SceneNode.hpp>
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

#include "WebGPUlib/Material.hpp"

#include <tiny_obj_loader.h>

#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/vec3.hpp>
#include <sdl2webgpu.h>
#include <stb_image.h>

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace WebGPUlib;
namespace fs = std::filesystem;

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

template<typename T>
constexpr T DivideByMultiple( T value, size_t alignment )
{
    return static_cast<T>( ( value + alignment - 1 ) / alignment );
}

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
    requestAdapterOptions.backendType     = WGPUBackendType_Undefined;
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
#ifdef __EMSCRIPTEN__
    surfaceConfiguration.presentMode = WGPUPresentMode_Fifo;  // This must be Fifo on Emscripten.
#else
    surfaceConfiguration.presentMode = WGPUPresentMode_Mailbox;
#endif

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
    constexpr glm::vec3 t[] = { { 1, 0, 0 }, { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 } };

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

std::shared_ptr<Texture> Device::loadTexture( const std::filesystem::path& _filePath )
{
    auto filePath = _filePath.string();
    // Replace double backslashes in the file path.
    // This is required on POSIX systems (like Emscripten).
    std::replace( filePath.begin(), filePath.end(), '\\', '/' );

    if ( !std::filesystem::exists( filePath ) || !std::filesystem::is_regular_file( filePath ) )
    {
        std::cerr << "ERROR: File not found or is not a regular file: " << filePath << std::endl;
        return nullptr;
    }

    // Load the texture
    int            width, height, channels;
    unsigned char* data = stbi_load( filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha );

    if ( !data )
    {
        std::cerr << "ERROR: Failed to load texture: " << filePath << std::endl;
        return nullptr;
    }

    const WGPUExtent3D textureSize { static_cast<uint32_t>( width ), static_cast<uint32_t>( height ), 1u };

    // Create the texture object.
    WGPUTextureDescriptor textureDesc {};
    textureDesc.label       = _filePath.filename().string().c_str();
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

    auto tex =
        std::make_shared<MakeTexture>( std::move( texture ), textureDesc );  // NOLINT(performance-move-const-arg)

    // Copy mip level 0.
    queue->writeTexture( *tex, 0, data, ( static_cast<std::size_t>( width ) * height * 4u ) );

    stbi_image_free( data );

    generateMips( *tex );

    std::cout << "INFO: Loaded texture: " << filePath << std::endl;

    return tex;
}

void Device::generateMips( Texture& texture )
{
    if ( !generateMipsPipelineState )
        generateMipsPipelineState = std::make_unique<GenerateMipsPipelineState>();

    auto desc = texture.getWGPUTextureDescriptor();

    auto commandBuffer = queue->createComputeCommandBuffer();

    commandBuffer->setComputePipeline( *generateMipsPipelineState );

    // Setup a temporary uniform buffer for uploading the mip info.
    auto uniformBuffer = createUniformBuffer( nullptr, 4096u );  // 4k should be more than enough?

    // Create a dummy texture to pad any unused mips.
    // Create a placeholder texture to use during mipmap generation.
    WGPUTextureDescriptor dummyTextureDesc {};
    dummyTextureDesc.label         = "Mip map placeholder texture";
    dummyTextureDesc.usage         = WGPUTextureUsage_StorageBinding;
    dummyTextureDesc.dimension     = WGPUTextureDimension_2D;
    dummyTextureDesc.size          = { 16, 16, 1 };
    dummyTextureDesc.format        = WGPUTextureFormat_RGBA8Unorm;
    dummyTextureDesc.mipLevelCount = 4;
    dummyTextureDesc.sampleCount   = 1;
    auto dummyTexture              = createTexture( dummyTextureDesc );

    // Create a sampler
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
    auto sampler                         = createSampler( linearClampSamplerDesc );

    // Bind the sampler
    commandBuffer->bindSampler( 0, 6, *sampler );

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
        uint32_t bufferOffset = 256 * pass;
        queue->writeBuffer( *uniformBuffer, &mip, sizeof( Mip ), bufferOffset );

        commandBuffer->bindBuffer( 0, 0, *uniformBuffer, bufferOffset, sizeof( Mip ) );

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
        auto srcTextureView                = texture.getView( &srcTextureViewDesc );

        commandBuffer->bindTexture( 0, 1, *srcTextureView );

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
            auto dstMipView                = texture.getView( &dstMipViewDesc );

            commandBuffer->bindTexture( 0, 2 + dstMip, *dstMipView );
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
            auto dstMipView                = dummyTexture->getView( &dstMipViewDesc );

            commandBuffer->bindTexture( 0, 2 + dstMip, *dstMipView );
        }

        commandBuffer->dispatch( DivideByMultiple( dstWidth, 8 ), DivideByMultiple( dstHeight, 8 ) );

        srcMip += mipCount;
    }

    queue->submit( *commandBuffer );
}

#if 0
glm::vec4 parseColor( const tinyobj::real_t color[3] )
{
    return { color[0], color[1], color[2], 1.0f };
}

std::shared_ptr<Scene> Device::loadScene( const std::filesystem::path& filePath )
{
    if ( !exists( filePath ) || !is_regular_file( filePath ) )
    {
        std::cerr << "ERROR: Failed to load scene file: " << filePath << std::endl;
        return nullptr;
    }

    tinyobj::ObjReaderConfig config {};
    tinyobj::ObjReader       reader;

    if ( !reader.ParseFromFile( filePath.string(), config ) )
    {
        if ( !reader.Error().empty() )
        {
            std::cerr << "ERROR: Failed to parse model file: " << filePath << std::endl;
            std::cerr << reader.Error() << std::endl;
        }
        return nullptr;
    }

    if ( !reader.Warning().empty() )
    {
        std::cout << "WARNING: Warning parsing model file: " << filePath << std::endl;
        std::cout << reader.Warning() << std::endl;
    }

    auto rootNode = std::make_shared<SceneNode>();

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& mats   = reader.GetMaterials();

    auto parentPath = filePath.parent_path();

    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve( mats.size() );

    // Loop over materials
    for ( auto& m: mats )
    {
        glm::vec4 diffuse       = parseColor( m.diffuse );
        glm::vec4 specular      = parseColor( m.specular );
        glm::vec4 ambient       = parseColor( m.ambient );
        glm::vec4 emissive      = parseColor( m.emission );
        float     specularPower = m.shininess;

        auto diffuseTexture  = m.diffuse_texname.empty() ? nullptr : loadTexture( parentPath / m.diffuse_texname );
        auto alphaTexture    = m.alpha_texname.empty() ? nullptr : loadTexture( parentPath / m.alpha_texname );
        auto specularTexture = m.specular_texname.empty() ? nullptr : loadTexture( parentPath / m.specular_texname );
        auto specularPowerTexture =
            m.specular_highlight_texname.empty() ? nullptr : loadTexture( parentPath / m.specular_highlight_texname );
        auto normalTexture   = m.bump_texname.empty() ? nullptr : loadTexture( parentPath / m.bump_texname );
        auto ambientTexture  = m.ambient_texname.empty() ? nullptr : loadTexture( parentPath / m.ambient_texname );
        auto emissiveTexture = m.emissive_texname.empty() ? nullptr : loadTexture( parentPath / m.emissive_texname );

        std::shared_ptr<Material> material = std::make_shared<Material>();
        material->setDiffuse( diffuse );
        material->setSpecular( specular );
        material->setSpecularPower( specularPower );
        material->setAmbient( ambient );
        material->setEmissive( emissive );
        material->setTexture( TextureSlot::Diffuse, diffuseTexture );
        material->setTexture( TextureSlot::Opacity, alphaTexture );
        material->setTexture( TextureSlot::Specular, specularTexture );
        material->setTexture( TextureSlot::SpecularPower, specularPowerTexture );
        material->setTexture( TextureSlot::Normal, normalTexture );
        material->setTexture( TextureSlot::Ambient, ambientTexture );
        material->setTexture( TextureSlot::Emissive, emissiveTexture );

        materials.push_back( material );
    }

    // Loop over shapes
    for ( auto& s: shapes )
    {
        auto& m = s.mesh;

        std::vector<VertexPositionNormalTexture> vertices;
        vertices.reserve( m.num_face_vertices.size() * 3 );

        size_t indexOffset = 0;

        for ( auto numVerts: m.num_face_vertices )
        {
            // We only want 3 vertices per face.
            assert( numVerts == 3 );

            // Loop over the vertices of the triangle.
            for ( size_t v = 0; v < numVerts; ++v )
            {
                VertexPositionNormalTexture vert {};

                auto idx = m.indices[indexOffset + v];

                vert.position.x = attrib.vertices[idx.vertex_index * 3 + 0];
                vert.position.y = attrib.vertices[idx.vertex_index * 3 + 1];
                vert.position.z = attrib.vertices[idx.vertex_index * 3 + 2];

                if ( idx.normal_index >= 0 )
                {
                    vert.normal.x = attrib.normals[idx.normal_index * 3 + 0];
                    vert.normal.y = attrib.normals[idx.normal_index * 3 + 1];
                    vert.normal.z = attrib.normals[idx.normal_index * 3 + 2];
                }

                if ( idx.texcoord_index >= 0 )
                {
                    vert.texCoord.x = attrib.texcoords[idx.texcoord_index * 2 + 0];
                    vert.texCoord.y = 1.0f - attrib.texcoords[idx.texcoord_index * 2 + 1];
                }

                vertices.push_back( vert );
            }

            indexOffset += numVerts;
        }

        auto vertexBuffer = createVertexBuffer( vertices );
        auto mesh         = std::make_shared<Mesh>( vertexBuffer );

        if ( !m.material_ids.empty() )
        {
            // Per-face materials are not supported.
            auto materialId = m.material_ids[0];
            if ( materialId >= 0 && materialId < static_cast<int>( materials.size() ) )
            {
                mesh->setMaterial( materials[materialId] );
            }
        }

        rootNode->addMesh( mesh );
    }

    return std::make_shared<Scene>( rootNode );
}
#else

std::shared_ptr<SceneNode> importSceneNode( const aiNode* aiNode, std::shared_ptr<SceneNode> parent,
                                            const std::vector<std::shared_ptr<Mesh>>& meshes )
{
    if (!aiNode)
    {
        return nullptr;
    }

    glm::mat4 transform {
        aiNode->mTransformation.a1, aiNode->mTransformation.a2, aiNode->mTransformation.a3, aiNode->mTransformation.a4,
        aiNode->mTransformation.b1, aiNode->mTransformation.b2, aiNode->mTransformation.b3, aiNode->mTransformation.b4,
        aiNode->mTransformation.c1, aiNode->mTransformation.c2, aiNode->mTransformation.c3, aiNode->mTransformation.c4,
        aiNode->mTransformation.d1, aiNode->mTransformation.d2, aiNode->mTransformation.d3, aiNode->mTransformation.d4,
    };

    auto node = std::make_shared<SceneNode>( transform );
    node->setParent( parent );

    for ( unsigned int i = 0; i < aiNode->mNumMeshes; ++i )
    {
        node->addMesh( meshes[aiNode->mMeshes[i]] );
    }

    // Import children.
    for (unsigned int i = 0; i < aiNode->mNumChildren; ++i)
    {
        auto child = importSceneNode( aiNode->mChildren[i], node, meshes );
        node->addChild( child );
    }

    return node;
}

std::shared_ptr<Scene> Device::loadScene( const std::filesystem::path& filePath )
{
    fs::path parentPath = filePath.parent_path();

    fs::path exportPath = filePath;
    exportPath.replace_extension( "assbin" );

    Assimp::Importer importer;
    const aiScene*   scene = nullptr;

    if ( exists( exportPath ) && is_regular_file( exportPath ) )
    {
        scene = importer.ReadFile( exportPath.string(), aiProcess_GenBoundingBoxes );
    }
    else
    {
        // File has not been preprocessed yet. Import and processes the file.
        importer.SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f );
        importer.SetPropertyInteger( AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE );

        unsigned int preprocessFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_OptimizeGraph |
                                       aiProcess_ConvertToLeftHanded | aiProcess_GenBoundingBoxes;
        scene = importer.ReadFile( filePath.string(), preprocessFlags );

        if ( scene )
        {
            // Export the preprocessed scene file for faster loading next time.
            Assimp::Exporter exporter;
            exporter.Export( scene, "assbin", exportPath.string(), 0 );
        }
    }

    if ( !scene )
    {
        return nullptr;
    }

    // Import materials.
    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve( scene->mNumMaterials );

    for ( unsigned int i = 0; i < scene->mNumMaterials; ++i )
    {
        const aiMaterial*         aiMaterial = scene->mMaterials[i];
        std::shared_ptr<Material> material   = std::make_shared<Material>();

        aiString  texturePath;
        aiColor4D ambientColor;
        aiColor4D emissiveColor;
        aiColor4D diffuseColor;
        aiColor4D specularColor;
        aiColor4D reflectiveColor;
        float     shininess;
        float     opacity;
        float     indexOfRefraction;
        float     bumpIntensity;

        if ( aiMaterial->Get( AI_MATKEY_COLOR_AMBIENT, ambientColor ) == aiReturn_SUCCESS )
        {
            material->setAmbient( { ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a } );
        }
        if ( aiMaterial->Get( AI_MATKEY_COLOR_EMISSIVE, emissiveColor ) == aiReturn_SUCCESS )
        {
            material->setEmissive( { emissiveColor.r, emissiveColor.g, emissiveColor.b, emissiveColor.a } );
        }
        if ( aiMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, diffuseColor ) == aiReturn_SUCCESS )
        {
            material->setDiffuse( { diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a } );
        }
        if ( aiMaterial->Get( AI_MATKEY_COLOR_SPECULAR, specularColor ) == aiReturn_SUCCESS )
        {
            material->setSpecular( { specularColor.r, specularColor.g, specularColor.b, specularColor.a } );
        }
        if ( aiMaterial->Get( AI_MATKEY_COLOR_REFLECTIVE, reflectiveColor ) == aiReturn_SUCCESS )
        {
            material->setReflectance( { reflectiveColor.r, reflectiveColor.g, reflectiveColor.b, reflectiveColor.a } );
        }
        if ( aiMaterial->Get( AI_MATKEY_SHININESS, shininess ) == aiReturn_SUCCESS )
        {
            material->setSpecularPower( shininess );
        }
        if ( aiMaterial->Get( AI_MATKEY_OPACITY, opacity ) == aiReturn_SUCCESS )
        {
            material->setOpacity( opacity );
        }
        if ( aiMaterial->Get( AI_MATKEY_REFRACTI, indexOfRefraction ) == aiReturn_SUCCESS )
        {
            material->setIndexOfRefraction( indexOfRefraction );
        }
        if ( aiMaterial->Get( AI_MATKEY_BUMPSCALING, bumpIntensity ) == aiReturn_SUCCESS )
        {
            material->setBumpIntensity( bumpIntensity );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_AMBIENT ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_AMBIENT, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Ambient, texture );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_EMISSIVE ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_EMISSIVE, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Emissive, texture );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_DIFFUSE ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Diffuse, texture );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_SPECULAR ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_SPECULAR, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Specular, texture );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_SHININESS ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_SHININESS, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::SpecularPower, texture );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_OPACITY ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_OPACITY, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Opacity, texture );
        }
        if ( aiMaterial->GetTextureCount( aiTextureType_NORMALS ) > 0 &&
             aiMaterial->GetTexture( aiTextureType_NORMALS, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Normal, texture );
        }
        else if ( aiMaterial->GetTextureCount( aiTextureType_HEIGHT ) > 0 &&
                  aiMaterial->GetTexture( aiTextureType_HEIGHT, 0, &texturePath ) == aiReturn_SUCCESS )
        {
            auto texture = loadTexture( parentPath / texturePath.C_Str() );
            material->setTexture( TextureSlot::Bump, texture );
        }

        materials.emplace_back( std::move(material) );
    }

    // Import meshes.
    std::vector<std::shared_ptr<Mesh>> meshes;
    meshes.reserve( scene->mNumMeshes );

    for ( unsigned int m = 0; m < scene->mNumMeshes; ++m )
    {
        const aiMesh* aiMesh = scene->mMeshes[m];

        std::shared_ptr<Mesh>                                    mesh = std::make_shared<Mesh>();
        std::vector<VertexPositionNormalTexture> vertexData { aiMesh->mNumVertices };

        assert( aiMesh->mMaterialIndex < materials.size() );
        mesh->setMaterial( materials[aiMesh->mMaterialIndex] );

        if ( aiMesh->HasPositions() )
        {
            for ( unsigned int v = 0; v < aiMesh->mNumVertices; ++v )
            {
                aiVector3D p           = aiMesh->mVertices[v];
                vertexData[v].position = { p.x, p.y, p.z };
            }
        }
        if ( aiMesh->HasNormals() )
        {
            for ( unsigned int v = 0; v < aiMesh->mNumVertices; ++v )
            {
                aiVector3D n         = aiMesh->mNormals[v];
                vertexData[v].normal = { n.x, n.y, n.z };
            }
        }
        //if ( aiMesh->HasTangentsAndBitangents() )
        //{
        //    for ( unsigned int v = 0; v < aiMesh->mNumVertices; ++v )
        //    {
        //        aiVector3D t            = aiMesh->mTangents[v];
        //        aiVector3D b            = aiMesh->mBitangents[v];
        //        vertexData[v].tangent   = { t.x, t.y, t.z };
        //        vertexData[v].bitangent = { b.x, b.y, b.z };
        //    }
        //}
        if ( aiMesh->HasTextureCoords( 0 ) )
        {
            for ( unsigned int v = 0; v < aiMesh->mNumVertices; ++v )
            {
                aiVector3D uv          = aiMesh->mTextureCoords[0][v];
                vertexData[v].texCoord = { uv.x, uv.y, uv.z };
            }
        }

        auto vertexBuffer = createVertexBuffer( vertexData );
        mesh->setVertexBuffer( 0, vertexBuffer );

        // Extract index buffer.
        if ( aiMesh->HasFaces() )
        {
            std::vector<unsigned int> indices;
            indices.reserve( aiMesh->mNumFaces * 3 );

            for ( unsigned int f = 0; f < aiMesh->mNumFaces; ++f )
            {
                const aiFace& face = aiMesh->mFaces[f];

                // We only care about triangular faces.
                if ( face.mNumIndices == 3 )
                {
                    indices.push_back( face.mIndices[0] );
                    indices.push_back( face.mIndices[1] );
                    indices.push_back( face.mIndices[2] );
                }
            }

            if ( !indices.empty() )
            {
                auto indexBuffer = createIndexBuffer( indices );
                mesh->setIndexBuffer( indexBuffer );
            }
        }

        meshes.emplace_back( std::move(mesh) );
    }

    auto rootNode = importSceneNode( scene->mRootNode, nullptr, meshes );

    return std::make_shared<Scene>( rootNode );
}
#endif

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