#pragma once

#include <WebGPUlib/GraphicsPipelineState.hpp>

namespace WebGPUlib
{
class Device;
class Surface;

class TextureLitPipelineState : public GraphicsPipelineState
{
public:
    TextureLitPipelineState();
    ~TextureLitPipelineState() override;

    TextureLitPipelineState( const TextureLitPipelineState& )         = delete;
    TextureLitPipelineState( TextureLitPipelineState&& ) noexcept = delete;

    TextureLitPipelineState&   operator=( const TextureLitPipelineState& )       = delete;
    TextureLitPipelineState& operator=( TextureLitPipelineState&& ) noexcept = delete;

    WGPUBindGroupLayout getWGPUBindGroupLayout( uint32_t groupIndex ) override
    {
        // This pipeline only has a single bind group.
        return bindGroupLayout;
    }

protected:
    void bind( GraphicsCommandBuffer& commandBuffer ) override;

private:
    WGPUBindGroupLayout bindGroupLayout = nullptr;
};
}  // namespace WebGPUlib