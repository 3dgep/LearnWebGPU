cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME 01-ClearScreen)

add_executable( ${TARGET_NAME} main.cpp )
target_link_libraries( ${TARGET_NAME}
	PRIVATE SDL2::SDL2 webgpu
)

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
endif(EMSCRIPTEN)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
target_copy_webgpu_binaries( ${TARGET_NAME} )