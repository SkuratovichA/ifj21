# @file scanner.h
#
# @brief Header file for scanner.
#
# @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
cmake_minimum_required(VERSION 3.16)
project(ifj21 C)
set(CMAKE_C_STANDARD 11)
set(PROJECT_NAME ifj21)

# definitions of cmake variables.
set(UTILS_SOURCES
        macros.h
        debug.h
        errors.c
        progfile.c
        scanner.c
        symstack.c
        tests/tests.c
        parser.c
        expressions.c
        semantics.c
        )
set(DATASTRUCTURES
        symtable.c
        list.c
        stack.c
        dynstring.c
        )
set(PROJ_FILES
        ${DATASTRUCTURES} ${UTILS_SOURCES}
        )
set(DEVELOP_FLAGS
        -Wno-unused-parameter -Wno-unused-function
        #-DDEBUG
        -DDEBUG_DYNSTRING
        )
add_compile_options(
        -Wall -Wextra -pedantic -g
        ${DEVELOP_FLAGS}
)

#adding targets
# The project itself.
add_executable(${PROJECT_NAME}
        ifj21.c
        ${PROJ_FILES}
        )
message(STATUS ${PROJECT_NAME} " execucumber compiled.")

# todo somehow make it more general
#parser tests
set(ATTRIBUTE parser)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")

#scanner tests
set(ATTRIBUTE scanner)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")

#stack tests
set(ATTRIBUTE stack)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")

#list tests
set(ATTRIBUTE list)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")

#dynstring tests.
set(ATTRIBUTE dynstring)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")

#expressions tests
set(ATTRIBUTE expressions)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")

#symtable tests
set(ATTRIBUTE symtable)
add_executable(test_${ATTRIBUTE}
        ${PROJ_FILES}
        )
target_compile_options(test_${ATTRIBUTE} PRIVATE -DSELFTEST_${ATTRIBUTE} -DDEBUG${ATTRIBUTE})
message(STATUS ${ATTRIBUTE} " execucumber compiled.")
