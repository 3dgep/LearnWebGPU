#include <WebGPUlib/Surface.hpp>
#include <WebGPUlib/TextureView.hpp>

#include <SDL2/SDL_video.h>

#include <iostream>

using namespace WebGPUlib;

struct MakeTextureView : TextureView
{
    MakeTextureView( const WGPUTexture& texture, const WGPUTextureViewDescriptor* textureViewDescriptor = nullptr )
    : TextureView( texture, textureViewDescriptor )
    {}
};

void Surface::present()
{
#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent( surface );
#endif
}

void Surface::resize( uint32_t width, uint32_t height )
{
    config.width  = width;
    config.height = height;

    wgpuSurfaceConfigure( surface, &config );
}

std::shared_ptr<TextureView> Surface::getNextTextureView()
{
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture( surface, &surfaceTexture );

    switch ( surfaceTexture.status )  // NOLINT(clang-diagnostic-switch-enum)
    {
    case WGPUSurfaceGetCurrentTextureStatus_Success:
        // All good. Just continue.
        break;
    case WGPUSurfaceGetCurrentTextureStatus_Timeout:
    case WGPUSurfaceGetCurrentTextureStatus_Outdated:
    case WGPUSurfaceGetCurrentTextureStatus_Lost:
    {
        if ( surfaceTexture.texture )
            wgpuTextureRelease( surfaceTexture.texture );

        int width, height;
        SDL_GetWindowSize( window, &width, &height );

        if ( width > 0 && height > 0 )
        {
            resize( width, height );
        }

        return nullptr;
    }
    default:
        // Handle the error.
        std::cerr << "Error getting surface texture: " << surfaceTexture.status << std::endl;
        return nullptr;
    }

    WGPUTextureViewDescriptor viewDescriptor {};
    viewDescriptor.label           = "Surface Texture View";
    viewDescriptor.format          = wgpuTextureGetFormat( surfaceTexture.texture );
    viewDescriptor.dimension       = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel    = 0;
    viewDescriptor.mipLevelCount   = 1;
    viewDescriptor.baseArrayLayer  = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect          = WGPUTextureAspect_All;

    std::shared_ptr<TextureView> view = std::make_shared<MakeTextureView>( surfaceTexture.texture, &viewDescriptor );

    wgpuTextureRelease( surfaceTexture.texture );

    return view;
}

Surface::Surface( WGPUSurface&& _surface,  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
                  const WGPUSurfaceConfiguration& _config, SDL_Window* _window )
: surface( _surface )
, config( _config )
, window( _window )
{}

Surface::~Surface()
{
    if ( surface )
        wgpuSurfaceRelease( surface );
}