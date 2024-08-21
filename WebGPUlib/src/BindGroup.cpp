#include <WebGPUlib/BindGroup.hpp>
#include <WebGPUlib/Buffer.hpp>
#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/Sampler.hpp>
#include <WebGPUlib/TextureView.hpp>

using namespace WebGPUlib;

BindGroup::BindGroup() = default;
BindGroup::~BindGroup()
{
    if ( bindGroup )
        wgpuBindGroupRelease( bindGroup );
}

void BindGroup::bind( uint32_t binding, const Buffer& buffer, uint64_t offset )
{
    WGPUBindGroupEntry entry {};
    entry.binding = binding;
    entry.buffer  = buffer.getWGPUBuffer();
    entry.offset  = offset;
    entry.size    = buffer.getSize();

    bindings.push_back( entry );
}

void BindGroup::bind( uint32_t binding, const Sampler& sampler )
{
    WGPUBindGroupEntry entry {};
    entry.binding = binding;
    entry.sampler = sampler.getWGPUSampler();

    bindings.push_back( entry );
}

void BindGroup::bind( uint32_t binding, const TextureView& textureView )
{
    WGPUBindGroupEntry entry {};
    entry.binding     = binding;
    entry.textureView = textureView.getWGPUTextureView();

    bindings.push_back( entry );
}

WGPUBindGroup BindGroup::getWGPUBindGroup( WGPUBindGroupLayout layout ) const
{
    if ( bindGroup )
        wgpuBindGroupRelease( bindGroup );

    WGPUBindGroupDescriptor bindGroupDescriptor {};
    bindGroupDescriptor.layout     = layout;
    bindGroupDescriptor.entryCount = bindings.size();
    bindGroupDescriptor.entries    = bindings.data();

    auto device = Device::get().getWGPUDevice();

    bindGroup = wgpuDeviceCreateBindGroup( device, &bindGroupDescriptor );

    return bindGroup;
}