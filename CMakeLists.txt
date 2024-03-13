cmake_minimum_required(VERSION 3.27)
project(filesystem C)

set(CMAKE_C_STANDARD 11)

add_executable(filesystem main.c
        file.c
        file.h)
