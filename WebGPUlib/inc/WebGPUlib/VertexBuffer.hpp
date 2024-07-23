#pragma once

#include "Buffer.hpp"

#include <webgpu/webgpu.h>

#include <cstddef>

namespace WebGPUlib
{
class VertexBuffer : public Buffer
{
public:
    std::size_t getVertexCount() const
    {
        return vertexCount;
    }

protected:
    VertexBuffer( WGPUBuffer&& buffer, std::size_t vertexCount );
    ~VertexBuffer() override = default;

private:
    std::size_t vertexCount;
};
}  // namespace WebGPUlib