#include "WebGPUlib/BindGroup.hpp"

#include <WebGPUlib/GraphicsCommandBuffer.hpp>
#include <WebGPUlib/GraphicsPipelineState.hpp>
#include <WebGPUlib/IndexBuffer.hpp>
#include <WebGPUlib/Mesh.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/VertexBuffer.hpp>

#include <iostream>
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

void GraphicsCommandBuffer::setBindGroup( uint32_t groupIndex, const BindGroup& _bindGroup )
{
    if ( currentPipelineState )
    {
        auto bindGroupLayout = currentPipelineState->getWGPUBindGroupLayout( groupIndex );
        auto bindGroup       = _bindGroup.getWGPUBindGroup( bindGroupLayout );
        wgpuRenderPassEncoderSetBindGroup( passEncoder, groupIndex, bindGroup, 0, nullptr );
    }
    else
    {
        std::cerr
            << "ERROR (GraphicsCommandBuffer::setBindGroup): No graphics pipeline set. Make sure to set the pipeline before drawing."
            << std::endl;
    }
}

void GraphicsCommandBuffer::setGraphicsPipeline( GraphicsPipelineState& pipeline )
{
    // Keep track of the currently bound pipeline state.
    currentPipelineState = &pipeline;

    pipeline.bind( *this );
}

void GraphicsCommandBuffer::draw( const Mesh& mesh )
{
    commitBindGroups();

    auto vertexBuffers = mesh.getVertexBuffers();
    auto indexBuffer   = mesh.getIndexBuffer();

    for ( uint32_t i = 0; i < vertexBuffers.size(); ++i )
    {
        if ( auto& vertexBuffer = vertexBuffers[i] )
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
        }

        wgpuRenderPassEncoderSetIndexBuffer( passEncoder, indexBuffer->getWGPUBuffer(), indexFormat, 0,
                                             indexBuffer->getSize() );
        wgpuRenderPassEncoderDrawIndexed( passEncoder, static_cast<uint32_t>( indexBuffer->getIndexCount() ), 1, 0, 0,
                                          0 );
    }
    else
    {
        if ( auto& vertexBuffer = vertexBuffers[0] )
        {
            wgpuRenderPassEncoderDraw( passEncoder, static_cast<uint32_t>( vertexBuffer->getVertexCount() ), 1, 0, 0 );
        }
    }
}

WGPUCommandBuffer GraphicsCommandBuffer::finish()
{
    wgpuRenderPassEncoderEnd( passEncoder );

    currentPipelineState = nullptr;

    WGPUCommandBufferDescriptor commandBufferDescriptor {};
    commandBufferDescriptor.label = "Graphics Command Buffer";

    return wgpuCommandEncoderFinish( commandEncoder, &commandBufferDescriptor );
}