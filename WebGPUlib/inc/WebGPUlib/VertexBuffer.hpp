#pragma once

#include "Buffer.hpp"

#include <cstddef>

namespace WebGPUlib
{
class VertexBuffer : public Buffer
{
public:
    VertexBuffer()                                     = default;
    VertexBuffer( const VertexBuffer& )                = delete;
    VertexBuffer( VertexBuffer&& ) noexcept            = delete;
    VertexBuffer& operator=( const VertexBuffer& )     = delete;
    VertexBuffer& operator=( VertexBuffer&& ) noexcept = delete;

    std::size_t getVertexCount() const
    {
        return vertexCount;
    }

    std::size_t getVertexStride() const
    {
        return vertexStride;
    }

    std::size_t getSize() const
    {
        return vertexCount * vertexStride;
    }

protected:
    VertexBuffer( WGPUBuffer&& buffer, std::size_t vertexCount, std::size_t vertexStride );
    ~VertexBuffer() override = default;

private:
    std::size_t vertexCount  = 0;
    std::size_t vertexStride = 0;
};
}  // namespace WebGPUlib