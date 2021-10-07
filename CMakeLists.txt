cmake_minimum_required(VERSION 3.19)
project(ifj21 C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-DDEBUG -DDEBUG_SCANNER -Wall -Wextra -pedantic -g -Wno-unused-function -Wno-unused-variable)
#add_compile_options(-DDEBUG_SCANNER)

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
        )

# todo: what i want to do?
# create test_part of the project files, in which there will be
# MAIN functoins, which become main because of -DSELFTEST flag.
# So there's need to create separate compile options for different targets(e.g test_scanner, test_parser and so
#add_compile_options(-DSELFTEST)
#add_executable(test_scanner
#        scanner.c scanner.h
#        debug.h macros.h
#        dynstring_t.c dynstring_t.h
#        progfile.c progfile.h
#        tests/tests.c tests/tests.h
#        )
