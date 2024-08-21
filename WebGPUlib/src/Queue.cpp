#include "WebGPUlib/ComputeCommandBuffer.hpp"

#include <WebGPUlib/Buffer.hpp>
#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/GraphicsCommandBuffer.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/Texture.hpp>
#include <WebGPUlib/TextureView.hpp>

#include <cassert>
#include <exception>
#include <vector>

using namespace WebGPUlib;

struct MakeGraphicsCommandBuffer : GraphicsCommandBuffer
{
    MakeGraphicsCommandBuffer( WGPUCommandEncoder&& encoder, WGPURenderPassEncoder&& passEncoder )
    : GraphicsCommandBuffer( std::move( encoder ), std::move( passEncoder ) )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeComputeCommandBuffer : ComputeCommandBuffer
{
    MakeComputeCommandBuffer( WGPUCommandEncoder&& encoder, WGPUComputePassEncoder&& passEncoder )
    : ComputeCommandBuffer { std::move( encoder ), std::move( passEncoder ) }  // NOLINT(performance-move-const-arg)
    {}
};

void Queue::writeBuffer( const Buffer& buffer, const void* data, std::size_t size, uint64_t offset ) const
{
    wgpuQueueWriteBuffer( queue, buffer.getWGPUBuffer(), offset, data, size );
}

static uint32_t bytesPerPixel( WGPUTextureFormat format, WGPUTextureAspect aspect )
{
    switch ( format )
    {
    case WGPUTextureFormat_R8Unorm:
    case WGPUTextureFormat_R8Snorm:
    case WGPUTextureFormat_R8Uint:
    case WGPUTextureFormat_R8Sint:
    case WGPUTextureFormat_Stencil8:
        return 1u;
    // case WGPUTextureFormat_R16Unorm:
    // case WGPUTextureFormat_R16Snorm:
    case WGPUTextureFormat_R16Uint:
    case WGPUTextureFormat_R16Sint:
    case WGPUTextureFormat_R16Float:
    case WGPUTextureFormat_RG8Unorm:
    case WGPUTextureFormat_RG8Snorm:
    case WGPUTextureFormat_RG8Uint:
    case WGPUTextureFormat_RG8Sint:
    case WGPUTextureFormat_Depth16Unorm:
        return 2u;
    case WGPUTextureFormat_R32Float:
    case WGPUTextureFormat_R32Uint:
    case WGPUTextureFormat_R32Sint:
    // case WGPUTextureFormat_RG16Unorm:
    // case WGPUTextureFormat_RG16Snorm:
    case WGPUTextureFormat_RG16Uint:
    case WGPUTextureFormat_RG16Sint:
    case WGPUTextureFormat_RG16Float:
    case WGPUTextureFormat_RGBA8Unorm:
    case WGPUTextureFormat_RGBA8UnormSrgb:
    case WGPUTextureFormat_RGBA8Snorm:
    case WGPUTextureFormat_RGBA8Uint:
    case WGPUTextureFormat_RGBA8Sint:
    case WGPUTextureFormat_BGRA8Unorm:
    case WGPUTextureFormat_BGRA8UnormSrgb:
    case WGPUTextureFormat_RGB10A2Uint:
    case WGPUTextureFormat_RGB10A2Unorm:
    case WGPUTextureFormat_RG11B10Ufloat:
    case WGPUTextureFormat_RGB9E5Ufloat:
    case WGPUTextureFormat_Depth32Float:
        return 4u;
    case WGPUTextureFormat_RG32Float:
    case WGPUTextureFormat_RG32Uint:
    case WGPUTextureFormat_RG32Sint:
    // case WGPUTextureFormat_RGBA16Unorm:
    // case WGPUTextureFormat_RGBA16Snorm:
    case WGPUTextureFormat_RGBA16Uint:
    case WGPUTextureFormat_RGBA16Sint:
    case WGPUTextureFormat_RGBA16Float:
    case WGPUTextureFormat_BC1RGBAUnorm:
    case WGPUTextureFormat_BC1RGBAUnormSrgb:
    case WGPUTextureFormat_BC4RUnorm:
    case WGPUTextureFormat_BC4RSnorm:
    case WGPUTextureFormat_ETC2RGB8Unorm:
    case WGPUTextureFormat_ETC2RGB8UnormSrgb:
    case WGPUTextureFormat_ETC2RGB8A1Unorm:
    case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
    case WGPUTextureFormat_EACR11Unorm:
    case WGPUTextureFormat_EACR11Snorm:
        return 8u;
    case WGPUTextureFormat_RGBA32Float:
    case WGPUTextureFormat_RGBA32Uint:
    case WGPUTextureFormat_RGBA32Sint:
    case WGPUTextureFormat_BC2RGBAUnorm:
    case WGPUTextureFormat_BC2RGBAUnormSrgb:
    case WGPUTextureFormat_BC3RGBAUnorm:
    case WGPUTextureFormat_BC3RGBAUnormSrgb:
    case WGPUTextureFormat_BC5RGUnorm:
    case WGPUTextureFormat_BC5RGSnorm:
    case WGPUTextureFormat_BC6HRGBUfloat:
    case WGPUTextureFormat_BC6HRGBFloat:
    case WGPUTextureFormat_BC7RGBAUnorm:
    case WGPUTextureFormat_BC7RGBAUnormSrgb:
    case WGPUTextureFormat_ETC2RGBA8Unorm:
    case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
    case WGPUTextureFormat_EACRG11Unorm:
    case WGPUTextureFormat_EACRG11Snorm:
    case WGPUTextureFormat_ASTC4x4Unorm:
    case WGPUTextureFormat_ASTC4x4UnormSrgb:
    case WGPUTextureFormat_ASTC5x4Unorm:
    case WGPUTextureFormat_ASTC5x4UnormSrgb:
    case WGPUTextureFormat_ASTC5x5Unorm:
    case WGPUTextureFormat_ASTC5x5UnormSrgb:
    case WGPUTextureFormat_ASTC6x5Unorm:
    case WGPUTextureFormat_ASTC6x5UnormSrgb:
    case WGPUTextureFormat_ASTC6x6Unorm:
    case WGPUTextureFormat_ASTC6x6UnormSrgb:
    case WGPUTextureFormat_ASTC8x5Unorm:
    case WGPUTextureFormat_ASTC8x5UnormSrgb:
    case WGPUTextureFormat_ASTC8x6Unorm:
    case WGPUTextureFormat_ASTC8x6UnormSrgb:
    case WGPUTextureFormat_ASTC8x8Unorm:
    case WGPUTextureFormat_ASTC8x8UnormSrgb:
    case WGPUTextureFormat_ASTC10x5Unorm:
    case WGPUTextureFormat_ASTC10x5UnormSrgb:
    case WGPUTextureFormat_ASTC10x6Unorm:
    case WGPUTextureFormat_ASTC10x6UnormSrgb:
    case WGPUTextureFormat_ASTC10x8Unorm:
    case WGPUTextureFormat_ASTC10x8UnormSrgb:
    case WGPUTextureFormat_ASTC10x10Unorm:
    case WGPUTextureFormat_ASTC10x10UnormSrgb:
    case WGPUTextureFormat_ASTC12x10Unorm:
    case WGPUTextureFormat_ASTC12x10UnormSrgb:
    case WGPUTextureFormat_ASTC12x12Unorm:
    case WGPUTextureFormat_ASTC12x12UnormSrgb:
        return 16u;
    case WGPUTextureFormat_Depth24PlusStencil8:
        switch ( aspect )
        {
        case WGPUTextureAspect_StencilOnly:
            return 1u;
        default:
            break;
        }
        break;
    case WGPUTextureFormat_Depth32FloatStencil8:
        switch ( aspect )
        {
        case WGPUTextureAspect_StencilOnly:
            return 1u;
        case WGPUTextureAspect_DepthOnly:
            return 4u;
        default:
            break;
        }
        break;
    case WGPUTextureFormat_Depth24Plus:
        // case WGPUTextureFormat_R8BG8Biplanar420Unorm:
        // case WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm:
        // case WGPUTextureFormat_R8BG8A8Triplanar420Unorm:
        // case WGPUTextureFormat_R8BG8Biplanar422Unorm:
        // case WGPUTextureFormat_R8BG8Biplanar444Unorm:
        // case WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm:
        // case WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm:
        // case WGPUTextureFormat_External:
        break;
    default:
        break;
    }

    throw std::invalid_argument( "Invalid texture format" );
}

void Queue::writeTexture( Texture& texture, uint32_t mip, const void* data, std::size_t size ) const
{
    constexpr float mipDiv[] = {
        1.0f,      // 2^0
        2.0f,      // 2^1
        4.0f,      // 2^2
        8.0f,      // 2^3
        16.0f,     // 2^4
        32.0f,     // 2^5
        64.0f,     // 2^6
        128.0f,    // 2^7
        256.0f,    // 2^8
        512.0f,    // 2^9
        1024.0f,   // 2^10
        2048.0f,   // 2^11
        4096.0f,   // 2^12
        8192.0f,   // 2^13
        16384.0f,  // 2^14
    };

    auto desc = texture.getWGPUTextureDescriptor();
    assert( mip < desc.mipLevelCount && mip < 14 );

    uint32_t w = static_cast<uint32_t>( std::floor( static_cast<float>( desc.size.width ) / mipDiv[mip] ) );
    uint32_t h = static_cast<uint32_t>( std::floor( static_cast<float>( desc.size.height ) / mipDiv[mip] ) );
    // Width and height must be greater than 0!
    assert( w > 0 );
    assert( h > 0 );

    WGPUTextureDataLayout src {};
    src.offset       = 0;
    src.bytesPerRow  = w * bytesPerPixel( desc.format, WGPUTextureAspect_All );
    src.rowsPerImage = h;
    // The source size and the data size must match.
    assert( static_cast<std::size_t>( src.bytesPerRow ) * src.rowsPerImage == size );

    WGPUImageCopyTexture dst {};
    dst.texture  = texture.getWGPUTexture();
    dst.mipLevel = mip;
    dst.origin   = { 0, 0, 0 };
    dst.aspect   = WGPUTextureAspect_All;

    wgpuQueueWriteTexture( queue, &dst, data, size, &src, &desc.size );
}

std::shared_ptr<GraphicsCommandBuffer> Queue::createGraphicsCommandBuffer( const RenderTarget& renderTarget,
                                                                           ClearFlags          clearFlags,
                                                                           const WGPUColor& clearColor, float depth,
                                                                           uint32_t stencil ) const
{
    WGPUCommandEncoderDescriptor commandEncoderDesc {};
    commandEncoderDesc.label = "Graphics Command Encoder";
    WGPUCommandEncoder commandEncoder =
        wgpuDeviceCreateCommandEncoder( Device::get().getWGPUDevice(), &commandEncoderDesc );

    std::vector<WGPURenderPassColorAttachment> colorAttachments;
    colorAttachments.reserve( 8 );  // Max color attachment points.

    auto& views = renderTarget.getTextureViews();
    for ( int i = 0; i < 8; ++i )
    {
        if ( auto& view = views[i] )
        {
            WGPURenderPassColorAttachment colorAttachment {};
            colorAttachment.view       = view->getWGPUTextureView();
            colorAttachment.loadOp     = ( clearFlags & ClearFlags::Color ) != 0 ? WGPULoadOp_Clear : WGPULoadOp_Load;
            colorAttachment.storeOp    = WGPUStoreOp_Store;
            colorAttachment.clearValue = clearColor;
#ifndef WEBGPU_BACKEND_WGPU
            colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif
            colorAttachments.push_back( colorAttachment );
        }
    }

    auto& depthStencilView = views[static_cast<std::size_t>( AttachmentPoint::DepthStencil )];
    WGPURenderPassDepthStencilAttachment depthStencilAttachment {};
    if ( depthStencilView )
    {
        WGPULoadOp  stencilLoadOp  = WGPULoadOp_Undefined;
        WGPUStoreOp stencilStoreOp = WGPUStoreOp_Undefined;
        if ( depthStencilView->getWGPUTextureViewDescriptor().aspect != WGPUTextureAspect_DepthOnly )
        {
            stencilLoadOp  = ( clearFlags & ClearFlags::Stencil ) != 0 ? WGPULoadOp_Clear : WGPULoadOp_Load;
            stencilStoreOp = WGPUStoreOp_Store;
        }

        depthStencilAttachment.view = depthStencilView->getWGPUTextureView();
        depthStencilAttachment.depthLoadOp =
            ( clearFlags & ClearFlags::Depth ) != 0 ? WGPULoadOp_Clear : WGPULoadOp_Load;
        depthStencilAttachment.depthStoreOp      = WGPUStoreOp_Store;
        depthStencilAttachment.depthClearValue   = depth;
        depthStencilAttachment.depthReadOnly     = false;
        depthStencilAttachment.stencilLoadOp     = stencilLoadOp;
        depthStencilAttachment.stencilStoreOp    = stencilStoreOp;
        depthStencilAttachment.stencilClearValue = stencil;
        depthStencilAttachment.stencilReadOnly   = false;
    }

    WGPURenderPassDescriptor renderPassDesc {};
    renderPassDesc.colorAttachmentCount     = static_cast<uint32_t>( colorAttachments.size() );
    renderPassDesc.colorAttachments         = colorAttachments.data();
    renderPassDesc.depthStencilAttachment   = depthStencilView ? &depthStencilAttachment : nullptr;
    renderPassDesc.timestampWrites          = nullptr;
    WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass( commandEncoder, &renderPassDesc );

    return std::make_shared<MakeGraphicsCommandBuffer>(
        std::move( commandEncoder ), std::move( renderPassEncoder ) );  // NOLINT(performance-move-const-arg)
}

std::shared_ptr<ComputeCommandBuffer> Queue::createComputeCommandBuffer()
{
    // Create a command encoder.
    WGPUCommandEncoderDescriptor commandEncoderDesc {};
    commandEncoderDesc.label = "Compute Command Encoder";
    WGPUCommandEncoder commandEncoder =
        wgpuDeviceCreateCommandEncoder( Device::get().getWGPUDevice(), &commandEncoderDesc );

    // Create a compute pass
    WGPUComputePassDescriptor computePassDesc {};
    computePassDesc.label              = "Compute Pass";
    computePassDesc.timestampWrites    = nullptr;
    WGPUComputePassEncoder passEncoder = wgpuCommandEncoderBeginComputePass( commandEncoder, &computePassDesc );

    return std::make_shared<MakeComputeCommandBuffer>( std::move( commandEncoder ), std::move( passEncoder ) );  // NOLINT(performance-move-const-arg)
}

void Queue::submit( CommandBuffer& commandBuffer )
{
    WGPUCommandBuffer cb = commandBuffer.finish();

    wgpuQueueSubmit( queue, 1, &cb );

    wgpuCommandBufferRelease( cb );
}

Queue::Queue( WGPUQueue&& _queue )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: queue { _queue }
{}

Queue::~Queue()
{
    if ( queue )
        wgpuQueueRelease( queue );
}
