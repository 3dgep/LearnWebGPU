#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/RenderTarget.hpp>
#include <WebGPUlib/Surface.hpp>

#include "Timer.hpp"

#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <iostream>

using namespace WebGPUlib;

constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char*   WINDOW_TITLE  = "Mesh";

bool isRunning = true;

std::shared_ptr<Mesh> cubeMesh;

void init()
{
    SDL_Init( SDL_INIT_VIDEO );

    SDL_Window* window = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                           WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE );

    if ( !window )
    {
        std::cerr << "Failed to create window." << std::endl;
        return;
    }

    Device::create( window );

    cubeMesh = Device::get().createCube();
}

void render()
{
    auto surface = Device::get().getSurface();

    RenderTarget renderTarget;
    renderTarget.attachTexture( AttachmentPoint::Color0, surface->getNextTextureView() );

    auto queue = Device::get().getQueue();

    auto commandBuffer =
        queue->createGraphicsCommandBuffer( renderTarget, ClearFlags::Color, { 0.4f, 0.6f, 0.9f, 1.0f } );

    queue->submit( commandBuffer );

    surface->present();

    // Poll the device to make sure work is done.
    Device::get().poll();
}

void update( void* userdata = nullptr )
{
    static Timer timer;
    static double totalTime = 0.0;
    static uint64_t frames    = 0;

    timer.tick();
    frames++;

    totalTime += timer.elapsedSeconds();
    if (totalTime > 1.0)
    {
        std::cout << "FPS: " << frames << std::endl;
        totalTime -= 1.0;
        frames    = 0;
    }


    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        switch ( event.type )
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            if ( event.key.keysym.sym == SDLK_ESCAPE )
            {
                isRunning = false;
            }
            break;
        default:;
        }
    }

    render();


}

void destroy()
{
    Device::destroy();
}

int main()
{
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg( update, nullptr, 0, 1 );
#else

    while ( isRunning )
    {
        update();
    }

    destroy();

#endif

    return 0;
}
