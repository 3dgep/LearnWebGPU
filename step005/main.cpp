#include <iostream>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

int main(int, char **) {
    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    const auto window = glfwCreateWindow(800, 600, "Learn WebGPU", nullptr, nullptr);

    if (!window) {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    constexpr WGPUInstanceDescriptor desc{};
    WGPUInstance instance = wgpuCreateInstance(&desc);

    if(!instance) {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return 1;
    }

    std::cout << "WPGPU instance: " << instance << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    wgpuInstanceRelease(instance);

    glfwDestroyWindow(window);

    return 0;
}
