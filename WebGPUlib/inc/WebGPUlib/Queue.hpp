#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
    class Queue
    {
    public:

    protected:
        Queue(WGPUQueue queue);
        virtual ~Queue();
    private:
        WGPUQueue queue;
    };
}