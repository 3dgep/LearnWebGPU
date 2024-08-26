#pragma once

#include <webgpu/webgpu.h>
#include <cstddef>


namespace WebGPUlib
{
class Buffer
{
public:
    Buffer()                    = default;
    Buffer( const Buffer& )     = delete;
    Buffer( Buffer&& ) noexcept = delete;

    Buffer& operator=( const Buffer& )     = delete;
    Buffer& operator=( Buffer&& ) noexcept = delete;

    virtual std::size_t getSize() const = 0;

    WGPUBuffer getWGPUBuffer() const
    {
        return buffer;
    }

protected:
    Buffer( WGPUBuffer&& buffer );
    virtual ~Buffer();

private:
    WGPUBuffer buffer = nullptr;
};
}  // namespace WebGPUlib