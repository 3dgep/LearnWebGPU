cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

add_executable(step001 main.cpp)

target_link_libraries(step001
        PRIVATE glfw
)
