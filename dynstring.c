//
// Created by xskura01 on 23.09.2021.
//

#include "dynstring.h"
#include "dynstring.h"
#include <stdlib.h>

static inline bool is_on_heap(const string *str) {
    return str->is_on_heap;
}

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

static void Str_create_empty(string *str) {
    str->is_on_heap = false;
    str->stack_size = 0;
    str->stack_str[0] = '\0';
}

static char *Str_c_str(string *str) {
    return (is_on_heap(str)) ? str->heap_str : str->stack_str;
}

static size_t Str_length(const string *str) {
    return (is_on_heap(str)) ? str->size : str->stack_size;
}

static void Str_free(string *str) {
    if (str != NULL && is_on_heap(str)) {
        free(str->heap_str);
        str->is_on_heap = false;
    }
}

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

// interface to use when dealing with strings
const struct string_op_struct_t Dynstring = {
        .create         = Str_create,        // create a string from a char *
        .create_empty   = Str_create_empty,  // string constructor
        .length         = Str_length,        // get string length
        .c_str          = Str_c_str,         // get char * from string
        .append_char    = Str_append_char,
        .free           = Str_free,
        .create_onheap  = Str_create_onheap,
};
