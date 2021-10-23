#pragma once

// Includes
#include "scanner.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include "debug.h"
#include "errors.h"


extern struct token_t;

typedef struct sym_table {
    struct sym_table *parent;        /**< */
    struct sym_table *child;         /**< */
    struct node_t *tree;             /**< */
    int scope_index;                 /**< */
} sym_table;

typedef struct sym_table sym_t;


extern const struct sym_table_op_struct Symtable;

struct sym_table_op_struct {

    sym_t *(*ctor)(sym_t *, token_t *);


};