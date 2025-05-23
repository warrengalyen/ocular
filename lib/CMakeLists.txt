cmake_minimum_required(VERSION 3.9)
project(ocular)

# Remove trailing '\' and '/' at the end of the path, so can reliably compare
STRING(REGEX REPLACE "[/\\]+$" "" CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
STRING(REGEX REPLACE "[/\\]+$" "" CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# When compiling as stand alone library set to C++17,
# when library is part of other project, respect parent project settings.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_CXX_STANDARD 17)
endif ()

if (NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "ERROR: CMAKE_CXX_STANDARD is not set or it is too low. Minimum C v17 is required to compile \"Ocular\" library.")
endif ()

# Call the function to get the current VERSION_BUILD
# get_version_build()
set(MODULE_FILES
    auxiliary.c
    core.c
    color.c
    threshold.c
    util.c
    ocr.c
    palette.c
    dither.c
    quanitize.c
    retinex.c
    edge_filters.c
    blur_filters.c
    ocular.c
)

add_library(ocular STATIC ${MODULE_FILES})

# Pre-build step to update VERSION_BUILD
if (!MSVC)
    add_custom_command(
        TARGET ocular
        PRE_BUILD
        COMMAND bash build_update.sh
    )
endif ()

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories(ocular PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# List files that install and/or package should provide.
# Each library should know what it want to distribute,
# which files are internal or intermediate and which are public library export.
install(TARGETS ocular DESTINATION ocular-bin)
install(FILES ocular.h DESTINATION ocular-include)