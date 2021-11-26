/**
 * @file expressions.h
 *
 * @brief Header file for expression parser.
 *
 * @author Evgeny Torbin <xtorbi00@vutbr.cz>
 */

#pragma once

#include "progfile.h"
#include "scanner.h"
#include "list.h"

/**
 * List of operators from the precedence table.
 */
typedef enum op_list {
    OP_ID,
    OP_LPAREN,
    OP_RPAREN,
    OP_MUL,
    OP_DIV_F,
    OP_DIV_I,
    OP_ADD,
    OP_SUB,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_HASH,
    OP_NOT,
    OP_STRCAT,
    OP_AND,
    OP_OR,
    OP_FUNC,
    OP_COMMA,
    OP_DOLLAR,
    OP_UNDEFINED
} op_list_t;

/**
 * List of stack item types.
 */
typedef enum item_type {
    ITEM_TYPE_GT,
    ITEM_TYPE_LT,
    ITEM_TYPE_EQ,
    ITEM_TYPE_DOLLAR,
    ITEM_TYPE_EXPR,
    ITEM_TYPE_TOKEN,
} item_type_t;

/**
 * List of expression types.
 */
typedef enum expr_type {
    EXPR_DEFAULT,
    EXPR_RETURN,
    EXPR_GLOBAL,
    EXPR_FUNC,
    EXPR_INVALID
} expr_type_t;

/**
 * Item of precedence analyse stack
 */
typedef struct stack_item {
    item_type_t type;
    token_t token;
    dynstring_t *ret_types;
} stack_item_t;

struct expr_interface_t {
    bool (*parse)(pfile_t *, expr_type_t, dynstring_t *);
    bool (*parse_expr_list)(pfile_t *, expr_type_t, dynstring_t *, list_t *);
};

extern const struct expr_interface_t Expr;
