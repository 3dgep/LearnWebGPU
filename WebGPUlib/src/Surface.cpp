#include <WebGPUlib/Surface.hpp>

#include <utility>

using namespace WebGPUlib;

void Surface::resize( uint32_t width, uint32_t height )
{
    config.width  = width;
    config.height = height;

    wgpuSurfaceConfigure( surface, &config );
}

Surface::Surface( WGPUSurface&& _surface,  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
                  const WGPUSurfaceConfiguration& _config )
: surface( _surface )
, config( _config )
{}

Surface::~Surface()
{
    if ( surface )
        wgpuSurfaceRelease( surface );
}