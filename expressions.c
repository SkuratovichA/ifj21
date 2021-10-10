#include "expressions.h"
#include "stdbool.h"

static bool Parse_expression(progfile_t *pfile) {
    return false;
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface Expr = {
        .parse = Parse_expression,
};
