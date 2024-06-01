#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#include <iostream>
#include <GLFW/glfw3.h>

static int counter = 0;

int main (int, char**) {

    if(!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    auto window = glfwCreateWindow(800, 600, "Learn WebGPU", nullptr, nullptr);
    
    if(!window) {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

#ifdef __EMSCRIPTEN__

    emscripten_set_main_loop_arg(
        [](void*) {
            glfwPollEvents();
            std::cout << "[" << counter++ << "] Hello, World!" << std::endl;
        }, nullptr, 0, true
    );

#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
#endif

    return 0;
}

