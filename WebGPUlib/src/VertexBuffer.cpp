#include <WebGPUlib/VertexBuffer.hpp>

#include <utility>

using namespace WebGPUlib;

#ifdef WEBGPU_BACKEND_DAWN
void wgpuBufferReference( WGPUBuffer buffer )
{
    wgpuBufferAddRef( buffer );
}
#endif

WGPUBuffer VertexBuffer::getBuffer() const
{
    return buffer;
}

VertexBuffer::VertexBuffer( WGPUBuffer&& _buffer )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: buffer { _buffer }
{}

VertexBuffer::~VertexBuffer()
{
    if ( buffer )
        wgpuBufferRelease( buffer );
}
