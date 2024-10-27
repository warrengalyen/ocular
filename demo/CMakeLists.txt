IF (COMMAND cmake_policy)
    cmake_policy(SET CMP0115 NEW)
ENDIF (COMMAND cmake_policy)

# Pass information about version to code through header
#    that will be supplemented by cmake.
#configure_file(
#        "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
#        "${CMAKE_CURRENT_BINARY_DIR}/version.h"
#)

# version.h file is in output directory.
# Add the binary tree directory to find the version.h file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/bin")

set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(demo demo.c)
add_executable(palette palette.c)

# Demo application require both libraries, so add them
target_link_libraries(demo
        ocular
)
target_link_libraries(palette
    ocular
)

# Point to which directory Demo is copied during installation and packaging
install(TARGETS demo DESTINATION demo)