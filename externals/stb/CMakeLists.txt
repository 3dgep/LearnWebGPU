cmake_minimum_required(VERSION 3.23.0) # The version of CMake used when creating this project.

project(stb LANGUAGES C)

option( STB_IMAGE "Build the stb_image library." ON )
option( STB_IMAGE_WRITE "Build the stb_image_write library." ON )
option( STB_IMAGE_RESIZE "Build the stb_image_resize library." ON )
option( STB_DXT "Build the stb_dxt library." ON )

if (STB_IMAGE)
	add_library( stb_image STATIC stb_image.h stb_image.c )
	target_include_directories( stb_image PUBLIC . )
endif(STB_IMAGE)

if(STB_IMAGE_WRITE)
	add_library( stb_image_write STATIC stb_image_write.h stb_image_write.c )
	target_include_directories( stb_image_write PUBLIC . )
endif(STB_IMAGE_WRITE)

if (STB_IMAGE_RESIZE)
	add_library( stb_image_resize STATIC stb_image_resize2.h stb_image_resize2.c )
	target_include_directories( stb_image_resize PUBLIC . )
endif(STB_IMAGE_RESIZE)

if(STB_DXT)
	add_library( stb_dxt STATIC stb_dxt.h stb_dxt.c )
	target_include_directories( stb_dxt PUBLIC . )
endif(STB_DXT)