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
    /**
     * @brief Expression in the return statement.
     *        semantic control of <return signature> x <its function signature> are performed in parser.c
     *
     * @param pfile_
     * @param received_signature is an initialized empty vector.
     * @param func_rets
     * @return true if successive parsing performed.
     */
    bool (*return_expressions)(pfile_t *, dynstring_t *, size_t);

    /**
     * @brief Default expression after = in the local assignment
     *        Or an expression in the conditional statement in cycles, conditions.
     *        semantic controls are performed inside parser.c
     *
     * @param pfile_
     * @param received_signature is an initialized empty vector.
     * @param type_expr_statement TYPE_EXPR_DEFAULT will not be casted to boolean.
     *                            TYPE_EXPR_CONDITIONAL will be casted to boolean.
     * @return true if successive parsing performed.
     */
    bool (*default_expression)(pfile_t *, dynstring_t *, type_expr_statement_t);

    /**
     * @brief Function calling or assignments in the local scope.
     * !rule [function_expression] -> id [func_call] | id [assign_id]
     * @param pfile_
     * @return true if successive parsing and semantic analysis of expressions performed.
     */
    bool (*function_expression)(pfile_t *);

    /**
     * @brief Function calling in the global scope. `id( ...`
     * !rule [global_expression] -> id [func_call]
     * @param pfile_
     * @return true if successive parsing and semantic analysis of expressions performed.
     */
    bool (*global_expression)(pfile_t *);
};

extern const struct expr_interface_t Expr;
