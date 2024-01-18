cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(step010 main.cpp)

target_link_libraries(step010
    PRIVATE glfw webgpu
)

target_copy_webgpu_binaries(step010)