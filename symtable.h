#pragma once

// Includes
#include "scanner.h"
#include "debug.h"
#include "scanner.h"
#include "errors.h"
#include "dynstring.h"
#include "bintree.h"

typedef struct sym_table_array sym_t;

typedef struct scope_table scope_t;

extern const struct symtable_interface_t Symt;

struct symtable_interface_t {

    sym_t *(*st_ctor)();

    scope_t *(*Ctor)(sym_t *, token_t, scope_t *);

    scope_t *(*get_parent_scope)(scope_t *);

    int (*get_scope_id)(scope_t *);

    int (*get_parent_scope_id)(scope_t *);

    void (*st_dtor)(sym_t *);
};
