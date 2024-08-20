#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class BindGroup
{
public:
    WGPUBindGroup getWGPUBindGroup() const
    {
        return bindGroup;
    }

protected:
    BindGroup( WGPUBindGroup&& bindGroup );
    virtual ~BindGroup();

private:
    WGPUBindGroup bindGroup;
};
}  // namespace WebGPUlib