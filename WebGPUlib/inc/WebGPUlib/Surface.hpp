#pragma once

#include <webgpu/webgpu.h>

struct SDL_Window;

namespace WebGPUlib
{
class Surface
{
public:
    void resize( uint32_t width, uint32_t height );

    WGPUTextureView getNextTextureView();

protected:
    Surface( WGPUSurface&& surface, const WGPUSurfaceConfiguration& config, SDL_Window* window );
    virtual ~Surface();

private:
    WGPUSurface              surface;
    WGPUSurfaceConfiguration config;
    SDL_Window*              window;
};
}  // namespace WebGPUlib