/********************************************
 * Project name: IFJ - projekt
 * File: debug.h
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 * Debug MACROS
 *
 *  @package debug
 *  @file debug.h
 *  @brief This file our advanced Macroc for debuging. So we don't have to use print.
 *
 *  @author Aliaksandr Skuratovich
 */
#pragma once

#ifndef __APPLE__
#define __FUNCTION__ __func__
#endif


#ifdef DEBUG
    #include <assert.h>
    #include <stdio.h>
#include <string.h>

    #define debug_msg(...) debug_print(stdout, __VA_ARGS__)
    #define debug_msg_stderr(...) debug_print(stderr, __VA_ARGS__)
    #define debug_msg_s(...) fprintf(stdout, __VA_ARGS__)

    #define DEBUG_SEP "\033[0;36m<><><><><><><><>\033[0m\n"

    #define debug_todo(...) do{ \
        fprintf(stderr, "TODO: "); \
        debug_msg_stderr(__VA_ARGS__); \
    } while(0)

#define debug_print(_dst, ...) do { \
        fprintf((_dst), __FILE__":%.4d in %s %*s%s: ", \
            __LINE__, \
            __FUNCTION__, (int)(15 - strlen(__FUNCTION__))/*an indentation number*/, "", " "\
            ); \
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
#define debug_msg_s(...)
#define DEBUG_SEP
#endif

