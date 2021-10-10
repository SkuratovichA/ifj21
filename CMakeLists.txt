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
        )

add_compile_options(-DDEBUG -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
#add_compile_options(-DDEBUG_SCANNER)



# todo: what i want to do?
# create test_part of the project files, in which there will be
# MAIN functoins, which become main because of -DSELFTEST flag.
# So there's need to create separate compile options for different targets(e.g test_scanner, test_parser and so
add_compile_options(-DSELFTEST -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
add_executable(test_scanner
        scanner.c scanner.h
        debug.h macros.h
        dynstring.c dynstring.h
        progfile.c progfile.h
        tests/tests.c tests/tests.h
        )

