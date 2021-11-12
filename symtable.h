#pragma once

// Includes
#include "scanner.h"
#include "debug.h"
#include "errors.h"
#include "dynstring.h"
#include "semantics.h"

// types of scope
#define SCOPE_TYPE_T(X) \
    X(function)          \
    X(cycle)             \
    X(do_cycle)          \
    X(condition)         \
    X(global)            \
    X(UNDEF)         \

#define ID_TYPE_T(X)   \
    X(string)          \
    X(boolean)         \
    X(number)          \
    X(integer)         \
    X(func_def)        \
    X(func_decl)       \
    X(nil)             \
    X(UNDEF)       \

typedef enum scope_type {
    #define X(a) SCOPE_TYPE_##a ,
    SCOPE_TYPE_T(X)
    #undef X
} scope_type_t;

#define KEYWORD_func_def (KEYWORD_UNDEF + 1)
#define KEYWORD_func_decl (KEYWORD_func_def + 1)

typedef enum id_type {
    #define X(a) ID_TYPE_##a = KEYWORD_##a,
    ID_TYPE_T(X)
    #undef X
} id_type_t;


typedef struct symbol {
    dynstring_t *id;
    id_type_t type;

    func_semantics_t *function_semantics;
} symbol_t;

typedef struct scope_info {
    scope_type_t scope_type;
    size_t scope_level;
    size_t unique_id;
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
    bool (*get_symbol)(symstack_t *, dynstring_t *, symbol_t **);

    // put_symbol symbol in to symtable on the top of the stack.
    void (*put_symbol)(symstack_t *, dynstring_t *, id_type_t);

    symtable_t *(*top)(symstack_t *);

    scope_info_t (*get_scope_info)(symstack_t *);
};

struct symtable_interface_t {
    symtable_t *(*ctor)();

    id_type_t (*id_type_of_token_type)(int);

    bool (*get_symbol)(symtable_t *, dynstring_t *, symbol_t **);

    void (*put)(symtable_t *, dynstring_t *, id_type_t);

    void (*add_builtin_function)(symtable_t *, char *, char *, char *);

    void (*dtor)(symtable_t *);

};
