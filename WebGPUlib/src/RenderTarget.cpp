#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/TextureView.hpp>

#include <cassert>

using namespace WebGPUlib;

void RenderTarget::attachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<TextureView> texture )
{
    textureViews[static_cast<std::size_t>( attachmentPoint )] = std::move( texture );
}

std::shared_ptr<TextureView> RenderTarget::getTextureView( AttachmentPoint attachmentPoint ) const
{
    assert( attachmentPoint < AttachmentPoint::NumAttachmentPoints );

    return textureViews[static_cast<std::size_t>( attachmentPoint )];
}

const TextureViewArray& RenderTarget::getTextureViews() const
{
    return textureViews;
}
