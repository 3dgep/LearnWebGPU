#include "WebGPUlib/Queue.hpp"

#include <WebGPUlib/GraphicsCommandBuffer.hpp>

#include <utility>

using namespace WebGPUlib;

GraphicsCommandBuffer::GraphicsCommandBuffer(
    WGPUCommandEncoder&&    encoder,       // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
    WGPURenderPassEncoder&& passEncoder )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: CommandBuffer( std::move( encoder ) )    // NOLINT(performance-move-const-arg)
, passEncoder { passEncoder }
{}

GraphicsCommandBuffer::~GraphicsCommandBuffer()
{
    if ( passEncoder )
        wgpuRenderPassEncoderRelease( passEncoder );
}

WGPUCommandBuffer GraphicsCommandBuffer::finish()
{
    wgpuRenderPassEncoderEnd( passEncoder );

    WGPUCommandBufferDescriptor commandBufferDescriptor {};
    commandBufferDescriptor.label   = "Graphics Command Buffer";
    return wgpuCommandEncoderFinish( commandEncoder, &commandBufferDescriptor );
}