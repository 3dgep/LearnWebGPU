#pragma once

#include <array>
#include <memory>

namespace WebGPUlib
{

class Texture;

enum class AttachmentPoint
{
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    DepthStencil,
    NumAttachmentPoints,
};

using TextureArray = std::array<std::shared_ptr<Texture>, static_cast<std::size_t>(AttachmentPoint::NumAttachmentPoints)>;

class RenderTarget
{
public:
    RenderTarget()                                 = default;
    RenderTarget( const RenderTarget& )            = delete;
    RenderTarget( RenderTarget&& )                 = delete;
    RenderTarget& operator=( const RenderTarget& ) = delete;
    RenderTarget& operator=( RenderTarget&& )      = delete;
    ~RenderTarget()                                = default;

    void attachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture );

    std::shared_ptr<Texture> getTexture( AttachmentPoint attachmentPoint ) const;

    const TextureArray& getTextures() const;

    void resize( uint32_t width, uint32_t height );

private:
    TextureArray textures;
};
}  // namespace WebGPUlib
