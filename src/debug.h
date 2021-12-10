/**
 * @file debug.c
 *
 * @brief Macros for debugging.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#pragma once

#ifndef __APPLE__
#define __FUNCTION__ __func__
#endif


#ifdef DEBUG

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string.h>

#define __FILENAME__ ((strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__))

#define debug_msg(...) debug_print(stderr, __VA_ARGS__)
#define debug_msg_stdout(...) debug_print(stdout, __VA_ARGS__)
#define debug_msg_s(...) fprintf(stderr, __VA_ARGS__)

#define DEBUG_SEP "\033[0;36m<><><><><><><><>\033[0m\n"

#define debug_todo(...)                \
    do {                               \
        fprintf(stderr, "TODO: ");     \
        debug_msg(__VA_ARGS__);        \
    } while(0)

#define debug_print(_dst, ...)                          \
    do {                                                \
        fprintf((_dst),"%s:%.4d in %s %*s%s: ",         \
            __FILENAME__,                               \
            __LINE__,                                   \
            __FUNCTION__,                               \
             /*an indentation number*/                  \
            (int)(20 - strlen(__FUNCTION__)),           \
            "", " "                                     \
            );                                          \
        fprintf((_dst), __VA_ARGS__);                   \
    } while(0)

#define debug_assert(cond)                     \
    do {                                      \
        if (!(cond)) {                        \
            debug_msg("ASSERTION FAILED\n");  \
        }                                     \
       soft_assert((cond), ERROR_INTERNAL);   \
    } while(0)

#else
// undef debug macros
#define debug_err(...)
#define debug_msg(...)
#define debug_msg_stdout(...)
#define debug_msg_stderr(...)
#define debug_todo(...)
#define debug_assert(cond)
#define debug_msg_s(...)
#define DEBUG_SEP
#endif

