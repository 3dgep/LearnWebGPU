cmake_minimum_required(VERSION 3.27)

set( SDL_SHARED OFF CACHE BOOL "Build a shared version of the library." )
set( SDL_TEST OFF CACHE BOOL "Build the SDL2_test library." )
set( SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Disable the SDLmain target." )
set( SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable the creation of an uninstall target." )

add_subdirectory(glm)
add_subdirectory(SDL)
add_subdirectory(webgpu)
add_subdirectory(sdl2webgpu)
add_subdirectory(stb)
add_subdirectory(tinyobjloader)

set_target_properties( SDL2-static sdl_headers_copy sdl2webgpu
	PROPERTIES
		FOLDER External/SDL2
)

set_target_properties( glm
	PROPERTIES
		FOLDER External/GLM
)

set_target_properties( tinyobjloader
	PROPERTIES
		FOLDER External
)

set( stb_targets
	stb_dxt
	stb_image
	stb_image_resize
	stb_image_write
)

foreach( stb_target ${stb_targets} )
	if( TARGET ${stb_target} )
		set_target_properties( ${stb_target} PROPERTIES FOLDER External/stb )			
	endif()
endforeach()
