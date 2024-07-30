#pragma once

#include "TextureView.hpp"

#include <webgpu/webgpu.h>

#include <unordered_map>

namespace WebGPUlib
{
class Texture
{
public:
    TextureView getView( const WGPUTextureViewDescriptor* textureViewDescriptor = nullptr );

    void resize( uint32_t width, uint32_t height );

protected:
    Texture( WGPUTexture&& texture, const WGPUTextureDescriptor& descriptor );
    virtual ~Texture();

private:
    WGPUTexture                                                texture = nullptr;
    WGPUTextureDescriptor                                      descriptor{};
    TextureView                                                defaultView;
    std::unordered_map<WGPUTextureViewDescriptor, TextureView> views;
};

}  // namespace WebGPUlib