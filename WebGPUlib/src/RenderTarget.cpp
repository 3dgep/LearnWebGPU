#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/Texture.hpp>

#include <cassert>

using namespace WebGPUlib;

void RenderTarget::attachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture )
{
    textures[static_cast<std::size_t>( attachmentPoint )] = std::move( texture );
}

std::shared_ptr<Texture> RenderTarget::getTexture( AttachmentPoint attachmentPoint ) const
{
    assert( attachmentPoint < AttachmentPoint::NumAttachmentPoints );

    return textures[static_cast<std::size_t>( attachmentPoint )];
}

const TextureArray& RenderTarget::getTextures() const
{
    return textures;
}

void RenderTarget::resize( uint32_t width, uint32_t height )
{
    for ( auto& texture: textures )
    {
        if ( texture )
            texture->resize( width, height );
    }
}
