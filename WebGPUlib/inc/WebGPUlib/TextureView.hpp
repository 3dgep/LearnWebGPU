#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class TextureView
{
public:
    TextureView() = default;
    TextureView( const TextureView& );
    TextureView( TextureView&& ) noexcept;
    TextureView& operator=( const TextureView& );
    TextureView& operator=( TextureView&& ) noexcept;

    WGPUTextureView getWGPUTextureView() const
    {
        return textureView;
    }

    explicit operator bool() const
    {
        return textureView != nullptr;
    }

protected:
    TextureView( const WGPUTexture& texture, const WGPUTextureViewDescriptor* textureViewDescriptor = nullptr );
    virtual ~TextureView();

private:
    WGPUTexture               texture     = nullptr;
    WGPUTextureView           textureView = nullptr;
    WGPUTextureViewDescriptor textureViewDescriptor {};
};
}  // namespace WebGPUlib