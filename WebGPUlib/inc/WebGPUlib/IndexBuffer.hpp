#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
    class IndexBuffer
    {
    public:

    protected:
        IndexBuffer(WGPUBuffer&& buffer);
        virtual ~IndexBuffer();

    private:
        WGPUBuffer buffer;
    };
}