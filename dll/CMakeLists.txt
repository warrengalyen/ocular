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

set(MODULE_FILES
    ../lib/auxiliary.c
    ../lib/core.c
    ../lib/color.c
    ../lib/threshold.c
    ../lib/util.c
    ../lib/ocr.c
    ../lib/palette.c
    ../lib/dither.c
    ../lib/quanitize.c
    ../lib/retinex.c
    ../lib/distort_filters.c
    ../lib/edge_filters.c
    ../lib/blur_filters.c
    ../lib/hazeremoval.c
    ../lib/fft.c
    ../lib/ocular.c
    dlib_export.h
)
if (WIN32)
    # prevent exporting all symbols
    set (MODULE_FILES ${MODULE_FILES} ocular.def) 
endif ()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/demo_ui")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/bin")

add_library(ocular_dll SHARED ${MODULE_FILES})

#include(GenerateExportHeader)
#generate_export_header(ocular)

set_target_properties(ocular_dll PROPERTIES PREFIX "")
set_target_properties(ocular_dll PROPERTIES OUTPUT_NAME "ocular")

# Copy compiled dll to demo_ui for testing
# check if demo_ui exists
# move up one level
if(EXISTS "${CMAKE_SOURCE_DIR}/demo_ui")
    message(STATUS "Copying compiled dll to demo_ui for testing")
    add_custom_command(
        TARGET ocular_dll POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/bin/ocular.dll" "${CMAKE_SOURCE_DIR}/demo_ui/bin/debug"
        COMMENT "Copying compiled dll to demo_ui for testing"
    )
endif()

# Make sure the compiler can find include files for our library
# when other libraries or executables link to
target_include_directories(ocular_dll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# List files that install and/or package should provide.
# Each library should know what it want to distribute,
# which files are internal or intermediate and which are public library export.
install(TARGETS ocular_dll DESTINATION ocular_dll-bin)
install(FILES ocular.h DESTINATION ocular_dll-include)