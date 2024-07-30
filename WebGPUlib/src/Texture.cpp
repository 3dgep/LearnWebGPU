#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/Hash.hpp>
#include <WebGPUlib/Texture.hpp>

using namespace WebGPUlib;

Texture::Texture( WGPUTexture&&                _texture,  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
                  const WGPUTextureDescriptor& descriptor )
: texture { _texture }
, descriptor { descriptor }
, defaultView { texture }
{}

Texture::~Texture()
{
    if ( texture )
        wgpuTextureRelease( texture );
}

TextureView Texture::getView( const WGPUTextureViewDescriptor* textureViewDescriptor )
{
    if ( textureViewDescriptor )
    {
        auto it = views.find( *textureViewDescriptor );
        if ( it != views.end() )
            return it->second;

        TextureView textureView { texture, textureViewDescriptor };
        views[*textureViewDescriptor] = textureView;
        return textureView;
    }

    return defaultView;
}

void Texture::resize( uint32_t width, uint32_t height )
{
    if ( texture )
        wgpuTextureRelease( texture );

    descriptor.size = { width, height, 1 };

    texture = wgpuDeviceCreateTexture( Device::get().getWGPUDevice(), &descriptor );

    defaultView = TextureView { texture };

    views.clear();
}
