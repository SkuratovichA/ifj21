
#include "semantics.h"


typedef struct func_info {
    list_t *returns; //< list with enum(int) values representing types of return values.
    list_t *params; //< list wish enum(int) values representing types of function arguments.
} func_info_t;

typedef struct func_semantics {
    func_info_t declaration; //< function info from the function declaration.
    func_info_t definition; //< function info from the function definition.
    bool is_declared;
    bool is_defined;
    bool is_builtin;
} func_semantics_t;

static int int_equal(void *a, void *b) {
    return (int) *((int *) a) == (int) *((int *) b);
}

static bool Function_signature_equality(func_semantics_t *func) {
    return
            List.equal(func->declaration.params, func->definition.params, int_equal) &&
            List.equal(func->declaration.returns, func->definition.returns, int_equal);
}

static bool Is_declared(func_semantics_t *self) {
    return self->is_declared;
}

static bool Is_defined(func_semantics_t *self) {
    return self->is_defined;
}

static bool Is_builtin(func_semantics_t *self) {
    return self->is_builtin;
}

/**
 * @brief Add an argument datatype.
 * @param info either declaration or definition info.
 * @param type datatype
 */
static void Add_param(func_info_t *info, int type) {
    int *list_item = calloc(1, sizeof(int));
    soft_assert(list_item != NULL, ERROR_INTERNAL);
    *list_item = type;
    List.prepend(info->params, list_item);
}

/**
 * @brief Add a return type.
 * @param info either declaration or definition info.
 * @param type datatype
 */
static void Add_return(func_info_t *self, int type) {
    int *list_item = calloc(1, sizeof(int));
    soft_assert(list_item != NULL, ERROR_INTERNAL);
    *list_item = type;
    List.prepend(self->returns, list_item);
}

/**
 * @brief Delete all information about a function.
 * @param self
 */
static void Dtor(func_semantics_t *self) {
    List.dtor(self->definition.returns, free);
    List.dtor(self->declaration.returns, free);
    List.dtor(self->definition.params, free);
    List.dtor(self->declaration.params, free);
    free(self);
}

static func_semantics_t *Ctor() {
    func_semantics_t *newbe = calloc(1, sizeof(func_semantics_t));
    soft_assert(newbe != NULL, ERROR_INTERNAL);

    newbe->declaration.returns = List.ctor();
    soft_assert(newbe->declaration.returns != NULL, ERROR_INTERNAL);
    newbe->declaration.params = List.ctor();
    soft_assert(newbe->declaration.params != NULL, ERROR_INTERNAL);
    newbe->definition.returns = List.ctor();
    soft_assert(newbe->definition.returns != NULL, ERROR_INTERNAL);
    newbe->definition.params = List.ctor();
    soft_assert(newbe->definition.params != NULL, ERROR_INTERNAL);

    return newbe;
}

const struct semantics_interface_t Semantics = {
        .dtor = Dtor,
        .ctor = Ctor,
        .is_declared = Is_declared,
        .is_defined = Is_defined,
        .is_builtin = Is_builtin,
        .add_return = Add_return,
        .add_param = Add_param,
        .signature_matched = Function_signature_equality,
};
