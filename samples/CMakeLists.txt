cmake_minimum_required(VERSION 3.27)

set( SAMPLES
	00-Common
	01-ClearScreen
	02-Cube
	03-Texture
	04-Mesh
	05-Masterclass
)

foreach( SAMPLE ${SAMPLES} )
	add_subdirectory( ${SAMPLE} )
endforeach()

set_target_properties( ${SAMPLES}
    PROPERTIES
        FOLDER Samples
)

# Set the startup project.
set_directory_properties( PROPERTIES 
	VS_STARTUP_PROJECT 04-Mesh
)
