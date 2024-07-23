#include <WebGPUlib/Queue.hpp>

using namespace WebGPUlib;

#ifdef WEBGPU_BACKEND_DAWN
void wgpuQueueReference( WGPUQueue queue )
{
    wgpuQueueAddRef( queue );
}
#endif

Queue::Queue( WGPUQueue _queue )
: queue { _queue }
{
    wgpuQueueReference( _queue );
}

Queue::~Queue()
{
    if ( queue )
        wgpuQueueRelease( queue );
}