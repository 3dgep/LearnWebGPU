#pragma once

#include <webgpu/webgpu.h>

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