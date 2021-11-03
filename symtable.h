#pragma once

// Includes
#include "scanner.h"
#include "debug.h"
#include "scanner.h"
#include "errors.h"
#include "dynstring.h"
#include "stack.h"

typedef sstack_t tables_t;


typedef enum dataytpe {
    TYPE_integer,
    TYPE_number,
    TYPE_string,
    TYPE_function,
    TYPE_boolean,
} id_type_t;


typedef struct symstack symstack_t;
typedef struct symtable symtable_t;

typedef struct symbol {
    id_type_t type;
    dynstring_t *id;
} symbol_t;


extern const struct symstack_interface_t Symstack;

struct symstack_interface_t {
    void *(*init)();

    bool (*push)(symtable_t *, dynstring_t *, symbol_t *);

    void (*pop)(symtable_t *, dynstring_t *, id_type_t);

    void (*dtor)(void *);

    // symtable functions
    bool (*get)(symtable_t *, dynstring_t *, symbol_t *);

    void (*put)(symtable_t *, dynstring_t *, id_type_t);
};


extern const struct symtable_interface_t Symtable;

struct symtable_interface_t {
    void *(*ctor)();

    bool (*get)(symtable_t *, dynstring_t *, symbol_t *);

    void (*put)(symtable_t *, dynstring_t *, id_type_t);

    void (*dtor)(void *);
};
