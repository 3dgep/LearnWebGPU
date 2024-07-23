#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class Buffer
{
public:
    WGPUBuffer getBuffer() const
    {
        return buffer;
    }

protected:
    Buffer(WGPUBuffer&& buffer);
    virtual ~Buffer();

private:
    WGPUBuffer buffer;
};
}  // namespace WebGPUlib