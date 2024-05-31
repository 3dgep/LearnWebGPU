cmake_minimum_required(VERSION 3.27)

if(NOT EMSCRIPTEN)
	add_subdirectory(glfw)
endif()

add_subdirectory(webgpu)
