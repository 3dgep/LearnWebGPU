#pragma once

#include "Buffer.hpp"

#include <cstddef>

namespace WebGPUlib
{
class IndexBuffer : public Buffer
{
public:
    IndexBuffer()                         = default;
    IndexBuffer( const IndexBuffer& )     = delete;
    IndexBuffer( IndexBuffer&& ) noexcept = delete;

    IndexBuffer& operator=( const IndexBuffer& )     = delete;
    IndexBuffer& operator=( IndexBuffer&& ) noexcept = delete;

    std::size_t getIndexCount() const
    {
        return indexCount;
    }

    std::size_t getIndexStride() const
    {
        return indexStride;
    }

    WGPUIndexFormat getIndexFormat() const
    {
        return indexStride == 2 ? WGPUIndexFormat_Uint16 : WGPUIndexFormat_Uint32;
    }

    std::size_t getSize() const
    {
        return indexCount * indexStride;
    }

protected:
    IndexBuffer( WGPUBuffer&& buffer, std::size_t indexCount, std::size_t indexStride );
    ~IndexBuffer() override = default;

private:
    std::size_t indexCount  = 0;
    std::size_t indexStride = 0;
};
}  // namespace WebGPUlib