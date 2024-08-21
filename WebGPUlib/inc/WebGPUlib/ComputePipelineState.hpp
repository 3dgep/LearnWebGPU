#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{

class ComputeCommandBuffer;

class ComputePipelineState
{
public:
    ComputePipelineState( const ComputePipelineState& )                = delete;
    ComputePipelineState( ComputePipelineState&& )                     = delete;
    ComputePipelineState& operator=( const ComputePipelineState& )     = delete;
    ComputePipelineState& operator=( ComputePipelineState&& ) noexcept = delete;

    WGPUComputePipeline getWGPUComputePipeline() const
    {
        return pipeline;
    }

    virtual WGPUBindGroupLayout getWGPUBindGroupLayout( uint32_t groupIndex ) = 0;

protected:
    friend class ComputeCommandBuffer;

    ComputePipelineState() = default;
    virtual ~ComputePipelineState();

    virtual void bind( ComputeCommandBuffer& commandBuffer ) = 0;

    WGPUComputePipeline pipeline = nullptr;
};
}  // namespace WebGPUlib