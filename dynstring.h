/********************************************
 * Project name: IFJ - projekt
 * File: dynstring.h
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 * //GENERAL INFO
 *
 *  @package dynstring
 *  @file dynstring.h
 *  @brief Header file for dynstring.c with data types definitions.
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin - move dynstring to the heap
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h" // debug macros

#define _SIZE_BITS __WORDSIZE - 1  // 0-63 == 8 BYTES


/**
 * A structure represented string
 */
struct string_t {
    size_t allocated_size;    /**< Allocated size on heap */
    size_t size;              /**< String length */
    char heap_str[];
};

typedef struct string_t string;

/**
 * A structure that store pointers to all the functions from dynstring.c. So we can use them in different files as interface.
 */
struct string_op_struct_t {
    void (*create)(string *, const char *);
    void (*append_char)(string *, char);
    size_t (*length)(const string *);
    char *(*c_str)(string *);
    void (*free)(string *);
    void (*create_onheap)(string *);

    int (*cmp)(string, const char *);
};

extern const struct string_op_struct_t Dynstring; // Functions from dynstring.c will be visible in different file under Dynstring name.
