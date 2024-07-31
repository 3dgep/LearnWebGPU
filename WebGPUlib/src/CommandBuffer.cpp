#include <WebGPUlib/CommandBuffer.hpp>

#ifdef WEBGPU_BACKEND_DAWN
void wgpuCommandEncoderReference( WGPUCommandEncoder encoder )
{
    wgpuCommandEncoderAddRef( encoder );
}
#endif

using namespace WebGPUlib;

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

CommandBuffer::CommandBuffer(
    WGPUCommandEncoder&& _commandEncoder )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: commandEncoder { _commandEncoder }
{}

CommandBuffer::~CommandBuffer()
{
    if ( commandEncoder )
        wgpuCommandEncoderRelease( commandEncoder );
}