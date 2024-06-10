cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME 01-ClearScreen)

add_executable( ${TARGET_NAME} main.cpp )

if(EMSCRIPTEN)
	target_link_options( ${TARGET_NAME}
	PRIVATE
		-sUSE_WEBGPU
		-sASYNCIFY
	)
	set_target_properties( ${TARGET_NAME}
	PROPERTIES
		SUFFIX .html
	)
endif()
