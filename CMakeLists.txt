cmake_minimum_required(VERSION 3.9)
project(demo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER gcc)

add_executable(demo
    ocular.c
    demo.c)