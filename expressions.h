/**
 * @file expressions.h
 *
 * @brief
 *
 * @author
 */
#pragma once

#include "progfile.h"
#include "macros.h"
#include "scanner.h"
#include "list.h"

/**
 * Add prefix before operator name.
 */
#define OP(name) _cat_2_(OP, name)

/**
 * Add prefix before item type.
 */
#define ITEM_TYPE(type) _cat_2_(ITEM_TYPE, type)

/**
 * Add prefix before expression type.
 */
#define EXPR(type) _cat_2_(EXPR, type)

/**
 * List of operators from the precedence table.
 */
typedef enum op_list {
    OP(ID),
    OP(LPAREN),
    OP(RPAREN),
    OP(MUL),
    OP(DIV_F),
    OP(DIV_I),
    OP(ADD),
    OP(SUB),
    OP(LT),
    OP(LE),
    OP(GT),
    OP(GE),
    OP(EQ),
    OP(NE),
    OP(HASH),
    OP(NOT),
    OP(STRCAT),
    OP(AND),
    OP(OR),
    OP(FUNC),
    OP(COMMA),
    OP(DOLLAR),
    OP(UNDEFINED)
} op_list_t;

/**
 * List of stack item types.
 */
typedef enum item_type {
    ITEM_TYPE(GT),
    ITEM_TYPE(LT),
    ITEM_TYPE(EQ),
    ITEM_TYPE(DOLLAR),
    ITEM_TYPE(EXPR),
    ITEM_TYPE(TOKEN),
} item_type_t;

/**
 * List of expression types.
 */
typedef enum expr_type {
    EXPR(DEFAULT),
    EXPR(ASSIGN),
    EXPR(FUNC),
    EXPR(INVALID)
} expr_type_t;

typedef struct stack_item {
    item_type_t type;
    token_t token;
} stack_item_t;

struct expr_interface_t {
    bool (*parse)(pfile_t *, bool, dynstring_t *);

    bool (*parse_expr_list)(pfile_t *, dynstring_t *);
};

extern const struct expr_interface_t Expr;
