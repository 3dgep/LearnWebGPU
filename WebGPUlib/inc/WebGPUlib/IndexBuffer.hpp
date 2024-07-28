#pragma once

#include "Buffer.hpp"

#include <cstddef>

namespace WebGPUlib
{
class IndexBuffer : public Buffer
{
public:

    std::size_t getIndexCount() const
    {
        return indexCount;
    }

    
    std::size_t getIndexStride() const
    {
        return indexStride;
    }

protected:
    IndexBuffer( WGPUBuffer&& buffer, std::size_t indexCount, std::size_t indexStride );
    ~IndexBuffer() override = default;

private:
    std::size_t indexCount = 0;
    std::size_t indexStride = 0;
};
}  // namespace WebGPUlib