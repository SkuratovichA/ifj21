#include "expressions.h"
#include "scanner.h"
#include "stdbool.h"

/**
 * Precedence functions.
 */

/**
 * f - represent rows of the precedence table.
 */
static int f[10] = {8, 0, 8, 6, 6, 4, 2, 1, 0, 0};

/**
 * g - represent columns.
 */
static int g[10] = {7, 7, 0, 7, 5, 1, 3, 7, 0, 0};

/**
 * @brief
 *
 * Return operator from the precedence table
 * using token information.
 *
 * @param token
 * @return op_list_t.
 */
static op_list_t get_op (token_t token) {
    switch (token.type) {
        case TOKEN_ID:      return OP_ID;
        case TOKEN_LPAREN:  return OP_LPAREN;
        case TOKEN_RPAREN:  return OP_RPAREN;
        case TOKEN_HASH:    return OP_HASH;
        case TOKEN_MUL:
        case TOKEN_DIV_F:
        case TOKEN_DIV_I:   return OP_MUL_GROUP;
        case TOKEN_ADD:
        case TOKEN_SUB:     return OP_ADD_GROUP;
        case TOKEN_STRCAT:  return OP_STRCAT;
        case TOKEN_COMMA:   return OP_COMMA;

        // TODO: detect if identifier is a function
        default: return OP_FUNC;
    }
}

/**
 * @brief Precedence functions error handling.
 * Check existence of relation between two operators.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @return bool.
 */
static bool precedence_check (op_list_t first_op, op_list_t second_op) {
    switch (first_op) {
        case_2(OP_ID, OP_RPAREN):
            switch (second_op) {
                case_3(OP_ID, OP_LPAREN, OP_FUNC):
                    return false;
            }
            break;
        case_2(OP_LPAREN, OP_COMMA):
            if (second_op == OP_DOLLAR) {
                return false;
            }
            break;
        case OP_FUNC:
            switch (second_op) {
                case_2(OP_LPAREN, OP_COMMA):
                    break;
                default:
                    return false;
            }
            break;
        case OP_DOLLAR:
            switch (second_op) {
                case_3(OP_RPAREN, OP_COMMA, OP_DOLLAR):
                    return false;
            }
            break;
    }

    return true;
}

/**
 * @brief Compare two operators using precedence functions.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @param cmp result of comparison.
 * >0 if first_op has a higher precedence,
 * =0 if operators have a similar precedence,
 * <0 if first_op has a lower precedence.
 * @return bool.
 */
static bool precedence_cmp (op_list_t first_op, op_list_t second_op, int *cmp) {
    if (precedence_check(first_op, second_op)) {
        *cmp = f[first_op] - g[second_op];
        return true;
    }

    return false;
}

/**
 * @brief Expression parsing driven by a precedence table.
 *
 * !rule <expr> -> // TODO
 *
 * @param pifle program file to pass in to scanner.
 * @return true or false.
 */
static bool Parse_expression(pfile_t *pfile) {

    return false;
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .parse = Parse_expression,
};
