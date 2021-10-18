cmake_minimum_required(VERSION 3.19)
project(ifj21 C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-DDEBUG -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
add_executable(ifj21
        ifj21.c scanner.c
        errors.c errors.h
        scanner.h debug.h
        dynstring.c dynstring.h
        macros.h
        progfile.c progfile.h
        dynstring.c dynstring.h
        parser.c parser.h
        tests/tests.c tests/tests.h
        expressions.c expressions.h
        list.c list.h
        list.h list.c stack.c stack.h)

add_compile_options(-DSELFTEST_SCANNER -DDEBUG -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
add_executable(test_scanner
        errors.c errors.h
        scanner.c scanner.h
        debug.h macros.h
        dynstring.c dynstring.h
        progfile.c progfile.h
        tests/tests.c tests/tests.h
        )

add_compile_options(-DSELFTEST_LIST -DDEBUG -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
add_executable(test_list
        errors.c errors.h
        debug.h macros.h
        dynstring.c dynstring.h
        progfile.c progfile.h
        list.c list.h
        tests/tests.c tests/tests.h
        )

add_compile_options(-DSELFTEST_STACK -DDEBUG -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
add_executable(test_stack
        errors.c errors.h
        debug.h macros.h
        dynstring.c dynstring.h
        progfile.c progfile.h
        list.c list.h
        tests/tests.c tests/tests.h
        )
##################################################3
