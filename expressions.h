#pragma once

#include "progfile.h"

struct expr_interface_t {
    bool (*parse)(pfile_t *);
};

extern const struct expr_interface_t Expr;
