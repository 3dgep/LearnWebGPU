#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/TextureView.hpp>

#include <cassert>

using namespace WebGPUlib;

void RenderTarget::attachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<TextureView> texture,
                                  std::shared_ptr<TextureView> resolveTarget )
{
    auto attachment                                           = std::pair( std::move( texture ), std::move(resolveTarget) );
    textureViews[static_cast<std::size_t>( attachmentPoint )] = attachment;
}

std::shared_ptr<TextureView> RenderTarget::getTextureView( AttachmentPoint attachmentPoint ) const
{
    assert( attachmentPoint < AttachmentPoint::NumAttachmentPoints );

    return textureViews[static_cast<std::size_t>( attachmentPoint )].first;
}

std::shared_ptr<TextureView> RenderTarget::getResolveTarget( AttachmentPoint attachmentPoint ) const
{
    assert( attachmentPoint < AttachmentPoint::NumAttachmentPoints );

    return textureViews[static_cast<std::size_t>( attachmentPoint )].second;
}

const TextureViewArray& RenderTarget::getTextureViews() const
{
    return textureViews;
}
