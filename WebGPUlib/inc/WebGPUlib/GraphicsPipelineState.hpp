#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class GraphicsCommandBuffer;

class GraphicsPipelineState
{
public:
    GraphicsPipelineState( const GraphicsPipelineState& )                = delete;
    GraphicsPipelineState( GraphicsPipelineState&& ) noexcept            = delete;
    GraphicsPipelineState& operator=( const GraphicsPipelineState& )     = delete;
    GraphicsPipelineState& operator=( GraphicsPipelineState&& ) noexcept = delete;

    WGPURenderPipeline getWGPURenderPipeline() const
    {
        return pipeline;
    }

    virtual WGPUBindGroupLayout getWGPUBindGroupLayout( uint32_t groupIndex ) = 0;

protected:

    friend class GraphicsCommandBuffer;

    GraphicsPipelineState() = default;
    virtual ~GraphicsPipelineState();

    virtual void bind( GraphicsCommandBuffer& commandBuffer ) = 0;

    WGPURenderPipeline pipeline = nullptr;
};
}  // namespace WebGPUlib