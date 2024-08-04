#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class Sampler
{
public:
    WGPUSampler getWGPUSampler() const noexcept
    {
        return sampler;
    }

protected:
    Sampler( WGPUSampler&& sampler, const WGPUSamplerDescriptor& descriptor );
    virtual ~Sampler();

private:
    WGPUSampler           sampler = nullptr;
    WGPUSamplerDescriptor samplerDescriptor {};
};
}  // namespace WebGPUlib