cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(step005 main.cpp)

if(EMSCRIPTEN)
	target_link_options(step005
	PRIVATE
		-sUSE_GLFW=3
		-sUSE_WEBGPU
		-sASYNCIFY
	)
	set_target_properties(step005
	PROPERTIES
		SUFFIX .html
	)
else()
	target_link_libraries(step005
		PRIVATE glfw webgpu
	)
	target_copy_webgpu_binaries(step005)
endif()
