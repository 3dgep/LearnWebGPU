#include "WebGPUlib/Helpers.hpp"
#include "WebGPUlib/Queue.hpp"

#include <WebGPUlib/BindGroup.hpp>
#include <WebGPUlib/CommandBuffer.hpp>
#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/UploadBuffer.hpp>

#ifdef WEBGPU_BACKEND_DAWN
void wgpuCommandEncoderReference( WGPUCommandEncoder encoder )
{
    wgpuCommandEncoderAddRef( encoder );
}
#endif

using namespace WebGPUlib;

struct MakeBindGroup : BindGroup
{
    MakeBindGroup() = default;
};

struct MakeUploadBuffer : UploadBuffer
{
    MakeUploadBuffer( WGPUBufferUsage usage, std::size_t pageSize )
    : UploadBuffer( usage, pageSize )
    {}
};

std::shared_ptr<BindGroup> CommandBuffer::getBindGroup( uint32_t groupIndex )
{
    if ( bindGroups.size() <= groupIndex )
        bindGroups.resize( groupIndex + 1, nullptr );

    auto bindGroup = bindGroups[groupIndex];
    if ( !bindGroup )
    {
        bindGroup              = std::make_shared<MakeBindGroup>();
        bindGroups[groupIndex] = bindGroup;
    }

    return bindGroup;
}

void CommandBuffer::commitBindGroups()
{
    for ( uint32_t i = 0; i < bindGroups.size(); ++i )
    {
        if ( auto& bindGroup = bindGroups[i] )
            setBindGroup( i, *bindGroup );
    }
}

void CommandBuffer::bindBuffer( uint32_t groupIndex, uint32_t binding, const Buffer& buffer, uint64_t offset,
                                std::optional<uint64_t> size )
{
    auto bindGroup = getBindGroup( groupIndex );
    bindGroup->bind( binding, buffer, offset, size );
}

void CommandBuffer::bindSampler( uint32_t groupIndex, uint32_t binding, const Sampler& sampler )
{
    auto bindGroup = getBindGroup( groupIndex );
    bindGroup->bind( binding, sampler );
}

void CommandBuffer::bindTexture( uint32_t groupIndex, uint32_t binding, const TextureView& texture )
{
    auto bindGroup = getBindGroup( groupIndex );
    bindGroup->bind( binding, texture );
}

void CommandBuffer::bindDynamicUniformBuffer( uint32_t groupIndex, uint32_t binding, const void* data,
                                              std::size_t sizeInBytes )
{
    auto allocation = uniformUploadBuffer->allocate( sizeInBytes, 256 );

    auto queue = Device::get().getQueue();

    queue->writeBuffer( allocation.buffer, data, sizeInBytes, allocation.offset );

    auto bindGroup = getBindGroup( groupIndex );
    bindGroup->bind( binding, allocation.buffer, allocation.offset, sizeInBytes );
}

void CommandBuffer::bindDynamicStorageBuffer( uint32_t groupIndex, uint32_t binding, const void* data,
                                              std::size_t elementCount, std::size_t elementSize )
{
    auto sizeInBytes = elementCount * elementSize;
    auto allocation  = storageUploadBuffer->allocate( sizeInBytes, 256 );

    auto queue = Device::get().getQueue();

    queue->writeBuffer( allocation.buffer, data, sizeInBytes, allocation.offset );

    auto bindGroup = getBindGroup( groupIndex );
    bindGroup->bind( binding, allocation.buffer, allocation.offset, sizeInBytes );
}

CommandBuffer::CommandBuffer(
    WGPUCommandEncoder&& _commandEncoder )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: commandEncoder { _commandEncoder }
{
    uniformUploadBuffer = std::make_unique<MakeUploadBuffer>( WGPUBufferUsage_Uniform, _2MB );
    storageUploadBuffer = std::make_unique<MakeUploadBuffer>( WGPUBufferUsage_Storage, _2MB );
}

CommandBuffer::~CommandBuffer()
{
    if ( commandEncoder )
        wgpuCommandEncoderRelease( commandEncoder );
}

void CommandBuffer::reset()
{
    uniformUploadBuffer->reset();
}
