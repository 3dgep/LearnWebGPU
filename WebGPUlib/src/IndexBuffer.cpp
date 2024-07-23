#include <WebGPUlib/IndexBuffer.hpp>

using namespace WebGPUlib;

IndexBuffer::IndexBuffer(WGPUBuffer&& _buffer)  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
    : buffer(_buffer)
{}

IndexBuffer::~IndexBuffer()
{
    if ( buffer )
        wgpuBufferRelease( buffer );
}

