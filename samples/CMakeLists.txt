cmake_minimum_required(VERSION 3.27)

add_subdirectory( 00-Common )
add_subdirectory( 01-ClearScreen )
add_subdirectory( 02-Cube )

set( ALL_SAMPLES
	00-Common
	01-ClearScreen
	02-Cube
)

set_target_properties( ${ALL_SAMPLES}
    PROPERTIES
        FOLDER Samples
)

# Set the startup project.
set_directory_properties( PROPERTIES 
	VS_STARTUP_PROJECT 02-Cube
)
