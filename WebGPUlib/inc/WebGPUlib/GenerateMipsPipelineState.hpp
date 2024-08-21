#pragma once

#include "ComputePipelineState.hpp"

#include <glm/vec2.hpp>

namespace WebGPUlib
{
struct Mip
{
    uint32_t  srcMipLevel = 0;
    uint32_t  numMips     = 0;
    uint32_t  dimensions  = 0;
    uint32_t  isSRGB      = 0;
    glm::vec2 texelSize { 0 };
};

class GenerateMipsPipelineState : public ComputePipelineState
{
public:
    GenerateMipsPipelineState();
    ~GenerateMipsPipelineState() override;
    
    GenerateMipsPipelineState( const GenerateMipsPipelineState& )                = delete;
    GenerateMipsPipelineState( GenerateMipsPipelineState&& ) noexcept            = delete;
    GenerateMipsPipelineState& operator=( const GenerateMipsPipelineState& )     = delete;
    GenerateMipsPipelineState& operator=( GenerateMipsPipelineState&& ) noexcept = delete;

    WGPUBindGroupLayout getWGPUBindGroupLayout( uint32_t groupIndex ) override
    {
        // This pipeline only has a single bind group.
        return bindGroupLayout;
    }

protected:
    void bind( ComputeCommandBuffer& commandBuffer ) override;


private:
    WGPUBindGroupLayout bindGroupLayout    = nullptr;
};
}  // namespace WebGPUlib