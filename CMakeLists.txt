cmake_minimum_required(VERSION 3.31)
project(c_learn C)

set(CMAKE_C_STANDARD 17)

file(GLOB_RECURSE SRC_FILES src/*.c)
add_executable(c_learn main.c ${SRC_FILES})
include_directories(include)
