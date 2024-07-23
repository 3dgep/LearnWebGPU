#pragma once

#include <webgpu/webgpu.h>

namespace WebGPUlib
{
class Surface
{
public:
    void resize( uint32_t width, uint32_t height );

protected:
    Surface( WGPUSurface&& surface, const WGPUSurfaceConfiguration& config );
    virtual ~Surface();

private:
    WGPUSurface              surface;
    WGPUSurfaceConfiguration config;
};
}  // namespace WebGPUlib