cmake_minimum_required(VERSION 3.29)

project( WebGPUlib VERSION 0.1.0 LANGUAGES CXX C)

set( INC
	inc/WebGPUlib/Device.hpp
)

set( SRC
	src/Device.cpp
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
