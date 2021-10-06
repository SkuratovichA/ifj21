/********************************************
 * Project name: IFJ - projekt
 * File: bin_tree.h
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 *
 *
 *  @package bin_tree
 *  @file bin_tree.h
 *  @brief Headers for bin_tree.c
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */


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
 * Each identificator has field of validity. If not GTS identificator is generated by function asci value.
 *      GTS - global table of symbols - has Highest priority
 *                                    - It is for Global variables and functions
 *      LTS - local table of symbols - each function or for has this
 *
 * In functions we have some validity levels. Smaller number means higher level.
 *
 */
typedef struct data {
    char *i_name;          /**< Identificator name. Could be name of function or variable.*/
 //   unsigned int i_name_num;  /**< Int identificator. This value defines how we store data. Means name in numbers. */
    char type;              /**< Type of identificator could be FUNCTION or VARIABLE */
} data;


/**
 * Binary tree structure. each node has pointer to two child nodes left and right.
 * Each node can have 0-2 child node.
 * Each node has just one parent.
 *
 * .data has to be more them .left child .data
 * .data has to be less them .right child .data
 */
typedef struct node {
    struct data *data;   /**< Data stored in node.   */
    struct node *l_child;       /**< Pointer to left side child.  */
    struct node *r_child;       /**< Pointer to right side child. */
    unsigned  int validity_field; /**< We are creating trees for each validity field */
} node;


/**
 * An interface to access scanner functions
 */
extern const struct tree_op_struct Tree;

struct tree_op_struct {
    void (*print_tree)();
    bool (*insert)(node *,token_t *, unsigned int);
    void (*delete_tree)(node *);
    void (*delete_node)(node *);
    int (*get_unique_id)(node *);
    node * (*create_tree)(token_t *, unsigned int);
    node * (*create_node)(token_t *);

};
