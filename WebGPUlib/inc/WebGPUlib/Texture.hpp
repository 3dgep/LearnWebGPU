#pragma once

#include <webgpu/webgpu.h>

#include <unordered_map>

namespace WebGPUlib
{
class TextureView;

class Texture
{
public:
    Texture()                            = delete;
    Texture( const Texture& )            = delete;
    Texture( Texture&& ) noexcept;
    Texture& operator=( const Texture& ) = delete;
    Texture& operator=( Texture&& ) noexcept;

    std::shared_ptr<TextureView> getView( const WGPUTextureViewDescriptor* textureViewDescriptor = nullptr );

    void resize( uint32_t width, uint32_t height );

protected:
    Texture( WGPUTexture&& texture, const WGPUTextureDescriptor& descriptor );
    virtual ~Texture();

private:
    WGPUTexture                                                                 texture = nullptr;
    WGPUTextureDescriptor                                                       descriptor {};
    std::shared_ptr<TextureView>                                                defaultView;
    std::unordered_map<WGPUTextureViewDescriptor, std::shared_ptr<TextureView>> views;
};

}  // namespace WebGPUlib