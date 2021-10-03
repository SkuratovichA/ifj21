/********************************************
 * Project name: IFJ - projekt
 * File: dynstring.c
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 * This file can create dynamic size strings. If the string is short, it is created on stack else it is created on heap.
 *
 *  @package dynstring
 *  @file dynstring.c
 *  @brief Contain function for operations with string. String is represented by data union str.
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 */


#include "dynstring.h"
#include <stdlib.h>
#include "assert.h"



/**
 * @brief Create a string from a char *
 *
 * @param str Output string. str is data struct defined in dynstring.h
 * @param s Char that we want to convert to string.
 * @return true when successfully converted. False when Calloc error.
 *
 */
static bool Str_create(string *str, const char *s) {
    size_t length = strlen(s);
    str->heap_str = (char *) calloc(length + 1, 1);
    if (!str->heap_str) {
        return false;
    }
    strcpy(str->heap_str, s);
    str->size = length;
    str->allocated_size = length + 1;

    return true;
}

/**
 * @brief Get char * (c string ending with '\0') from string.
 *
 * @param str
 * @return Pointer to char. Could be on heap or stack.
 */
static char *Str_c_str(string *str) {
    return str->heap_str;
}

/**
 * @brief Return length of given string.
 *
 * @param str
 * @return size of string
 */
static size_t Str_length(const string *str) {
    return str->size;
}

/**
 * @brief Call free() on string if string was allocated on the heap.
 *
 * @param str
 * @return void
 */
static void Str_free(string *str) {
    if (str != NULL) {
        free(str->heap_str);
    }
}

/**
 * @brief Appends given character to end of string.
 *
 * @param str
 * @param ch Character that we'll be appending.
 * @return true when xxx
 */
static bool Str_append_char(string *str, char ch) {
    // new_char + null byte = 2
    if (str->size + 2 > str->allocated_size) {
        str->allocated_size *= 2;
        void *tmp = realloc(str->heap_str, str->allocated_size);
        if (!tmp) {
            free(str->heap_str);
            return false;
        }
        str->heap_str = tmp;
    }
    str->heap_str[str->size++] = ch;
    str->heap_str[str->size] = '\0';

    return true;
}

/**
 * @brief Allocate string on heap.
 *
 * @param str
 * @return True if success. If given pointer is NULL return false. If Calloc failed the program ends.
 */
static bool Str_create_onheap(string *str) {
    if (!str) {
        return false;
    }

    str->heap_str = calloc(1, (str->allocated_size = 42)); // here's the answer ...
    str->size = 0;
    // I believe this is never happens
//    assert((bool) NULL == false && "NULL must be false");
//    assert((bool) !NULL == true && "!NULL must be true");
    return (bool) str->heap_str; // NULL == 0 == false
}

/**
 * @brief Compare dynstring and char* using strcmp
 *
 * @param s
 * @param s1
 * @return -1, 0, 1 depends on lexicographical ordering of two strings.
 */
static int Str_cmp(string s, const char *s1) {
    return strcmp(Str_c_str(&s), s1);
}

// TODO: implement functions Str_concat{str, dynstr}

/**
 * Interface to use when dealing with strings.
 * Functions are in struct so we can use them in different files.
 */
const struct string_op_struct_t Dynstring = {
        /*@{*/
        .create         = Str_create,        // create a string from a char *
        .length         = Str_length,
        .c_str          = Str_c_str,         // get char * from string
        .append_char    = Str_append_char,
        .free           = Str_free,
        .create_onheap  = Str_create_onheap,
        .cmp            = Str_cmp,
};
