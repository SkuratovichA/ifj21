# @file CMakeLists.h
#
# @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
cmake_minimum_required(VERSION 3.16)
project(ifj21 C)
set(CMAKE_C_STANDARD 11)
set(PROJECT_NAME ifj21)

# definitions of cmake variables.
set(UTILS_SOURCES
        src/macros.h
        src/debug.h
        src/errors.c
        src/progfile.c
        src/scanner.c
        src/symstack.c
        src/parser.c
        src/expressions.c
        src/semantics.c
        src/code_generator.c
        )
set(DATASTRUCTURES
        src/symtable.c
        src/list.c
        src/stack.c
        src/dynstring.c
        )
set(PROJ_FILES
        ${DATASTRUCTURES} ${UTILS_SOURCES}
        )

set(DEVELOP_FLAGS
        #-Wno-unused-parameter
        -Wno-unused-function
        -Wall
        -Wextra
        -pedantic
        -DDEBUG
        -DDEBUG_DYNSTRING
        )
set(CODE_COVERAGE_FLAGS
        -ftest-coverage -fprofile-arcs --coverage
        )
add_compile_options(
        -Wall -Wextra -pedantic -g
        ${DEVELOP_FLAGS}
        ${CODE_COVERAGE_FLAGS}
)
# flag for enabling code coverage on mac
set(CMAKE_EXE_LINKER_FLAGS
        --coverage
        )

#adding targets
# The project itself.
add_executable(${PROJECT_NAME}
        src/ifj21.c
        ${PROJ_FILES}
        )
message(STATUS ${PROJECT_NAME} " execucumber compiled.")

#testgen file
# program to generate tests inputs.
add_executable(testgen
        tests/testgen.c
        ${PROJ_FILES}
        )