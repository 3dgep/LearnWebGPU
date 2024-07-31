#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class Queue;

class CommandBuffer
{
public:
    CommandBuffer();
    CommandBuffer( const CommandBuffer& );
    CommandBuffer( CommandBuffer&& ) noexcept;
    CommandBuffer& operator=( const CommandBuffer& );
    CommandBuffer& operator=( CommandBuffer&& ) noexcept;

    WGPUCommandEncoder getWGPUCommandEncoder() const
    {
        return commandEncoder;
    }

protected:
    CommandBuffer( WGPUCommandEncoder&& commandEncoder );
    virtual ~CommandBuffer();

    friend class Queue;
    virtual WGPUCommandBuffer finish() = 0;

    WGPUCommandEncoder commandEncoder = nullptr;
};
}  // namespace WebGPUlib