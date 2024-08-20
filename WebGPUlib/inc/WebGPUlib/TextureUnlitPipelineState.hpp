#pragma once

#include "GraphicsPipelineState.hpp"

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

    WGPUBindGroupLayout getBindGroupLayout() const noexcept
    {
        return bindGroupLayout;
    }

private:
    WGPUBindGroupLayout bindGroupLayout = nullptr;
};
}  // namespace WebGPUlib