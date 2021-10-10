#pragma once

#include "progfile.h"

struct expr_interface {
    bool (*parse)(pfile_t *);
};

extern const struct expr_interface Expr;
