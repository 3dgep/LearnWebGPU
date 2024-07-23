cmake_minimum_required(VERSION 3.29)

project( WebGPUlib VERSION 0.1.0 LANGUAGES CXX C)

set( INC
	inc/WebGPUlib/Device.hpp
	inc/WebGPUlib/Queue.hpp
	inc/WebGPUlib/IndexBuffer.hpp
	inc/WebGPUlib/VertexBuffer.hpp
)

set( SRC
	src/Device.cpp
	src/Queue.cpp
	src/IndexBuffer.cpp
	src/VertexBuffer.cpp
)

add_library( WebGPUlib STATIC ${SRC} ${INC} ../.clang-format )

target_compile_features( WebGPUlib
PUBLIC
	cxx_std_20
)

target_include_directories( WebGPUlib
PUBLIC
	inc
)

target_link_libraries( WebGPUlib
PUBLIC
	webgpu
)
