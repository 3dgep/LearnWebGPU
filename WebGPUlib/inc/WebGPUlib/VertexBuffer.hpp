#pragma once

#include "Buffer.hpp"

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

    std::size_t getVertexStride() const
    {
        return vertexStride;
    }

protected:
    VertexBuffer( WGPUBuffer&& buffer, std::size_t vertexCount, std::size_t vertexStride );
    ~VertexBuffer() override = default;

private:
    std::size_t vertexCount  = 0;
    std::size_t vertexStride = 0;
};
}  // namespace WebGPUlib