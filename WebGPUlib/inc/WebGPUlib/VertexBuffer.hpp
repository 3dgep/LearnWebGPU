#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class VertexBuffer
{
public:
    WGPUBuffer getBuffer() const;

protected:
    VertexBuffer( WGPUBuffer&& buffer );
    virtual ~VertexBuffer();

private:
    WGPUBuffer buffer;
};
}  // namespace WebGPUlib