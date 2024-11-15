cmake_minimum_required(VERSION 3.30)
project(demo)

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

# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# find_package(ZLIB REQUIRED)

# search for pkg-config
include(FindPkgConfig)

if(NOT PKG_CONFIG_FOUND)
message(FATAL_ERROR "pkg-config not found")
endif()

# pkg_check_modules(LIBPNG libpng16 REQUIRED)
# if(NOT LIBPNG_FOUND)
# message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
# else()
# include_directories(${LIBPNG_INCLUDE_DIRS})
# link_directories(${LIBPNG_LIBRARY_DIRS})
# link_libraries(${LIBPNG_LIBRARIES})
# endif()

if (ENABLE_LCMS2)
    find_package(LCMS2 REQUIRED)
    pkg_check_modules(LCMS2 REQUIRED lcms2)
    if (NOT LCMS2_FOUND)
        message(FATAL_ERROR "You don't seem to have lcms2 development libraries installed")
    else()
        include_directories(${LCMS2_INCLUDE_DIRS})
        link_directories(${LCMS2_LIBRARY_DIRS})
        link_libraries(${LCMS2_LIBRARIES})
    endif()
endif()

target_link_libraries(demo
    ocular
    ${LCMS2_LIBRARIES}

    # ${LIBPNG_LIBRARIES}
)
if (ENABLE_LCMS2)
    # target_link_libraries(demo ${LCMS2_LIBRARIES})
    target_include_directories(demo ${LCMS2_INCLUDE_DIRS})
endif()


target_link_libraries(palette
    ocular
)

# Point to which directory Demo is copied during installation and packaging
install(TARGETS demo DESTINATION demo)