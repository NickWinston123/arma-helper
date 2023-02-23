cmake_minimum_required(VERSION 3.12)
project(Armagetron)

set(CMAKE_CXX_STANDARD 11)

# Add the source files here
# ...

# Add the include directories here
# ...

# Add the libraries here
# ...

# Define the two targets, Win32 Release and Win32 Debug
add_executable(Win32_Release ${source_files})
target_compile_options(Win32_Release PRIVATE -O2 -W -DNDEBUG -DDIRTY)
target_link_libraries(Win32_Release PRIVATE -s libSDL libSDL_image libSDL_mixer)

add_executable(Win32_Debug ${source_files})
target_compile_options(Win32_Debug PRIVATE -W -g -DNDEBUG -DDEBUG)
target_link_libraries(Win32_Debug PRIVATE libSDL libSDL_image libSDL_mixer)

# Set the output directories
set_target_properties(Win32_Release PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ../dist
    WORKING_DIRECTORY ../dist
    OBJECT_OUTPUT_DIRECTORY tmp/Armagetron___Win32_Release
)
set_target_properties(Win32_Debug PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ../debug
    WORKING_DIRECTORY ../debug
    OBJECT_OUTPUT_DIRECTORY tmp/Armagetron___Win32_Debug
)
