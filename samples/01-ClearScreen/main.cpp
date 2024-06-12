#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

#include <iostream>
#include <string>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
const char* WINDOW_TITLE = "Clear Screen";

SDL_Window* window = nullptr;

WGPUInstance instance;
WGPUSurface surface;
bool isRunning = true;

// Initialize the application.
void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if(!window)
    {
        std::cerr << "Failed to create window." << std::endl;
        return;
    }

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    // For some reason, the instance descriptor must be null when using emscripten.
    instance = wgpuCreateInstance(nullptr);
#else
    WGPUInstanceDescriptor desc{};
    instance = wgpuCreateInstance(&desc);
#endif

    if(!instance)
    {
        std::cerr << "Failed to create WebGPU instance." << std::endl;
        return;
    }
}

void update(void* userdata = nullptr)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
            break;
        default: ;
        }
    }
}

void destroy()
{
    wgpuInstanceRelease(instance);

    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int, char**) {
    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(update, nullptr, 0, 1);
#else

    while (isRunning)
    {
        update();
    }

    destroy();

#endif

    return 0;
}

