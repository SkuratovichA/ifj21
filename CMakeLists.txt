cmake_minimum_required(VERSION 3.19)
project(ifj21 C)

set(CMAKE_C_STANDARD 11)

add_executable(ifj21 interpret.c scanner.c scanner.h errors.c errors.h)