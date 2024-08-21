#pragma once

#include "CommandBuffer.hpp"

namespace WebGPUlib
{
class ComputePipelineState;

class ComputeCommandBuffer : public CommandBuffer
{
public:
    ComputeCommandBuffer()                                         = delete;
    ComputeCommandBuffer( const ComputeCommandBuffer& )            = delete;
    ComputeCommandBuffer( ComputeCommandBuffer&& )                 = delete;
    ComputeCommandBuffer& operator=( const ComputeCommandBuffer& ) = delete;
    ComputeCommandBuffer& operator=( ComputeCommandBuffer&& )      = delete;

    void setComputePipeline( ComputePipelineState& pipeline );

    void dispatch( uint32_t x, uint32_t y = 1, uint32_t z = 1 );

    WGPUComputePassEncoder getWGPUPassEncoder() const
    {
        return passEncoder;
    }

protected:
    ComputeCommandBuffer( WGPUCommandEncoder&& encoder, WGPUComputePassEncoder&& passEncoder );
    ~ComputeCommandBuffer() override;

    void setBindGroup( uint32_t groupIndex, const BindGroup& bindGroup ) override;

    WGPUCommandBuffer finish() override;

private:
    WGPUComputePassEncoder passEncoder          = nullptr;
    ComputePipelineState*  currentPipelineState = nullptr;
};
}  // namespace WebGPUlib