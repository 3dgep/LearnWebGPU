#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

#include <cassert>
#include <iostream>

WGPUAdapter requestAdapter(WGPUInstance instance, const WGPURequestAdapterOptions *options) {
    // A simple stucture holding the local information shared with the onAdapterRequestEnded callback.
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char *message,
                                    void *pUserData) {
        UserData &userData = *static_cast<UserData *>(pUserData);

        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }

        userData.requestEnded = true;
    };

    // Call to the WebGPU request adapter procedure.
    wgpuInstanceRequestAdapter(
        instance,
        options,
        onAdapterRequestEnded,
        &userData
    );

    assert(userData.requestEnded);

    return userData.adapter;
}

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

    if (!instance) {
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
