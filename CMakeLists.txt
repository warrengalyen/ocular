cmake_minimum_required(VERSION 3.9)
project(demo)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/bin") # .exe and .dll

add_executable(demo
    ocular.c
    demo.c browse.h)