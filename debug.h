//
// Created by xskura01 on 23.09.2021.
//
#pragma once

#ifndef __APPLE__
#define __FUNCTION__ __func__
#endif


#ifdef DEBUG
    #include <assert.h>
    #include <stdio.h>

    #define debug_msg(...) debug_print(stdout, __VA_ARGS__)
    #define debug_msg_stderr(...) debug_print(stderr, __VA_ARGS__)
    #define debug_msg_s(...) fprintf(stdout, __VA_ARGS__)

    #define DEBUG_SEP "\033[0;36m<><><><><><><><>\033[0m\n"

    #define debug_todo(...) do{ \
        fprintf(stderr, "TODO: "); \
        debug_msg_stderr(__VA_ARGS__); \
    } while(0)

    #define debug_print(_dst, ...) do { \
        fprintf((_dst), "- %s:%d in %s: ", __FILE__, __LINE__, __FUNCTION__);\
        fprintf((_dst), __VA_ARGS__);\
    } while(0)

    #define debug_assert(cond) assert((cond))
#else
// undef debug macros
#define debug_err(...)
#define debug_msg(...)
#define debug_msg_stdout(...)
#define debug_msg_stderr(...)
#define debug_todo(...)
#define debug_assert(cond)
#define DEBUG_SEP
#endif

