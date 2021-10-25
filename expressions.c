#include "expressions.h"
#include "scanner.h"
#include "stdbool.h"


/**
 * @brief Expression parsing driven by a precedence table.
 *
 * !rule <expr> -> // TODO
 *
 * @param pifle program file to pass in to scanner.
 * @return true or false.
 */
static bool Parse_expression(pfile_t *pfile) {
    Scanner.get_next_token(pfile);
    return true;
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .parse = Parse_expression,
};
