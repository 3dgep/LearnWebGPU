#include <WebGPUlib/GraphicsCommandBuffer.hpp>
#include <WebGPUlib/GraphicsPipelineState.hpp>
#include <WebGPUlib/IndexBuffer.hpp>
#include <WebGPUlib/Mesh.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/VertexBuffer.hpp>

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

void GraphicsCommandBuffer::setGraphicsPipeline( GraphicsPipelineState& pipeline )
{
    wgpuRenderPassEncoderSetPipeline( passEncoder, pipeline.getWGPURenderPipeline() );
}

void WebGPUlib::GraphicsCommandBuffer::draw( const Mesh& mesh )
{

    auto vertexBuffers = mesh.getVertexBuffers();
    auto indexBuffer   = mesh.getIndexBuffer();

    for ( uint32_t i = 0; i < vertexBuffers.size(); ++i )
    {
        auto vertexBuffer = vertexBuffers[i];
        if ( vertexBuffer )
        {
            wgpuRenderPassEncoderSetVertexBuffer( passEncoder, i, vertexBuffer->getWGPUBuffer(), 0,
                                                  vertexBuffer->getSize() );
        }
    }

    if ( indexBuffer )
    {
        WGPUIndexFormat indexFormat = WGPUIndexFormat_Undefined;
        switch ( indexBuffer->getIndexStride() )
        {
        case 2:
            indexFormat = WGPUIndexFormat_Uint16;
            break;
        case 4:
            indexFormat = WGPUIndexFormat_Uint32;
            break;
        };

        wgpuRenderPassEncoderSetIndexBuffer( passEncoder, indexBuffer->getWGPUBuffer(), indexFormat, 0,
                                             indexBuffer->getSize() );
        wgpuRenderPassEncoderDrawIndexed( passEncoder, indexBuffer->getIndexCount(), 1, 0, 0, 0 );
    }
    else
    {
        auto vertexBuffer = vertexBuffers[0];
        if ( vertexBuffer )
        {
            wgpuRenderPassEncoderDraw( passEncoder, vertexBuffer->getVertexCount(), 1, 0, 0 );
        }
    }
}

WGPUCommandBuffer GraphicsCommandBuffer::finish()
{
    wgpuRenderPassEncoderEnd( passEncoder );

    WGPUCommandBufferDescriptor commandBufferDescriptor {};
    commandBufferDescriptor.label = "Graphics Command Buffer";
    return wgpuCommandEncoderFinish( commandEncoder, &commandBufferDescriptor );
}