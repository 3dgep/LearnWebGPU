#include <iostream>
#include <GLFW/glfw3.h>

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

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    return 0;
}

