#include <WebGPUlib/BindGroup.hpp>

using namespace WebGPUlib;

BindGroup::BindGroup( WGPUBindGroup&& bindGroup )
: bindGroup( bindGroup )
{}

BindGroup::~BindGroup()
{
    if (bindGroup)
        wgpuBindGroupRelease(bindGroup);
}