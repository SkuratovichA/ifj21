#pragma once

#include "list.h"
#include "stdbool.h"
#include "debug.h"


typedef struct func_info {
    list_t *returns; //< list with enum(int) values representing types of return values.
    list_t *params; //< list wish enum(int) values representing types of function arguments.
} func_info_t;

typedef struct var_semantics {
    int datatype; // can be id_type_t or keyword
} var_semantics_t;

typedef struct func_semantics {
    func_info_t declaration; //< function info from the function declaration.
    func_info_t definition; //< function info from the function definition.
    bool is_declared;
    bool is_defined;
    bool is_builtin;
} func_semantics_t;

typedef struct func_semantics func_semantics_t;
typedef struct func_info func_info_t;

extern const struct semantics_interface_t Semantics;

struct semantics_interface_t {
    void (*dtor)(func_semantics_t *);

    func_semantics_t *(*ctor)(bool, bool, bool);

    bool (*is_declared)(func_semantics_t *);

    bool (*is_defined)(func_semantics_t *);

    bool (*is_builtin)(func_semantics_t *);

    bool (*check_signatures)(func_semantics_t *);

    void (*add_return)(func_info_t, int);

    void (*add_param)(func_info_t, int);

    void (*declare)(func_semantics_t *);

    void (*define)(func_semantics_t *);

    void (*builtin)(func_semantics_t *);
};
