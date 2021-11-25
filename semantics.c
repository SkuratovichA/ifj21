/**
 * @file semantics.c
 *
 * @brief Function semantics structure and functions.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */

#include "semantics.h"
#include "dynstring.h"
#include "expressions.h"
#include "parser.h"

/** Function checks if return values and parameters
 *  of the function are equal.
 *
 * @param func function definition or declaration semantics.
 * @return bool.
 */
static bool Check_signatures(func_semantics_t *func) {
    bool res;
    res = Dynstring.cmp(func->declaration.params, func->definition.params) == 0;
    res &= (Dynstring.cmp(func->declaration.returns, func->definition.returns) == 0);

    debug_msg("\n\t[semantics] function signatures are %s\n", res ? "same" : "different");
    return res;
}

/** A predicate.
 *
 * @param self an atom.
 * @return the truth.
 */
static bool Is_declared(func_semantics_t *self) {
    return self->is_declared;
}

/** A predicate.
 *
 * @param self an atom.
 * @return the truth.
 */
static bool Is_defined(func_semantics_t *self) {
    return self->is_defined;
}

/** A predicate.
 *
 * @param self an atom.
 * @return the truth.
 */
static bool Is_builtin(func_semantics_t *self) {
    return self->is_builtin;
}

/** Set is_declared.
 *
 * TODO: depricate?
 *
 * @param self semantics to change it_declared flag.
 * @return void.
 */
static void Declare(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    self->is_declared = true;
}

/** Set is_defined.
 *
 * TODO: depricate?
 *
 * @param self semantics to change it_defined flag.
 * @return void.
 */
static void Define(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    self->is_defined = true;
}

/** Set is_builtin.
 *
 * TODO: depricate?
 *
 * @param self semantics to change it_builtin flag.
 * @return void.
 */
static void Builtin(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    self->is_builtin = true;
}

/** Convert an id_type to a character for vector representation of types.
 *
 * @param type id_type to convert.
 * @return a converted char.
 */
static char of_id_type(int type) {
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

/** Add a return type to a function semantics.
 *
 * @param self info to add a param.
 * @param type param to add.
 */
static void Add_return(func_info_t *self, int type) {
    Dynstring.append(self->returns, of_id_type(type));
    debug_msg("\n\t[semantics] add return\n");
}

/** Add a function parameter to a function semantics.
 *
 * @param self info to add a param.
 * @param type param to add.
 */
static void Add_param(func_info_t *self, int type) {
    Dynstring.append(self->params, of_id_type(type));
    debug_msg("\n\t[semantics] add return\n");
}

/** Add a function parameter to a function semantics.
 *
 * @param self info to set a vector.
 * @param vec vector to add.
 */
static void Set_returns(func_info_t *self, dynstring_t *vec) {
    debug_msg("\n\t[semantics] Set params: %s\n", Dynstring.c_str(vec));
    self->returns = vec;
}

/** Directly set a vector with function parameters.
 *
 * @param self info to set a vector.
 * @param vec vector to add.
 */
static void Set_params(func_info_t *self, dynstring_t *vec) {
    debug_msg("\n\t[semantics] Set returns: %s\n", Dynstring.c_str(vec));
    self->params = vec;
}

/** Function semantics destructor.
 *
 * @param self a victim.
 */
static void Dtor(func_semantics_t *self) {
    if (self == NULL) {
        return;
    }
    Dynstring.dtor(self->definition.returns);
    Dynstring.dtor(self->declaration.returns);

    Dynstring.dtor(self->definition.params);
    Dynstring.dtor(self->declaration.params);
    free(self);
    debug_msg("\n\t[dtor] delete function semantic\n");
}

/** Function semantics constructor.
 *
 * @param is_defined flag to set.
 * @param is_declared flag to set.
 * @param is_builtin flag to set.
 * @return new function semantics.
 */
static func_semantics_t *Ctor(bool is_defined, bool is_declared, bool is_builtin) {
    debug_msg_s("\n");
    func_semantics_t *newbe = calloc(1, sizeof(func_semantics_t));
    soft_assert(newbe != NULL, ERROR_INTERNAL);

    if (is_defined) { Define(newbe); }
    if (is_declared) { Declare(newbe); }

    // set params and returns manually if there's a builtin function.
    if (is_builtin) {
        debug_msg_s("\tIn case of builtin function, there's need to set parameters manually\n");
        Builtin(newbe);
    } else {
        newbe->declaration.returns = Dynstring.ctor("");
        soft_assert(newbe->declaration.returns != NULL, ERROR_INTERNAL);
        newbe->declaration.params = Dynstring.ctor("");
        soft_assert(newbe->declaration.params != NULL, ERROR_INTERNAL);

        newbe->definition.returns = Dynstring.ctor("");
        soft_assert(newbe->definition.returns != NULL, ERROR_INTERNAL);
        newbe->definition.params = Dynstring.ctor("");
        soft_assert(newbe->definition.params != NULL, ERROR_INTERNAL);
    }

    return newbe;
}

/** Expression semantics constructor.
 *
 * @return new expressions semantics.
 */
static expr_semantics_t *Ctor_expr() {
    expr_semantics_t *expr_sem = calloc(1, sizeof(expr_semantics_t));
    soft_assert(expr_sem != NULL, ERROR_INTERNAL);

    expr_sem->sem_state = SEMANTIC_IDLE;
    expr_sem->op = OP_UNDEFINED;
    expr_sem->conv_type = NO_CONVERSION;
    expr_sem->result_type = ID_TYPE_UNDEF;

    return expr_sem;
}

/** Expression semantics destructor.
 *
 * @param self expression semantics struct.
 */
static void Dtor_expr(expr_semantics_t *self) {
    if (self == NULL) {
        return;
    }

    free(self);
}

/** Expression semantics add operand.
 *
 * @param self expression semantics struct.
 * @param tok operand.
 */
static void Add_operand(expr_semantics_t *self, token_t tok) {
    if (self->sem_state == SEMANTIC_DISABLED ||
        self->sem_state == SEMANTIC_UNARY ||
        self->sem_state == SEMANTIC_BINARY) {
        return;
    }

    if (self->op == OP_UNDEFINED) {
        self->first_operand = tok;
        self->sem_state = SEMANTIC_OPERAND;
    } else {
        if (self->sem_state == SEMANTIC_IDLE) {
            self->first_operand = tok;
            self->sem_state = SEMANTIC_UNARY;
        } else {
            self->second_operand = tok;
            self->sem_state = SEMANTIC_BINARY;
        }
    }
}

static void Add_operator(expr_semantics_t *self, op_list_t op) {
    if (self->sem_state == SEMANTIC_DISABLED ||
        self->sem_state == SEMANTIC_UNARY ||
        self->sem_state == SEMANTIC_BINARY) {
        return;
    }

    self->op = op;
}

static int type_to_token (id_type_t id_type) {
    switch (id_type) {
        case ID_TYPE_string:
            return TOKEN_STR;
        case ID_TYPE_integer:
            return TOKEN_NUM_I;
        case ID_TYPE_number:
            return TOKEN_NUM_F;
        case ID_TYPE_boolean:
            return KEYWORD_boolean;
        case ID_TYPE_nil:
            return KEYWORD_nil;
        default:
            return TOKEN_DEAD;
    }
}

static int token_to_type(int typ) {
    switch (typ) {
        case TOKEN_STR:
            return ID_TYPE_string;
        case TOKEN_NUM_I:
            return ID_TYPE_integer;
        case TOKEN_NUM_F:
            return ID_TYPE_number;
        case KEYWORD_boolean:
            return ID_TYPE_boolean;
        case KEYWORD_nil:
            return ID_TYPE_nil;
        default:
            return ID_TYPE_UNDEF;
    }
}

static bool is_var_exists (token_t tok, expr_semantics_t *self) {
    symbol_t *symbol;
    bool res;
    res = Symstack.get_local_symbol(symstack, tok.attribute.id, &symbol);
    if (res) {
        self->result_type = type_to_token(symbol->type);
        return true;
    } else {
        Errors.set_error(ERROR_DEFINITION);
        return false;
    }
}

static bool type_compatability(expr_semantics_t *self) {
    if (self->sem_state == SEMANTIC_UNARY) {
        id_type_t f_var_type = token_to_type(self->first_operand.type);

        // String length
        if (self->op == OP_HASH && f_var_type == ID_TYPE_string) {
            self->result_type = TOKEN_NUM_I;
            return true;
        }

        // not
        else if (self->op == OP_NOT && f_var_type == ID_TYPE_boolean) {
            self->result_type = KEYWORD_boolean;
            return true;
        }
    } else {
        id_type_t f_var_type = token_to_type(self->first_operand.type);
        id_type_t s_var_type = token_to_type(self->second_operand.type);

        // Addition, subtraction, multiplication, float division
        if (self->op == OP_ADD || self->op == OP_SUB || self->op == OP_MUL || self->op == OP_DIV_F) {
            self->result_type = TOKEN_NUM_F;

            if (f_var_type == ID_TYPE_number && s_var_type == ID_TYPE_number) {
                return true;
            } else if (f_var_type == ID_TYPE_integer && s_var_type == ID_TYPE_integer) {
                self->result_type = TOKEN_NUM_I;
                if (self->op == OP_DIV_F) {
                    self->result_type = TOKEN_NUM_F;
                    self->conv_type = CONVERT_BOTH;
                }
                return true;
            } else if (f_var_type == ID_TYPE_integer && s_var_type == ID_TYPE_number) {
                self->conv_type = CONVERT_FIRST;
                return true;
            } else if (f_var_type == ID_TYPE_number && s_var_type == ID_TYPE_integer) {
                self->conv_type = CONVERT_SECOND;
                return true;
            }
        }

        // Integer division
        else if (self->op == OP_DIV_I) {
            self->result_type = TOKEN_NUM_I;

            if (f_var_type == ID_TYPE_integer && s_var_type == ID_TYPE_integer) {
                return true;
            }
        }

        // String concatenation
        else if (self->op == OP_STRCAT) {
            self->result_type = TOKEN_STR;

            if (f_var_type == ID_TYPE_string && s_var_type == ID_TYPE_string) {
                return true;
            }
        }

        // Relation operators
        else if (self->op == OP_LT || self->op == OP_LE ||
                 self->op == OP_GT || self->op == OP_GE ||
                 self->op == OP_EQ || self->op == OP_NE) {
            self->result_type = KEYWORD_boolean;

            if ((f_var_type == ID_TYPE_number && s_var_type == ID_TYPE_number) ||
                (f_var_type == ID_TYPE_string && s_var_type == ID_TYPE_string) ||
                (f_var_type == ID_TYPE_integer && s_var_type == ID_TYPE_integer)) {
                return true;
            } else if (f_var_type == ID_TYPE_integer && s_var_type == ID_TYPE_number) {
                self->conv_type = CONVERT_FIRST;
                return true;
            } else if (f_var_type == ID_TYPE_number && s_var_type == ID_TYPE_integer) {
                self->conv_type = CONVERT_SECOND;
                return true;
            }

            // nil
            if (self->op == OP_EQ || self->op == OP_NE) {
                if ((f_var_type == ID_TYPE_boolean && s_var_type == ID_TYPE_boolean) ||
                    (f_var_type == ID_TYPE_nil || s_var_type == ID_TYPE_nil)) {
                    return true;
                }
            }
        }

        // and, or
        else if (self->op == OP_AND || self->op == OP_OR) {
            self->result_type = KEYWORD_boolean;

            if (f_var_type == ID_TYPE_boolean && s_var_type == ID_TYPE_boolean) {
               return true;
            }
        }
    }

    Errors.set_error(ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY);
    return false;
}

static bool Check_expression(expr_semantics_t *self) {
    if (self->sem_state == SEMANTIC_DISABLED || self->sem_state == SEMANTIC_IDLE) {
        return true;
    }

    // Check if variable is exists in symtable
    if (self->sem_state == SEMANTIC_OPERAND) {
        if (self->first_operand.type == TOKEN_ID) {
            return is_var_exists(self->first_operand, self);
        }

        // true, false -> bool
        int tok_type = self->first_operand.type;
        if (tok_type == KEYWORD_1 || tok_type == KEYWORD_0) {
            tok_type = KEYWORD_boolean;
        }

        self->result_type = tok_type;
        return true;
    }

    // Check type compatability
    return type_compatability(self);
}

static bool Check_signatures_compatibility(dynstring_t *signature_expected,
                                           dynstring_t *return_received,
                                           int error) {
    debug_msg("\n");
    if (signature_expected == NULL || return_received == NULL) {
        debug_msg_s("\t%s is NULL!\n", signature_expected == NULL ? "expected signature" : "received signature");
        Errors.set_error(ERROR_INTERNAL);
        return false;
    }
    // return more than we can.
    if (Dynstring.len(return_received) > Dynstring.len(signature_expected)) {
        debug_msg_s("\t#received_signature(%zu) > #expected_signature(%zu)\n",
                    Dynstring.len(return_received), Dynstring.len(signature_expected));
        Errors.set_error(error);
        return false;
    }

    // truncate to the length of the shortest string.
    if (Dynstring.len(return_received) < Dynstring.len(signature_expected)) {
        Dynstring.trunc_to_len(signature_expected, Dynstring.len(return_received));
    }

    // check datatypes.
    // take in mind:
    //               1. number is a superset of integer.
    //               2. nil in signature_expected -> nil in return_received.
    //               3. everything in signature_expected -> nil in return_received. received[i] = 'n'
    char *expected = Dynstring.c_str(signature_expected);
    char *received = Dynstring.c_str(return_received);
    for (size_t i = 0; i < Dynstring.len(signature_expected); i++) {
        if (expected[i] != received[i]) {
            bool err = !((expected[i] == 'f' && received[i] == 'i') || received[i] == 'n');
            if (err) {
                debug_msg_s("\tmismatched signatures: exp(%s) x res(%s)\n",
                            Dynstring.c_str(signature_expected), Dynstring.c_str(return_received));
                Errors.set_error(error);
                return false;
            }
        }
    }

    return true;
}

const struct semantics_interface_t Semantics = {
        .ctor_expr = Ctor_expr,
        .ctor = Ctor,
        .dtor_expr = Dtor_expr,
        .dtor = Dtor,
        .add_operand = Add_operand,
        .add_operator = Add_operator,
        .is_declared = Is_declared,
        .is_defined = Is_defined,
        .is_builtin = Is_builtin,
        .add_return = Add_return,
        .add_param = Add_param,
        .declare = Declare,
        .define = Define,
        .builtin = Builtin,
        .set_returns = Set_returns,
        .set_params = Set_params,

        .check_signatures = Check_signatures,
        .check_expression = Check_expression,
        .check_signatures_compatibility = Check_signatures_compatibility,
        .of_id_type = of_id_type,
        .token_to_id_type = token_to_type,
};
