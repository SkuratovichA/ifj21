#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "errors.h"
#include "debug.h" // debug macros


typedef struct dynstring dynstring_t;

/**
 * A structure that store pointers to all the functions from dynstring_t.c. So we can use them in different files as interface.
 */
struct dynstring_interface_t {

    /**
     * @brief Create a dynstring_t from a c_string, with len(@param str + STRSIZE).
     *
     * @param str Output dynstring_t. str is data struct defined in dynstring_t.h
     * @param s Char that we want to convert to dynstring_t.
     * @return non-null pointer to dynstring_t object.
     */
    dynstring_t *(*ctor)(const char *);

    /**
     * @brief Appends a character shrunk.
     *
     * @param str dynstring_t heap structure.
     * @param ch char to append.
     */
    void (*append)(dynstring_t *, char);

    /**
     * @brief Compares two dynstrings.
     *
     * @param s
     * @param s1
     * @returns -1, 0, 1 depends on lexicographical ordering of two strings.
     */
    int (*cmp)(dynstring_t *, dynstring_t *);

    /**
     * @brief Return len of given dynstring_t.
     *
     * @param str
     * @return len of dynstring_t
     */
    size_t (*len)(dynstring_t *);

    /**
     * @brief Frees memory.
     *
     * @param str dynstring_t to dtor.
     */
    void (*dtor)(dynstring_t *);

    /**
     * @brief Get char * (c dynstring_t ending with '\0') from dynstring_t.
     *
     * @param str
     * @return c dynstring_t representation.
     */
    char *(*c_str)(dynstring_t *);

    /**
     * @brief Concatenate two dynstrings in the not very efficient way.
     *
     * @param s1
     * @param s2
     * @returns new dysntring, which is product of s1 and s2.
     */
    dynstring_t *(*cat)(dynstring_t *, dynstring_t *);

    /**
    * @brief Clears the dynstring. Set everything to 0 except allocated_size.
    *
    * @param str string to clear.
    */
    void (*clear)(dynstring_t *);
};


// Functions from dynstring_t.c will be visible in different file under Dynstring name.
extern const struct dynstring_interface_t Dynstring;
