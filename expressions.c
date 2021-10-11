#include "expressions.h"
#include "stdbool.h"


/**
 * @brief Expression parsing driven by a precedence table.
 *
 * !rule <expr> -> your mother
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
const struct expr_interface_i Expr = {
        .parse = Parse_expression,
};
