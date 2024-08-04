#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
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

protected:
    GraphicsPipelineState() = default;
    virtual ~GraphicsPipelineState();

    WGPURenderPipeline pipeline = nullptr;
};
}  // namespace WebGPUlib