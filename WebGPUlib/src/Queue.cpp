#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/Buffer.hpp>

using namespace WebGPUlib;

void Queue::writeBuffer( const std::shared_ptr<Buffer>& buffer, const void* data, std::size_t size ) const
{
    wgpuQueueWriteBuffer( queue, buffer->getBuffer(), 0, data, size );
}

Queue::Queue( WGPUQueue&& _queue )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: queue { _queue }
{}

Queue::~Queue()
{
    if ( queue )
        wgpuQueueRelease( queue );
}

