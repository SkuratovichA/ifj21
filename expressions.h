#pragma once

#include "progfile.h"

struct expr_interface_i {
    bool (*parse)(pfile_t *);
};

extern const struct expr_interface_i Expr;
