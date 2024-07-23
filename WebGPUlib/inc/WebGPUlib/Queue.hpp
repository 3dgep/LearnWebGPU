#pragma once

#include <webgpu/webgpu.h>

#include <memory>

namespace WebGPUlib
{
class Device;
class Buffer;

class Queue
{
public:
    void writeBuffer( const std::shared_ptr<Buffer>& buffer, const void* data, std::size_t size ) const;

protected:
    Queue( WGPUQueue&& queue );
    virtual ~Queue();

private:
    WGPUQueue queue;
};
}  // namespace WebGPUlib