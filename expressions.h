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

// Added for compile
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

struct expr_interface_t {
    bool (*return_expressions)(pfile_t *, dynstring_t *);

    bool (*default_expression)(pfile_t *, dynstring_t *);

    bool (*function_expression)(pfile_t *);

    bool (*global_expression)(pfile_t *);
};

extern const struct expr_interface_t Expr;
