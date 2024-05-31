cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(step000 main.cpp)

if(EMSCRIPTEN)
	target_link_options(step000
	PRIVATE
		-sUSE_GLFW=3
		-sUSE_WEBGPU
		-sASYNCIFY
	)
	set_target_properties(step000
	PROPERTIES
		SUFFIX .html
	)
endif()
