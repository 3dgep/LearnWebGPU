#include <WebGPUlib/TextureView.hpp>

#ifdef WEBGPU_BACKEND_DAWN
void wgpuTextureReference( WGPUTexture texture )
{
    wgpuTextureAddRef( texture );
}

void wgpuTextureViewReference( WGPUTextureView textureView )
{
    wgpuTextureViewAddRef( textureView );
}
#endif

using namespace WebGPUlib;

TextureView::TextureView( const TextureView& copy )
{
    texture = copy.texture;
    if ( texture )
        wgpuTextureReference( texture );

    textureView = copy.textureView;
    if ( textureView )
        wgpuTextureViewReference( textureView );

    textureViewDescriptor = copy.textureViewDescriptor;
}

TextureView::TextureView( TextureView&& other ) noexcept
{
    texture = other.texture;
    other.texture = nullptr;

    textureView = other.textureView;
    other.textureView = nullptr;

    textureViewDescriptor = other.textureViewDescriptor;
    other.textureViewDescriptor = {};
}

TextureView& TextureView::operator=( const TextureView& copy)
{
    if ( &copy == this )
        return *this;

    texture = copy.texture;
    if (texture)
        wgpuTextureReference( texture );

    textureView = copy.textureView;
    if (textureView)
        wgpuTextureViewReference( textureView );

    textureViewDescriptor = copy.textureViewDescriptor;

    return *this;
}

TextureView& TextureView::operator=( TextureView&& other ) noexcept
{
    if ( &other == this )
        return *this;

    texture = other.texture;
    other.texture = nullptr;

    textureView = other.textureView;
    other.textureView = nullptr;

    textureViewDescriptor = other.textureViewDescriptor;
    other.textureViewDescriptor = {};

    return *this;
}

TextureView::TextureView( const WGPUTexture& _texture, const WGPUTextureViewDescriptor* _textureViewDescriptor )
: texture( _texture )
{
    if ( texture )
        wgpuTextureReference( texture );

    textureView = wgpuTextureCreateView( texture, _textureViewDescriptor );

    if ( _textureViewDescriptor )
        textureViewDescriptor = *_textureViewDescriptor;
}

TextureView::~TextureView()
{
    if ( textureView )
        wgpuTextureViewRelease( textureView );
    if ( texture )
        wgpuTextureRelease( texture );
}