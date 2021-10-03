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
    /**
     * Create a string on heap
     */
    char *heap_str;
    size_t allocated_size;    /**< Allocated size on heap*/
    size_t size: _SIZE_BITS;  /**< String length */
};

typedef struct string_t string;

/**
 * A structure that store pointers to all the functions from dynstring.c. So we can use them in different files as interface.
 */
struct string_op_struct_t {
    bool (*create)(string *, const char *);
    bool (*append_char)(string *, char);
    size_t (*length)(const string *);
    char *(*c_str)(string *);
    void (*free)(string *);
    bool (*create_onheap)(string *);

    int (*cmp)(string, const char *);
};

extern const struct string_op_struct_t Dynstring; // Functions from dynstring.c will be visible in different file under Dynstring name.
