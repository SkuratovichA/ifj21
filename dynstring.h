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
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h" // debug macros

#define _SIZE_BITS __WORDSIZE - 1  // 0-63 == 8 BYTES


/**
 * A Union represented string.
 * Size of the structure is 24 bits. String can be
 */
union string_t {
    /**
     * If string is big enough(> 24 bytes) we need to create it on heap.
     */
    struct {
        char *heap_str;
        size_t allocated_size;    /**< Allocated size on heap*/
        size_t size: _SIZE_BITS;  /**< String length */
        size_t is_on_heap: 1;     /**< Use 1 bit for a flag denoting the string is allocated on heap */
    };
    /**
     * If string is on stack, the last bit(which aliases with is_on_heap is '\0', which is false)
     */
    struct {
        uint8_t stack_size; /**< Size on stack: 1 byte */
        char stack_str[sizeof(size_t) * 3 - 1]; /**< Array where is string stored: 23 bytes */
    };
};

typedef union string_t string;

/**
 * A structure that store pointers to all the functions from dynstring.c. So we can use them in different files as interface.
 */
struct string_op_struct_t {
    bool (*create)(string *, const char *);
    bool (*append_char)(string *, char);
    void (*create_empty)(string *);
    size_t (*length)(const string *);
    char *(*c_str)(string *);
    void (*free)(string *);
    bool (*create_onheap)(string *);

    int (*cmp)(string, const char *);
};

extern const struct string_op_struct_t Dynstring; // Functions from dynstring.c will be visible in different file under Dynstring name.
