#pragma once

#include "progfile.h"
#include "macros.h"
#include "list.h"

/**
 * Add prefix before operator name
 */
#define OP(name) _cat_2_(OP, name)

/**
 * List of operators from the precedence table
 */
enum op_list {
    OP(ID),
    OP(LPAREN),
    OP(RPAREN),
    OP(HASH),
    OP(MUL_GROUP),
    OP(ADD_GROUP),
    OP(STRCAT),
    OP(FUNC),
    OP(COMMA),
    OP(DOLLAR)
};

struct expr_interface_t {
    bool (*parse)(pfile_t *);
};

extern const struct expr_interface_t Expr;
