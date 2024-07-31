#pragma once

#include "CommandBuffer.hpp"

namespace WebGPUlib
{
class GraphicsCommandBuffer : public CommandBuffer
{
public:
protected:
    GraphicsCommandBuffer( WGPUCommandEncoder&& encoder, WGPURenderPassEncoder&& passEncoder );
    ~GraphicsCommandBuffer() override;

    WGPUCommandBuffer finish() override;

private:
    WGPURenderPassEncoder passEncoder    = nullptr;
};
}  // namespace WebGPUlib