#pragma once

// Includes
#include "scanner.h"
#include "debug.h"
#include "errors.h"
#include "dynstring.h"


typedef enum scope_type {
    _SCOPE_UNDEF_,
    SCOPE_function,
    SCOPE_cycle,
    SCOPE_do_cycle,
    SCOPE_condition,
    SCOPE_global,
} scope_type_t;

typedef enum id_type {
    _TYPE_UNDEF,
    TYPE_string = KEYWORD_string,
    TYPE_boolean = KEYWORD_boolean,
    TYPE_number = KEYWORD_number,
    TYPE_integer = KEYWORD_integer,
    TYPE_func_def,
    TYPE_func_decl,
} id_type_t;

typedef struct symbol {
    id_type_t type;
    dynstring_t *id;
} symbol_t;

typedef struct scope_info {
    scope_type_t scope_type;
    size_t scope_level;
} scope_info_t;

extern const struct symtable_interface_t Symtable;
extern const struct symstack_interface_t Symstack;
typedef struct symstack symstack_t;
typedef struct symtable symtable_t;

struct symstack_interface_t {
    // init once.
    void *(*init)();

    // caller pushes.
    // Symstack.push(...)
    // some_body(...)
    void (*push)(symstack_t *, symtable_t *, scope_type_t);

    // caller pops.
    // some_body(...)
    // Symstack.pop(...)
    void (*pop)(symstack_t *);

    // destruct once.
    void (*dtor)(symstack_t *);

    // get_symbol an item from symstack through the pointer.
    // true if we find an element.
    bool (*get_symbol)(symstack_t *, dynstring_t *, symbol_t *);

    // put symbol in to symtable on the top of the stack.
    void (*put)(symstack_t *, dynstring_t *, id_type_t);

    symtable_t *(*top)(symstack_t *);

    scope_info_t (*get_scope_info)(symstack_t *);
};

struct symtable_interface_t {
    symtable_t *(*ctor)();

    bool (*get)(symtable_t *, dynstring_t *, symbol_t *);

    void (*put)(symtable_t *, dynstring_t *, id_type_t);

    void (*dtor)(symtable_t *);
};
