#pragma once

#include "Buffer.hpp"

#include <cstddef>

namespace WebGPUlib
{
class UniformBuffer : public Buffer
{
public:
    UniformBuffer()                                  = default;
    UniformBuffer( const UniformBuffer& )            = delete;
    UniformBuffer( UniformBuffer&& )                 = delete;
    UniformBuffer& operator=( const UniformBuffer& ) = delete;
    UniformBuffer& operator=( UniformBuffer&& )      = delete;

    std::size_t getSize() const
    {
        return size;
    }

protected:
    UniformBuffer( WGPUBuffer&& buffer, std::size_t size );
    ~UniformBuffer() override = default;

private:
    std::size_t size = 0;
};
}  // namespace WebGPUlib