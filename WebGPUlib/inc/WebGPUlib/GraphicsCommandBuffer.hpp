#pragma once

#include "CommandBuffer.hpp"

namespace WebGPUlib
{
class GraphicsPipelineState;
class Mesh;

class GraphicsCommandBuffer : public CommandBuffer
{
public:
    GraphicsCommandBuffer()                                          = delete;
    GraphicsCommandBuffer( const GraphicsCommandBuffer& )            = delete;
    GraphicsCommandBuffer( GraphicsCommandBuffer&& )                 = delete;
    GraphicsCommandBuffer& operator=( const GraphicsCommandBuffer& ) = delete;
    GraphicsCommandBuffer& operator=( GraphicsCommandBuffer&& )      = delete;

    void setGraphicsPipeline( GraphicsPipelineState& pipeline );

    void draw( const Mesh& mesh );

    WGPURenderPassEncoder getWGPUPassEncoder() const
    {
        return passEncoder;
    }

protected:
    GraphicsCommandBuffer( WGPUCommandEncoder&& encoder, WGPURenderPassEncoder&& passEncoder );
    ~GraphicsCommandBuffer() override;

    void setBindGroup( uint32_t groupIndex, const BindGroup& bindGroup ) override;

    WGPUCommandBuffer finish() override;

private:
    WGPURenderPassEncoder  passEncoder          = nullptr;
    GraphicsPipelineState* currentPipelineState = nullptr;
};
}  // namespace WebGPUlib