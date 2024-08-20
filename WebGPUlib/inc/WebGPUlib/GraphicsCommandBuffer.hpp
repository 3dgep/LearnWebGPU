#pragma once

#include "CommandBuffer.hpp"

namespace WebGPUlib
{
class GraphicsPipelineState;
class Mesh;

class GraphicsCommandBuffer : public CommandBuffer
{
public:
    void setGraphicsPipeline( GraphicsPipelineState& pipeline );

    void draw( const Mesh& mesh );

protected:
    GraphicsCommandBuffer( WGPUCommandEncoder&& encoder, WGPURenderPassEncoder&& passEncoder );
    ~GraphicsCommandBuffer() override;


    WGPUCommandBuffer finish() override;

private:
    WGPURenderPassEncoder passEncoder    = nullptr;
};
}  // namespace WebGPUlib