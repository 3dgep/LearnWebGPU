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

void BindGroup::bind( uint32_t binding, WGPUBuffer buffer, uint64_t offset, uint64_t size )
{
    if ( bindings.size() <= binding )
        bindings.resize( binding + 1, {} );

    WGPUBindGroupEntry entry {};
    entry.binding = binding;
    entry.buffer  = buffer;
    entry.offset  = offset;
    entry.size    = size;

    bindings[binding] = entry;
}

void BindGroup::bind( uint32_t binding, const Buffer& buffer, uint64_t offset, std::optional<uint64_t> size )
{
    bind( binding, buffer.getWGPUBuffer(), offset, size ? *size : buffer.getSize() );
}

void BindGroup::bind( uint32_t binding, const Sampler& sampler )
{
    if ( bindings.size() <= binding )
        bindings.resize( binding + 1, {} );

    WGPUBindGroupEntry entry {};
    entry.binding = binding;
    entry.sampler = sampler.getWGPUSampler();

    bindings[binding] = entry;
}

void BindGroup::bind( uint32_t binding, const TextureView& textureView )
{
    if ( bindings.size() <= binding )
        bindings.resize( binding + 1, {} );

    WGPUBindGroupEntry entry {};
    entry.binding     = binding;
    entry.textureView = textureView.getWGPUTextureView();

    bindings[binding] = entry;
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