cmake_minimum_required(VERSION 3.19)
project(ifj21 C)

set(CMAKE_C_STANDARD 11)

#SET(CMAKE_C_FLAGS  "-DDEBUG -Wall -Werror -Wextra -pedantic -g")
#SET(CMAKE_CXX_FLAGS  "-DDEBUG -Wall -Werror -Wextra -pedantic -g")
add_compile_options(-DDEBUG -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)

add_executable(ifj21
        interpret.c scanner.c
        errors.c errors.h
        scanner.h debug.h
        dynstring.c dynstring.h
        macros.h
        progfile.c progfile.h
        dynstring.c dynstring.h)
