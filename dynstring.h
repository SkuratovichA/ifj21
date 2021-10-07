/********************************************
 * Project name: IFJ - projekt
 * File: dynstring_t.h
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
 *  @package dynstring_t
 *  @file dynstring_t.h
 *  @brief Header file for dynstring_t.c with data types definitions.
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin - move dynstring_t to the heap
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "errors.h"
#include "debug.h" // debug macros


/**
 * A structure represented dynstring_t
 */
typedef struct dynstring {
    size_t allocated_size;    /**< Allocated len on heap */
    size_t len;              /**< String length */
    char *str;
} dynstring_t;


/**
 * A structure that store pointers to all the functions from dynstring_t.c. So we can use them in different files as interface.
 */
typedef struct dynstring_interface_t {
    /**
     * @brief Create a dynstring_t from a c_string, with length(@param str + STRSIZE).
     *
     * @param str Output dynstring_t. str is data struct defined in dynstring_t.h
     * @param s Char that we want to convert to dynstring_t.
     * @return non-null pointer to dynstring_t object.
     */
    dynstring_t (*create)(const char *);

    /**
     * @brief Appends a character shrunk.
     *
     * @param str dynstring_t heap structure.
     * @param ch char to append.
     */
    void (*append_char)(dynstring_t *, char);

    /**
     * @brief Compares two dynstrings.
     *
     * @param s
     * @param s1
     * @returns -1, 0, 1 depends on lexicographical ordering of two strings.
     */
    int (*cmp)(dynstring_t, dynstring_t);

    /**
     * @brief Return length of given dynstring_t.
     *
     * @param str
     * @return len of dynstring_t
     */
    size_t (*length)(dynstring_t *);

    /**
     * @brief Frees memory.
     *
     * @param str dynstring_t to free.
     */
    void (*free)(dynstring_t *);

    /**
     * @brief Get char * (c dynstring_t ending with '\0') from dynstring_t.
     *
     * @param str
     * @return c dynstring_t representation.
     */
    char *(*c_str)(dynstring_t);

    /**
     * @brief Concatenate two dynstrings in the not very efficient way.
     *
     * @param s1
     * @param s2
     * @returns new dysntring, which is product of s1 and s2.
     */
    dynstring_t (*cat)(dynstring_t *, dynstring_t *);

    /**
    * @brief Clears the dynstring. Set everything to 0 except allocated_size.
    *
    * @param str string to clear.
    */
    void (*clear)(dynstring_t *);
} dynstring_interface;


// Functions from dynstring_t.c will be visible in different file under Dynstring name.
extern const dynstring_interface Dynstring;
