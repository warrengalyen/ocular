cmake_minimum_required(VERSION 3.9)
project(ocular)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/bin") # .exe and .dll

add_subdirectory(lib)
add_subdirectory(dll)
add_subdirectory(demo)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Below line set Demo as startup application in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Demo)

# Add installer (it is called CPack) which brings all files together (.exe, .lib, .dll, and .h) and put into one file
include(InstallRequiredSystemLibraries)
include(CPack)