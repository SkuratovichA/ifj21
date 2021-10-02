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
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */


#include "dynstring.h"
#include <stdlib.h>


/**
 * @brief Find if string is on heap or stack.
 *
 * @param str String that is checked.
 * @return str->is_on_heap
 */
static inline bool is_on_heap(const string *str) {
    return str->is_on_heap;
}

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
    if (length < sizeof(string)) {
        strcpy(str->stack_str, s);
        str->stack_size = length;
        str->is_on_heap = false;
    } else {
        str->heap_str = (char *) calloc(length + 1, 1);
        if (!str->heap_str) {
            return false;
        }
        strcpy(str->heap_str, s);
        str->size = length;
        str->allocated_size = length + 1;
        str->is_on_heap = true;
    }
    return true;
}

/**
 * @brief Create empty string.
 *
 * Union variables are set to.
 * str->is_on_heap = false;
 * str->stack_size = 0;
 * str->stack_str[0] = '\0';
 *
 * @param str
 * @return void
 */
static void Str_create_empty(string *str) {
    str->is_on_heap = false;
    str->stack_size = 0;
    str->stack_str[0] = '\0';
}


/**
 * @brief Get char * (c string ending with '\0') from string.
 *
 * @param str
 * @return Pointer to char. Could be on heap or stack.
 */
static char *Str_c_str(string *str) {
    return (is_on_heap(str)) ? str->heap_str : str->stack_str;
}

/**
 * @brief Return length of given string.
 *
 * @param str
 * @return size of string
 */
static size_t Str_length(const string *str) {
    return (is_on_heap(str)) ? str->size : str->stack_size;
}

/**
 * @brief Call free() on string if string was allocated on the heap.
 *
 * @param str
 * @return void
 */
static void Str_free(string *str) {
    if (str != NULL && is_on_heap(str)) {
        free(str->heap_str);
        str->is_on_heap = false;
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
    if (is_on_heap(str)) {
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
    } else {
        // move stack str to heap if too long
        if (str->stack_size + 1U >= sizeof(string)) {
            size_t new_length = str->stack_size + 1;
            // allocate string on the heap
            void *tmp = malloc(new_length + 1);
            if (!tmp) {
                return false;
            }
            // copy contents and set size
            strcpy(tmp, str->stack_str);
            str->heap_str = tmp;
            str->size = new_length;
            str->heap_str[new_length - 1] = ch;
            str->heap_str[new_length] = '\0';
            str->allocated_size = new_length + 1;
            str->is_on_heap = true;
        } else {
            str->stack_str[str->stack_size++] = ch;
            str->stack_str[str->stack_size] = '\0';
        }
    }
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
    str->is_on_heap = true;
    assert((bool) NULL == false && "NULL must be false");
    assert((bool) !NULL == true && "!NULL must be true");
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
        .create_empty   = Str_create_empty,
        .length         = Str_length,
        .c_str          = Str_c_str,         // get char * from string
        .append_char    = Str_append_char,
        .free           = Str_free,
        .create_onheap  = Str_create_onheap,
        .cmp            = Str_cmp,
};
