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
    TYPE_integer = KEYWORD_integer,
    TYPE_number = KEYWORD_number,
    TYPE_string = KEYWORD_string,
    TYPE_boolean = KEYWORD_boolean,
    TYPE_func_def,
    TYPE_func_decl,
} id_type_t;


typedef struct symstack symstack_t;
typedef struct symtable symtable_t;

typedef struct symbol {
    id_type_t type;
    dynstring_t *id;
} symbol_t;


extern const struct symstack_interface_t Symstack;

struct symstack_interface_t {
    // init once.
    void *(*init)();

    // caller pushes.
    // Symstack.push(...)
    // some_body(...)
    void (*push)(symstack_t **, symtable_t *);

    // caller pops.
    // some_body(...)
    // Symstack.pop(...)
    void (*pop)(symstack_t **);

    // destruct once.
    void (*dtor)(void **);

    // get an item from symstack through the pointer.
    // true if we find an element.
    bool (*get)(symstack_t *, dynstring_t *, symbol_t *);

    // put symbol in to symtable on the top of the stack.
    void (*put)(symstack_t *, dynstring_t *, id_type_t);
};


extern const struct symtable_interface_t Symtable;

struct symtable_interface_t {
    void *(*ctor)();

    bool (*get)(symtable_t *, dynstring_t *, symbol_t *);

    void (*put)(symtable_t *, dynstring_t *, id_type_t);

    void (*dtor)(void *);
};
