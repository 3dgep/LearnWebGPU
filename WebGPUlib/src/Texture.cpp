#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/Hash.hpp>
#include <WebGPUlib/Texture.hpp>
#include <WebGPUlib/TextureView.hpp>

using namespace WebGPUlib;

struct MakeTextureView : TextureView
{
    MakeTextureView( const WGPUTexture& texture, const WGPUTextureViewDescriptor* textureViewDescriptor = nullptr )
    : TextureView { texture, textureViewDescriptor }
    {}
};

Texture::Texture( WGPUTexture&&                _texture,  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
                  const WGPUTextureDescriptor& descriptor )
: texture { _texture }
, descriptor { descriptor }
{
    defaultView = std::make_shared<MakeTextureView>( texture );
}

Texture::~Texture()
{
    if ( texture )
        wgpuTextureRelease( texture );
}

Texture::Texture( Texture&& other ) noexcept
{
    texture       = other.texture;
    other.texture = nullptr;

    descriptor       = other.descriptor;
    other.descriptor = {};

    defaultView = std::move( other.defaultView );
    views       = std::move( other.views );
}

Texture& Texture::operator=( Texture&& other ) noexcept
{
    if ( this == &other )
        return *this;

    texture       = other.texture;
    other.texture = nullptr;

    descriptor       = other.descriptor;
    other.descriptor = {};

    defaultView = std::move( other.defaultView );
    views       = std::move( other.views );

    return *this;
}

std::shared_ptr<TextureView> Texture::getView( const WGPUTextureViewDescriptor* textureViewDescriptor )
{
    if ( textureViewDescriptor )
    {
        auto it = views.find( *textureViewDescriptor );
        if ( it != views.end() )
            return it->second;

        std::shared_ptr<TextureView> textureView = std::make_shared<MakeTextureView>( texture, textureViewDescriptor );
        views[*textureViewDescriptor]            = textureView;
        return textureView;
    }

    return defaultView;
}

void Texture::resize( uint32_t width, uint32_t height )
{
    if ( texture )
        wgpuTextureRelease( texture );

    width  = std::max( width, 1u );
    height = std::max( height, 1u );

    descriptor.size = { width, height, 1 };

    texture = wgpuDeviceCreateTexture( Device::get().getWGPUDevice(), &descriptor );

    defaultView = std::make_shared<MakeTextureView>( texture );

    views.clear();
}
