#pragma once

#include "Buffer.hpp"

#include <cstddef>

namespace WebGPUlib
{
class StorageBuffer : public Buffer
{
public:
    StorageBuffer()                                  = default;
    StorageBuffer( const StorageBuffer& )            = delete;
    StorageBuffer( StorageBuffer&& )                 = delete;
    StorageBuffer& operator=( const StorageBuffer& ) = delete;
    StorageBuffer& operator=( StorageBuffer&& )      = delete;

    std::size_t getElementCount() const
    {
        return elementCount;
    }

    std::size_t getElementSize() const
    {
        return elementSize;
    }

    std::size_t getSize() const override
    {
        return elementCount * elementSize;
    }

protected:
    StorageBuffer( WGPUBuffer&& buffer, std::size_t elementCount, std::size_t elementSize );
    ~StorageBuffer() override = default;

private:
    std::size_t elementCount = 0;
    std::size_t elementSize  = 0;
};
}  // namespace WebGPUlib