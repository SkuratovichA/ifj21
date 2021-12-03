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
 * List of the operators of the precedence table.
 */
typedef enum op_list {
    OP_ID,
    OP_CARET,
    OP_MUL,
    OP_DIV_I,
    OP_DIV_F,
    OP_PERCENT,
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
    OP_MINUS_UNARY,
    OP_STRCAT,
    OP_AND,
    OP_OR,
    OP_DOLLAR,
    OP_UNDEFINED,
} op_list_t;

/**
 * List of stack item types.
 */
typedef enum item_type {
    ITEM_TYPE_LT,
    ITEM_TYPE_DOLLAR,
    ITEM_TYPE_EXPR,
    ITEM_TYPE_TOKEN,
} item_type_t;

typedef enum type_expr_statement {
    TYPE_EXPR_CONDITIONAL,
    TYPE_EXPR_DEFAULT,
} type_expr_statement_t;

/**
 * Item of precedence analyse stack
 */
typedef struct stack_item {
    item_type_t type;
    token_t token;
    dynstring_t *expression_type;
} stack_item_t;

struct expr_interface_t {
    bool (*return_expressions)(pfile_t *, dynstring_t *, size_t);

    bool (*default_expression)(pfile_t *, dynstring_t *, type_expr_statement_t);

    bool (*function_expression)(pfile_t *);

    bool (*global_expression)(pfile_t *);
};

extern const struct expr_interface_t Expr;
