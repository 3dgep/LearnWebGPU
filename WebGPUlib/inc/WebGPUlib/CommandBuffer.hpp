#pragma once

#include <webgpu/webgpu.h>

#include <vector>
#include <memory>

namespace WebGPUlib
{
class Queue;
class BindGroup;
class Buffer;
class Sampler;
class TextureView;

class CommandBuffer
{
public:
    CommandBuffer();
    CommandBuffer( const CommandBuffer& );
    CommandBuffer( CommandBuffer&& ) noexcept;
    CommandBuffer& operator=( const CommandBuffer& );
    CommandBuffer& operator=( CommandBuffer&& ) noexcept;

    void bindBuffer( uint32_t groupIndex, uint32_t binding, const Buffer& buffer, uint64_t offset = 0 );
    void bindSampler( uint32_t groupIndex, uint32_t binding, const Sampler& sampler );
    void bindTexture( uint32_t groupIndex, uint32_t binding, const TextureView& texture );

    WGPUCommandEncoder getWGPUCommandEncoder() const
    {
        return commandEncoder;
    }

protected:
    CommandBuffer( WGPUCommandEncoder&& commandEncoder );
    virtual ~CommandBuffer();

    friend class Queue;
    virtual WGPUCommandBuffer finish() = 0;

    virtual void setBindGroup( uint32_t groupIndex, const BindGroup& bindGroup ) = 0;
    std::shared_ptr<BindGroup> getBindGroup( uint32_t groupIndex );

    void commitBindGroups();

    WGPUCommandEncoder commandEncoder = nullptr;
    std::vector<std::shared_ptr<BindGroup>> bindGroups;
};
}  // namespace WebGPUlib