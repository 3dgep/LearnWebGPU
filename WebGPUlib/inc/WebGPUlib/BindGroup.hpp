#pragma once

#include <webgpu/webgpu.h>

#include <vector>

namespace WebGPUlib
{

class Buffer;
class Sampler;
class TextureView;

class BindGroup
{
public:
    void bind( uint32_t binding, const Buffer& buffer, uint64_t offset = 0 );
    void bind( uint32_t binding, const Sampler& sampler );
    void bind( uint32_t binding, const TextureView& textureView );

    WGPUBindGroup getWGPUBindGroup( WGPUBindGroupLayout layout ) const;

protected:
    BindGroup();
    virtual ~BindGroup();

private:
    std::vector<WGPUBindGroupEntry> bindings;
    mutable WGPUBindGroup           bindGroup = nullptr;
};
}  // namespace WebGPUlib