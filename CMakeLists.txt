cmake_minimum_required(VERSION 3.19)
project(ifj21 C)

set(CMAKE_C_STANDARD 11)

set(UTILS_SOURCES
        macros.h
        debug.h
        tests/tests.c
        errors.c
        )
set(DATASTRUCTURES
        bintree.c
        list.c
        stack.c
        dynstring.c
        )
set(PROJ_FILES ${DATASTRUCTURES} ${UTILS_SOURCES})

set(PARSER parser.c expressions.c)


# Compile options for a project.
add_compile_options(
        -Wall -Wextra -pedantic -g
        -DDEBUG
)

# The project itself.
add_executable(ifj21
        ifj21.c
        ${PROJ_FILES}
        ${PARSER}
        progfile.c
        scanner.c
        )

## Tests for scanner.
add_executable(test_scanner
        ${PROJ_FILES}
        progfile.c
        scanner.c
        )
target_compile_options(test_scanner PRIVATE -DSELFTEST_SCANNER)

# Stack tests.
add_executable(test_stack
        ${PROJ_FILES}
        list.c
        stack.c
        )
target_compile_options(test_stack PUBLIC -DSELFTEST_STACK)

# List tests.
add_executable(test_list
        ${PROJ_FILES}
        list.c
        )
target_compile_options(test_list PRIVATE -DSELFTEST_LIST)

#binary tree tests.
add_executable(test_bintree
        ${PROJ_FILES}
        bintree.c
        )
target_compile_options(test_bintree PRIVATE -DSELFTEST_BINTREE)

