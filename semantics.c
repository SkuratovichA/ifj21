
#include "semantics.h"
#include "dynstring.h"
#include "symtable.h"


static bool Check_signatures(func_semantics_t *func) {
    bool res;
    res = Dynstring.cmp(func->declaration.params, func->definition.params) == 0;
    res &= (Dynstring.cmp(func->declaration.returns, func->definition.returns) == 0);

    debug_msg("[semantics] function signatures are %s\n", res ? "same" : "different");
    return res;
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

static void Declare(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    self->is_declared = true;
}

static void Define(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    self->is_defined = true;
}

static void Builtin(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    self->is_builtin = true;
}

static char of_id_type(id_type_t type) {
    switch (type) {
        case ID_TYPE_string:
            return 's';
        case ID_TYPE_boolean:
            return 'b';
        case ID_TYPE_integer:
            return 'i';
        case ID_TYPE_number:
            return 'f';
        case ID_TYPE_nil:
            return 'n';

        default:
            //ID_TYPE_func_def
            //ID_TYPE_func_decl
            //ID_TYPE_UNDEF
            return 'u';
    }
}

/**
 * @brief Add a return type.
 * @param info either declaration or definition info.
 * @param type datatype
 */
static void Add_return(func_info_t self, int type) {
    Dynstring.append(self.returns, of_id_type(type));
    debug_msg("[semantics] add return\n");
}

/**
 * @brief Add an argument datatype.
 * @param info either declaration or definition info.
 * @param type datatype
 */
static void Add_param(func_info_t self, int type) {
    Dynstring.append(self.params, of_id_type(type));
    debug_msg("[semantics] add return\n");
}

static void Set_returns(func_info_t self, dynstring_t *vec) {
    debug_msg("[semantics] Set params: %s\n", Dynstring.c_str(vec));
    self.returns = vec;
}

static void Set_params(func_info_t self, dynstring_t *vec) {
    debug_msg("[semantics] Set returns: %s\n", Dynstring.c_str(vec));
    self.params = vec;
}

/**
 * @brief Delete all information about a function.
 * @param self
 */
static void Dtor(func_semantics_t *self) {
    Dynstring.dtor(self->definition.returns);
    Dynstring.dtor(self->declaration.returns);
    Dynstring.dtor(self->definition.params);
    Dynstring.dtor(self->declaration.params);
    free(self);
    debug_msg("[dtor] delete function semantic\n");
}

static func_semantics_t *Ctor(bool is_defined, bool is_declared, bool is_builtin) {
    func_semantics_t *newbe = calloc(1, sizeof(func_semantics_t));
    soft_assert(newbe != NULL, ERROR_INTERNAL);

    if (is_defined) { Define(newbe); }
    if (is_declared) { Declare(newbe); }
    if (is_builtin) { Builtin(newbe); }

    newbe->declaration.returns = Dynstring.ctor("");
    soft_assert(newbe->declaration.returns != NULL, ERROR_INTERNAL);
    newbe->declaration.params = Dynstring.ctor("");
    soft_assert(newbe->declaration.params != NULL, ERROR_INTERNAL);

    newbe->definition.returns = Dynstring.ctor("");
    soft_assert(newbe->definition.returns != NULL, ERROR_INTERNAL);
    newbe->definition.params = Dynstring.ctor("");
    soft_assert(newbe->definition.params != NULL, ERROR_INTERNAL);

    //debug_msg("[ctor] Create a function semantics:\n"
    //          "\t{ "
    //          "\t\t\t.is_defined = '%s', .is_declared = '%s', is_builtin = '%s'\n"
    //          "\t\t\tdefinition{ .params ='%s', returns ='%s'}\n"
    //          "\t\t\tdeclaration{ .params ='%s', returns ='%s'}\n"
    //          "\t}\n",
    //          newbe->is_defined ? "true" : "false",
    //          newbe->is_declared ? "true" : "false",
    //          newbe->is_builtin ? "true" : "false",

    //          Dynstring.c_str(newbe->definition.params),
    //          Dynstring.c_str(newbe->definition.returns),

    //          Dynstring.c_str(newbe->declaration.params),
    //          Dynstring.c_str(newbe->declaration.returns)
    //          );
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
        .check_signatures = Check_signatures,
        .declare = Declare,
        .define = Define,
        .builtin = Builtin,
        .set_returns = Set_returns,
        .set_params = Set_params,
};
