#pragma once

#include "list.h"
#include "stdbool.h"
#include "debug.h"


typedef struct func_semantics func_semantics_t;
typedef struct func_info func_info_t;

extern const struct semantics_interface_t Semantics;

struct semantics_interface_t {
    void (*dtor)(func_semantics_t *);

    func_semantics_t *(*ctor)();

    bool (*is_declared)(func_semantics_t *);

    bool (*is_defined)(func_semantics_t *);

    bool (*is_builtin)(func_semantics_t *);

    bool (*signature_matched)(func_semantics_t *);

    void (*add_return)(func_info_t *, int);

    void (*add_param)(func_info_t *, int);
};
