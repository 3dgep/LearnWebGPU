#pragma once

#include <webgpu/webgpu.h>

#include <vector>
#include <memory>
#include <optional>

namespace WebGPUlib
{
class Queue;
class BindGroup;
class Buffer;
class Sampler;
class TextureView;
class UploadBuffer;

class CommandBuffer
{
public:
    CommandBuffer() = delete;
    CommandBuffer( const CommandBuffer& ) = delete;
    CommandBuffer( CommandBuffer&& ) noexcept = delete;
    CommandBuffer& operator=( const CommandBuffer& ) = delete;
    CommandBuffer& operator=( CommandBuffer&& ) noexcept = delete;

    void bindBuffer( uint32_t groupIndex, uint32_t binding, const Buffer& buffer, uint64_t offset = 0, std::optional<uint64_t> size = {} );
    void bindSampler( uint32_t groupIndex, uint32_t binding, const Sampler& sampler );
    void bindTexture( uint32_t groupIndex, uint32_t binding, const TextureView& texture );

    void bindDynamicUniformBuffer( uint32_t groupIndex, uint32_t binding, const void* data, std::size_t sizeInBytes );
    template<typename T>
    void bindDynamicUniformBuffer( uint32_t groupIndex, uint32_t binding, const T& data );

    template<typename T>
    void bindDynamicStorageBuffer( uint32_t groupIndex, uint32_t binding, const std::vector<T>& data );
    void bindDynamicStorageBuffer( uint32_t groupIndex, uint32_t binding, const void* data, std::size_t elementCount, std::size_t elementSize );

    WGPUCommandEncoder getWGPUCommandEncoder() const
    {
        return commandEncoder;
    }

protected:
    CommandBuffer( WGPUCommandEncoder&& commandEncoder );
    virtual ~CommandBuffer();

    friend class Queue;
    virtual WGPUCommandBuffer finish() = 0;

    // Reset dynamic upload buffers.
    void reset();

    virtual void setBindGroup( uint32_t groupIndex, const BindGroup& bindGroup ) = 0;
    std::shared_ptr<BindGroup> getBindGroup( uint32_t groupIndex );

    void commitBindGroups();

    WGPUCommandEncoder commandEncoder = nullptr;
    std::vector<std::shared_ptr<BindGroup>> bindGroups;
    std::unique_ptr<UploadBuffer>           uniformUploadBuffer;
    std::unique_ptr<UploadBuffer>           storageUploadBuffer;
};

template<typename T>
void CommandBuffer::bindDynamicUniformBuffer( uint32_t groupIndex, uint32_t binding, const T& data )
{
    bindDynamicUniformBuffer( groupIndex, binding, &data, sizeof( T ) );
}

template<typename T>
void CommandBuffer::bindDynamicStorageBuffer( uint32_t groupIndex, uint32_t binding, const std::vector<T>& data )
{
    bindDynamicStorageBuffer( groupIndex, binding, data.data(), data.size(), sizeof( T ) );
}

}  // namespace WebGPUlib