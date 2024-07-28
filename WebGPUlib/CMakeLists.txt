cmake_minimum_required(VERSION 3.29)

project( WebGPUlib VERSION 0.1.0 LANGUAGES CXX C)

set( TARGET_NAME WebGPUlib )

set( INC
	inc/WebGPUlib/Buffer.hpp
	inc/WebGPUlib/Device.hpp
	inc/WebGPUlib/Queue.hpp
	inc/WebGPUlib/IndexBuffer.hpp
	inc/WebGPUlib/Surface.hpp
	inc/WebGPUlib/Texture.hpp
	inc/WebGPUlib/VertexBuffer.hpp
)

set( SRC
	src/Buffer.cpp
	src/Device.cpp
	src/Queue.cpp
	src/IndexBuffer.cpp
	src/Surface.cpp
	src/Texture.cpp
	src/VertexBuffer.cpp
)

add_library( ${TARGET_NAME} STATIC ${SRC} ${INC} )

target_compile_features( WebGPUlib
PUBLIC
	cxx_std_17
)

target_include_directories( ${TARGET_NAME}
PUBLIC
	inc
)

target_link_libraries( ${TARGET_NAME}
PUBLIC
	SDL2::SDL2 webgpu sdl2webgpu
)
