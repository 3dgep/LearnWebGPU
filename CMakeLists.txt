cmake_minimum_required(VERSION 3.27)

project(LearnWebGPU VERSION 0.1.0 LANGUAGES CXX C)

option( INCLUDE_SAMPLES "Include sample projects." ON )

add_subdirectory(externals)
add_subdirectory(WebGPUlib)

if( INCLUDE_SAMPLES )
	add_subdirectory( samples )
endif( INCLUDE_SAMPLES )

