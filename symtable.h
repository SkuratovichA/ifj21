#pragma once

// Includes
#include "scanner.h"
#include "debug.h"
#include "errors.h"


typedef struct symtable symtable_t;

struct symtable_interface_t {
    symtable_t *(*ctor)(symtable_t *, token_t);
};

extern const struct symtable_interface_t Symtable;

