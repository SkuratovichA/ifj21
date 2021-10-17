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
 *      GTS - global table of symbols - has highest priority
 *                                    - It is for Global variables and functions
 *      LTS - local table of symbols - each function, a conditional statement, or a loop has this. 
 *
 * In functions we have some validity levels. Smaller number means higher level.
 *
 */
typedef struct data {
    dynstring_t i_name;          /**< identifier name. can be name of function or variable.*/
    char type;              /**< Type of identifier can be FUNCTION or VARIABLE */
    // unsigned int i_name_num;  /**< Int identifier. This value defines how we store data. Means name in numbers. */
} data;


/**
 * Binary tree structure. each node has pointer to two child nodes left and right.
 * Each node can have 0-2 child node.
 * Each node has just one parent.
 *
 * .data has to be greater than .left child .data
 * .data has to be less than .right child .data
 */
typedef struct node {
    struct data *data;   /**< Data stored in node.   */
    struct node *l_child;       /**< Pointer to left side child.  */
    struct node *r_child;       /**< Pointer to right side child. */
    unsigned int validity_field; /**< We are creating trees for each validity field */
} node;


/**
 * An interface to access scanner functions
 */
extern const struct tree_op_struct Tree;

struct tree_op_struct {

    void (*delete_tree)(node *);

    void (*delete_node)(node *);

    int (*get_unique_id)(node *);

    bool (*insert)(node *, token_t *, unsigned int);

    node *(*create_tree)(token_t *, unsigned int);

    node *(*create_node)(token_t *);

    node *(*find_node)(node *, dynstring_t);
};
