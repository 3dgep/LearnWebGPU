cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

add_executable(step001 main.cpp)

if(EMSCRIPTEN)
	target_link_options(step001
	PRIVATE
		-sUSE_GLFW=3
		-sUSE_WEBGPU
		-sASYNCIFY
	)
	set_target_properties(step001
	PROPERTIES
		SUFFIX .html
	)
else()
	target_link_libraries(step001
		PRIVATE glfw
	)
endif()
