cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)
set(TARGET_NAME 00-Common)

set( INC
	inc/Camera.hpp
	inc/CameraController.hpp
	inc/GamePad.hpp
	inc/Keyboard.hpp
	inc/Mouse.hpp
	inc/Timer.hpp
)

set( SRC
	src/Camera.cpp
	src/CameraController.cpp
	src/GamePad.cpp
	src/Keyboard.cpp
	src/Mouse.cpp
	src/Timer.cpp
)

add_library( ${TARGET_NAME} STATIC ${INC} ${SRC} )

target_link_libraries( ${TARGET_NAME}
	PUBLIC glm::glm SDL2::SDL2
)

target_include_directories( ${TARGET_NAME} PUBLIC inc )