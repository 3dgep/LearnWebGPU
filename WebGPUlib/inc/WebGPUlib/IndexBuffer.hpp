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

protected:
    IndexBuffer( WGPUBuffer&& buffer, std::size_t indexCount );
    ~IndexBuffer() override = default;

private:
    std::size_t indexCount;
};
}  // namespace WebGPUlib