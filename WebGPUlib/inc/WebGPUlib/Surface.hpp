#pragma once

#include "TextureView.hpp"

struct SDL_Window;

namespace WebGPUlib
{
class Surface
{
public:
    void resize( uint32_t width, uint32_t height );

    TextureView getNextTextureView();

protected:
    Surface( WGPUSurface&& surface, const WGPUSurfaceConfiguration& config, SDL_Window* window );
    virtual ~Surface();

private:
    WGPUSurface              surface = nullptr;
    WGPUSurfaceConfiguration config{};
    SDL_Window*              window = nullptr;
};
}  // namespace WebGPUlib