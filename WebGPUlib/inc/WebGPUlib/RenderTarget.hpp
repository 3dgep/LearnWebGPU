#pragma once

#include <array>
#include <memory>

namespace WebGPUlib
{

class TextureView;

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

using TextureViewArray = std::array<std::pair<std::shared_ptr<TextureView>, std::shared_ptr<TextureView>>, static_cast<std::size_t>(AttachmentPoint::NumAttachmentPoints)>;

class RenderTarget
{
public:
    RenderTarget()                                 = default;
    RenderTarget( const RenderTarget& )            = delete;
    RenderTarget( RenderTarget&& )                 = delete;
    RenderTarget& operator=( const RenderTarget& ) = delete;
    RenderTarget& operator=( RenderTarget&& )      = delete;
    ~RenderTarget()                                = default;

    void attachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<TextureView> texture, std::shared_ptr<TextureView> resolveTarget = nullptr );

    std::shared_ptr<TextureView> getTextureView( AttachmentPoint attachmentPoint ) const;
    std::shared_ptr<TextureView> getResolveTarget( AttachmentPoint attachmentPoint ) const;

    const TextureViewArray& getTextureViews() const;
private:
    TextureViewArray textureViews;
};
}  // namespace WebGPUlib
