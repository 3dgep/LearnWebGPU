#include <WebGPUlib/BindGroup.hpp>
#include <WebGPUlib/CommandBuffer.hpp>

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

CommandBuffer::CommandBuffer() = default;

CommandBuffer::CommandBuffer( const CommandBuffer& other )
: commandEncoder { other.commandEncoder }
{
    if ( commandEncoder )
        wgpuCommandEncoderReference( commandEncoder );
}

CommandBuffer::CommandBuffer( CommandBuffer&& other ) noexcept
: commandEncoder { other.commandEncoder }
{
    other.commandEncoder = nullptr;
}

CommandBuffer& CommandBuffer::operator=( const CommandBuffer& other )
{
    if ( this == &other )
        return *this;

    if ( commandEncoder )
        wgpuCommandEncoderRelease( commandEncoder );

    commandEncoder = other.commandEncoder;
    wgpuCommandEncoderReference( commandEncoder );

    return *this;
}

CommandBuffer& CommandBuffer::operator=( CommandBuffer&& other ) noexcept
{
    if ( this == &other )
        return *this;

    if ( commandEncoder )
        wgpuCommandEncoderRelease( commandEncoder );

    commandEncoder       = other.commandEncoder;
    other.commandEncoder = nullptr;

    return *this;
}

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
    for (uint32_t i = 0; i < bindGroups.size(); ++i)
    {
        if ( auto& bindGroup = bindGroups[i] )
            setBindGroup( i, *bindGroup );
    }
}

void CommandBuffer::bindBuffer( uint32_t groupIndex, uint32_t binding, const Buffer& buffer, uint64_t offset )
{
    auto bindGroup = getBindGroup( groupIndex );
    bindGroup->bind( binding, buffer, offset );
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

CommandBuffer::CommandBuffer(
    WGPUCommandEncoder&& _commandEncoder )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: commandEncoder { _commandEncoder }
{}

CommandBuffer::~CommandBuffer()
{
    if ( commandEncoder )
        wgpuCommandEncoderRelease( commandEncoder );
}
