#include <WebGPUlib/Buffer.hpp>
#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/GraphicsCommandBuffer.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/Texture.hpp>
#include <WebGPUlib/TextureView.hpp>

#include <vector>

using namespace WebGPUlib;

struct MakeGraphicsCommandBuffer : GraphicsCommandBuffer
{
    MakeGraphicsCommandBuffer( WGPUCommandEncoder&& encoder, WGPURenderPassEncoder&& passEncoder )
    : GraphicsCommandBuffer( std::move( encoder ), std::move( passEncoder ) )  // NOLINT(performance-move-const-arg)
    {}
};

void Queue::writeBuffer( const std::shared_ptr<Buffer>& buffer, const void* data, std::size_t size ) const
{
    wgpuQueueWriteBuffer( queue, buffer->getWGPUBuffer(), 0, data, size );
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
        depthStencilAttachment.view = depthStencilView->getWGPUTextureView();
        depthStencilAttachment.depthLoadOp =
            ( clearFlags & ClearFlags::Depth ) != 0 ? WGPULoadOp_Clear : WGPULoadOp_Load;
        depthStencilAttachment.depthStoreOp    = WGPUStoreOp_Store;
        depthStencilAttachment.depthClearValue = depth;
        depthStencilAttachment.depthReadOnly   = false;
        depthStencilAttachment.stencilLoadOp =
            ( clearFlags & ClearFlags::Stencil ) != 0 ? WGPULoadOp_Clear : WGPULoadOp_Load;
        depthStencilAttachment.stencilStoreOp    = WGPUStoreOp_Store;
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

void Queue::submit( std::shared_ptr<CommandBuffer> commandBuffer )
{
    WGPUCommandBuffer cb = commandBuffer->finish();

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
