#pragma once

#include <webgpu/webgpu.h>

#include <memory>

struct SDL_Window;

namespace WebGPUlib
{
class TextureView;

class Surface
{
public:

    void present();
    void resize( uint32_t width, uint32_t height );

    std::shared_ptr<TextureView> getNextTextureView();

protected:
    Surface( WGPUSurface&& surface, const WGPUSurfaceConfiguration& config, SDL_Window* window );
    virtual ~Surface();

private:
    WGPUSurface              surface = nullptr;
    WGPUSurfaceConfiguration config{};
    SDL_Window*              window = nullptr;
};
}  // namespace WebGPUlib