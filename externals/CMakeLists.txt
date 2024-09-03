cmake_minimum_required(VERSION 3.27)

set( ASSIMP_INSTALL OFF CACHE BOOL "Disable this if you want to use assimp as a submodule.")
set( ASSIMP_INSTALL_PDB OFF CACHE BOOL "Create MSVC debug symbol files and add to Install target.")
set( ASSIMP_BUILD_TESTS OFF CACHE BOOL "If the test suite for Assimp is built in addition to the library.")
set( ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build Assimp Tools." )
set( ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Build Assimp Samples." )
set( ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build zlib for Assimp." )

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
add_subdirectory(assimp)

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

set( assimp_targets
	assimp
	uninstall
	UpdateAssimpLibsDebugSymbolsAndDLLs
	zlib
	zlibstatic
)	

foreach( assimp_target ${assimp_targets} )
	if( TARGET ${assimp_target} )
		set_target_properties( ${assimp_target} PROPERTIES FOLDER External/assimp )			
	endif()
endforeach()

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
