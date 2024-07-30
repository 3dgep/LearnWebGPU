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
    virtual ~TextureView();

    WGPUTextureView getTextureView() const
    {
        return textureView;
    }

    explicit operator bool() const
    {
        return textureView != nullptr;
    }

protected:
    friend class Texture;
    friend class Surface;
    TextureView( const WGPUTexture& texture, const WGPUTextureViewDescriptor* textureViewDescriptor = nullptr );

private:
    WGPUTexture               texture     = nullptr;
    WGPUTextureView           textureView = nullptr;
    WGPUTextureViewDescriptor textureViewDescriptor {};
};
}  // namespace WebGPUlib