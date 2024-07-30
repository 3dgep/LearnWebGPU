#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class GraphicsCommandBuffer
{
public:
protected:
    GraphicsCommandBuffer( WGPUCommandEncoder&& encoder, WGPURenderPassEncoder&& passEncoder );
    virtual ~GraphicsCommandBuffer();

private:
    WGPUCommandEncoder    commandEncoder = nullptr;
    WGPURenderPassEncoder passEncoder    = nullptr;
};
}  // namespace WebGPUlib