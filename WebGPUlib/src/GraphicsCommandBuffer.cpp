#include <WebGPUlib/GraphicsCommandBuffer.hpp>

using namespace WebGPUlib;

GraphicsCommandBuffer::GraphicsCommandBuffer(
    WGPUCommandEncoder&&    encoder,       // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
    WGPURenderPassEncoder&& passEncoder )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: commandEncoder { encoder }
, passEncoder { passEncoder }
{}

GraphicsCommandBuffer::~GraphicsCommandBuffer()
{
    if ( passEncoder )
        wgpuRenderPassEncoderRelease( passEncoder );

    if ( commandEncoder )
        wgpuCommandEncoderRelease( commandEncoder );
}