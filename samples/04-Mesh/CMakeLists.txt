cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME 04-Mesh)

set( SRC
	main.cpp
	TextureUnlitPipelineState.hpp
	TextureUnlitPipelineState.cpp
	TextureLitPipelineState.hpp
	TextureLitPipelineState.cpp
	TextureUnlitShader.wgsl
	TextureLitShader.wgsl
)

add_executable( ${TARGET_NAME} ${SRC} )
target_link_libraries( ${TARGET_NAME}
	PRIVATE 00-Common SDL2::SDL2 webgpu sdl2webgpu glm::glm tinyobjloader stb_image WebGPUlib
)

if(EMSCRIPTEN)
	target_link_options( ${TARGET_NAME}
	PRIVATE
		-sUSE_WEBGPU
		-sASYNCIFY
		-sALLOW_MEMORY_GROWTH
		--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/../assets@/assets
	)
	set_target_properties( ${TARGET_NAME}
	PROPERTIES
		SUFFIX .html
	)
endif(EMSCRIPTEN)

if(MSVC)
	set_target_properties( ${TARGET_NAME}
	PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
	)
endif(MSVC)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
target_copy_webgpu_binaries( ${TARGET_NAME} )