#include "expressions.h"
#include "scanner.h"
#include "stdbool.h"

/**
 * @brief
 *
 * Return operator from the precedence table
 * using token information.
 *
 * @param token
 * @return int.
 */
static int get_op (token_t token) {
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
