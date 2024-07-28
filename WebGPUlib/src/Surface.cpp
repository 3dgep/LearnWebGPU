#include <WebGPUlib/Surface.hpp>

#include <SDL2/SDL_video.h>

#include <iostream>

using namespace WebGPUlib;

void Surface::resize( uint32_t width, uint32_t height )
{
    config.width  = width;
    config.height = height;

    wgpuSurfaceConfigure( surface, &config );
}

WGPUTextureView Surface::getNextTextureView()
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
    WGPUTextureView targetView     = wgpuTextureCreateView( surfaceTexture.texture, &viewDescriptor );

    return targetView;
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