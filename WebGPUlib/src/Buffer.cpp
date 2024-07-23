#include <WebGPUlib/Buffer.hpp>

using namespace WebGPUlib;

Buffer::Buffer( WGPUBuffer&& _buffer )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: buffer( _buffer )
{}

Buffer::~Buffer()
{
    if ( buffer )
        wgpuBufferRelease( buffer );
}
