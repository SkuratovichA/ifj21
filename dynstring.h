//
// Created by xskura01 on 23.09.2021.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h" // debug macros

#define _SIZE_BITS __WORDSIZE - 1

union string_t {
    struct {
        char *heap_str;
        size_t allocated_size;
        size_t size: _SIZE_BITS;
        size_t is_on_heap: 1; // use 1 bit for a flag denoting the string is allocated on heap
    };
    struct {
        uint8_t stack_size;
        char stack_str[sizeof(char *) + 2 * sizeof(size_t) - 1];
    };
};

typedef union string_t string;

struct string_op_struct_t {
    bool (*create)(string *, const char *);
    bool (*append_char)(string *, char);
    void (*create_empty)(string *);
    size_t (*length)(const string *);
    char *(*c_str)(string *);
    void (*free)(string *);
    bool (*create_onheap)(string *);
};

extern const struct string_op_struct_t Dynstring;
