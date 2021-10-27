#pragma once

// Includes
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
#include "scanner.h"
#include "errors.h"
//

/**
 * In binary tree we store table of symbols.
 * Each identifier has field of validity.
 *      GTS - global table of symbols - has the highest priority
 *                                    - It is for Global variables and functions
 *      LTS - local table of symbols - each function, a conditional statement, or a loop has this. 
 *
 * In functions we have some validity levels. Smaller number means higher level.
 *
 */
typedef struct data {
    dynstring_t *i_name;        /**< identifier name. can be name of function or variable.*/
    int type;                   /**< Type of identifier can be FUNCTION or VARIABLE_TYPE */
    // unsigned int i_name_num;  /**< Int identifier. This value defines how we store data. Means name in numbers. */
} data;


typedef struct node node_t;

/**
 * An interface to access scanner functions
 */
extern const struct tree_interface_t Tree;

struct tree_interface_t {
    void (*delete_tree)(node_t *);

    int (*get_unique_id)(node_t *);

    bool (*insert)(node_t *, token_t);

    void (*dtor)(node_t );

    node_t *(*ctor)(token_t );

    node_t *(*find)(node_t *, token_t);
};
