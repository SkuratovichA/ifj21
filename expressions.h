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

struct expr_interface_t {
    bool (*return_expressions)(pfile_t *, dynstring_t *);

    bool (*assignment_expression)(pfile_t *, dynstring_t *);

    bool (*function_expression)(pfile_t *);

    bool (*global_expression)(pfile_t *);
};

extern const struct expr_interface_t Expr;
