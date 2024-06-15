cmake_minimum_required(VERSION 3.27)

add_subdirectory( 01-ClearScreen )

set_target_properties( 01-ClearScreen
    PROPERTIES
        FOLDER Samples
)

# Set the startup project.
set_directory_properties( PROPERTIES 
	VS_STARTUP_PROJECT 01-ClearScreen
)
