#include <WebGPUlib/BindGroup.hpp>
#include <WebGPUlib/ComputeCommandBuffer.hpp>
#include <WebGPUlib/ComputePipelineState.hpp>
#include <WebGPUlib/UploadBuffer.hpp>

#include <iostream>

using namespace WebGPUlib;

void ComputeCommandBuffer::setComputePipeline( ComputePipelineState& pipeline )
{
    // Keep track of the currently bound pipeline
    currentPipelineState = &pipeline;

    pipeline.bind( *this );
}

void ComputeCommandBuffer::dispatch( uint32_t x, uint32_t y, uint32_t z )
{
    commitBindGroups();

    wgpuComputePassEncoderDispatchWorkgroups( passEncoder, x, y, z );
}

ComputeCommandBuffer::ComputeCommandBuffer( WGPUCommandEncoder&& encoder, WGPUComputePassEncoder&& passEncoder )
: CommandBuffer { std::move( encoder ) }  // NOLINT(performance-move-const-arg)
, passEncoder { passEncoder }
{}

ComputeCommandBuffer::~ComputeCommandBuffer()
{
    if ( passEncoder )
        wgpuComputePassEncoderRelease( passEncoder );
}

void ComputeCommandBuffer::setBindGroup( uint32_t groupIndex, const BindGroup& _bindGroup )
{
    if ( currentPipelineState )
    {
        auto bindGroupLayout = currentPipelineState->getWGPUBindGroupLayout( groupIndex );
        auto bindGroup       = _bindGroup.getWGPUBindGroup( bindGroupLayout );
        wgpuComputePassEncoderSetBindGroup( passEncoder, groupIndex, bindGroup, 0, nullptr );
    }
    else
    {
        std::cerr
            << "ERROR (ComputeCommandBuffer::setBindGroup): No compute pipeline set. Make sure to set the pipeline before dispatch."
            << std::endl;
    }
}

WGPUCommandBuffer ComputeCommandBuffer::finish()
{
    wgpuComputePassEncoderEnd( passEncoder );

    currentPipelineState = nullptr;
 
    WGPUCommandBufferDescriptor commandBufferDesc {};
    commandBufferDesc.label = "Compute Command Buffer";

    reset();

    return wgpuCommandEncoderFinish( commandEncoder, &commandBufferDesc );
}
