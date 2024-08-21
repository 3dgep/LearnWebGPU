#pragma once

#include <WebGPUlib/GraphicsPipelineState.hpp>

#include <cassert>

namespace WebGPUlib
{
class Device;
class Surface;

class TextureUnlitPipelineState : public GraphicsPipelineState
{
public:
    TextureUnlitPipelineState();
    ~TextureUnlitPipelineState() override;

    TextureUnlitPipelineState( const TextureUnlitPipelineState& )     = delete;
    TextureUnlitPipelineState( TextureUnlitPipelineState&& ) noexcept = delete;

    TextureUnlitPipelineState& operator=( const TextureUnlitPipelineState& )     = delete;
    TextureUnlitPipelineState& operator=( TextureUnlitPipelineState&& ) noexcept = delete;

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