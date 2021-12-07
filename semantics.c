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

static int token_to_type(int typ) {
    switch (typ) {
        case TOKEN_STR:
            return ID_TYPE_string;
        case TOKEN_NUM_I:
            return ID_TYPE_integer;
        case TOKEN_NUM_F:
            return ID_TYPE_number;
        case KEYWORD_1:
        case KEYWORD_0:
            return ID_TYPE_boolean;
        case KEYWORD_nil:
            return ID_TYPE_nil;
        default:
            return ID_TYPE_UNDEF;
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

static bool Check_signatures_compatibility(dynstring_t *signature_expected,
                                           dynstring_t *signature_received,
                                           int error) {
    debug_msg("\n");
    if (signature_expected == NULL || signature_received == NULL) {
        debug_msg_s("\t%s is NULL!\n", signature_expected == NULL ? "expected signature" : "received signature");
        Errors.set_error(ERROR_INTERNAL);
        return false;
    }

    // more than we can.
    if (Dynstring.len(signature_received) > Dynstring.len(signature_expected)) {
        debug_msg_s("\t#received_signature(%zu) > #expected_signature(%zu)\n",
                    Dynstring.len(signature_received), Dynstring.len(signature_expected));
        Errors.set_error(error);
        return false;
    }

    // truncate to the length of the received vector if received is shorted.
    if (Dynstring.len(signature_received) < Dynstring.len(signature_expected)) {
        Dynstring.trunc_to_len(signature_expected, Dynstring.len(signature_received));
    }

    // check datatypes.
    // take in mind:
    //               1. number is a superset of integer.
    //               2. nil in signature_expected -> nil in signature_received.
    //               3. everything in signature_expected -> nil in signature_received. received[i] = 'n'
    char *expected = Dynstring.c_str(signature_expected);
    char *received = Dynstring.c_str(signature_received);
    for (size_t i = 0; i < Dynstring.len(signature_expected); i++) {
        if (expected[i] != received[i]) {
            bool err = !((expected[i] == 'f' && received[i] == 'i') || received[i] == 'n');
            if (err) {
                debug_msg_s("\tmismatched signatures: exp(%s) x res(%s)\n",
                            Dynstring.c_str(signature_expected), Dynstring.c_str(signature_received));
                Errors.set_error(error);
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief Truncate signature to one type.
 *
 * @param type_signature is an initialized vector with expression type/s.
 */
static void Trunc_signature(dynstring_t *type_signature) {
    if (Dynstring.len(type_signature) > 1) {
        Dynstring.trunc_to_len(type_signature, 1);
    }
}

/**
 * @brief Check semantics of binary operation.
 *
 * @param first_type type of the first operand.
 * @param second_type type of the second operand.
 * @param op binary operator.
 * @param expression_type initialized vector to store an expression type.
 * @param r_type variable to store a type of recast.
 * @return bool.
 */
static bool Check_binary_compatibility(dynstring_t *first_type,
                                       dynstring_t *second_type,
                                       op_list_t op,
                                       dynstring_t *expression_type,
                                       type_recast_t *r_type) {
    char result_type;

    switch (op) {
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_EQ:
        case OP_NE:
            result_type = 'b';

            // boolean -> integer <|<=|>|>=|==|~= integer
            // boolean -> number  <|<=|>|>=|==|~= number
            // boolean -> string  <|<=|>|>=|==|~= string
            if (Dynstring.cmp(first_type, second_type) == 0 &&
                Dynstring.cmp_c_str(first_type, "n") != 0 &&
                Dynstring.cmp_c_str(first_type, "b") != 0) {
                goto ret;
            }

            // boolean -> integer <|<=|>|>=|==|~= number
            if (Dynstring.cmp_c_str(first_type, "i") == 0 &&
                Dynstring.cmp_c_str(second_type, "f") == 0) {
                *r_type = TYPE_RECAST_FIRST;
                goto ret;
            }

            // boolean -> number  <|<=|>|>=|==|~= integer
            if (Dynstring.cmp_c_str(first_type, "f") == 0 &&
                Dynstring.cmp_c_str(second_type, "i") == 0) {
                *r_type = TYPE_RECAST_SECOND;
                goto ret;
            }

            if (op == OP_EQ || op == OP_NE) {
                // boolean -> nil == integer|number|string|boolean|nil
                // boolean -> nil ~= integer|number|string|boolean|nil
                if (Dynstring.cmp_c_str(first_type, "n") == 0) {
                    goto ret;
                }

                // boolean -> integer|number|string|boolean == nil
                // boolean -> integer|number|string|boolean ~= nil
                if (Dynstring.cmp_c_str(second_type, "n") == 0) {
                    goto ret;
                }

                // boolean -> boolean == boolean
                // boolean -> boolean ~= boolean
                if (Dynstring.cmp_c_str(first_type, "b") == 0 &&
                    Dynstring.cmp_c_str(second_type, "b") == 0) {
                    goto ret;
                }
            }

            break;

        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV_F:
        case OP_CARET:
            result_type = 'f';

            // check nil
            if (Dynstring.cmp_c_str(first_type, "n") == 0 ||
                Dynstring.cmp_c_str(second_type, "n") == 0) {
                break;
            }

            // check string
            if (Dynstring.cmp_c_str(first_type, "s") == 0 ||
                Dynstring.cmp_c_str(second_type, "s") == 0) {
                break;
            }

            // number -> number +|-|*|/|^ number
            if (Dynstring.cmp(first_type, second_type) == 0) {

                if (Dynstring.cmp_c_str(first_type, "i") == 0 &&
                    Dynstring.cmp_c_str(second_type, "i") == 0) {
                    // integer -> integer +|-|* integer
                    if (op != OP_DIV_F && op != OP_CARET) {
                        result_type = 'i';
                    }
                    // number -> integer /|^ integer
                    else {
                        *r_type = TYPE_RECAST_BOTH;
                    }
                }

                goto ret;
            }

            // number -> integer +|-|*|/|^ number
            if (Dynstring.cmp_c_str(first_type, "i") == 0 &&
                Dynstring.cmp_c_str(second_type, "f") == 0) {
                *r_type = TYPE_RECAST_FIRST;
                goto ret;
            }

            // number -> number +|-|*|/|^ integer
            if (Dynstring.cmp_c_str(first_type, "f") == 0 &&
                Dynstring.cmp_c_str(second_type, "i") == 0) {
                *r_type = TYPE_RECAST_SECOND;
                goto ret;
            }

            break;

        case OP_DIV_I:
        case OP_PERCENT:
            result_type = 'i';

            // integer -> integer //|% integer
            if (Dynstring.cmp_c_str(first_type, "i") == 0 &&
                Dynstring.cmp_c_str(second_type, "i") == 0) {
                goto ret;
            }

            break;

        case OP_STRCAT:
            result_type = 's';

            // string -> string .. string
            if (Dynstring.cmp_c_str(first_type, "s") == 0 &&
                Dynstring.cmp_c_str(second_type, "s") == 0) {
                goto ret;
            }

            break;

        case OP_AND:
        case OP_OR:
            result_type = 'b';

            // boolean -> boolean and|or boolean
            if (Dynstring.cmp_c_str(first_type, "b") == 0 &&
                Dynstring.cmp_c_str(second_type, "b") == 0) {
                goto ret;
            }

            break;

        default:
            break;
    }

    Errors.set_error(ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY);
    return false;

    ret:
    Dynstring.append(expression_type, result_type);
    return true;
}

/**
 * @brief Check semantics of unary operation.
 *
 * @param type type of the operand.
 * @param op unary operator.
 * @param expression_type initialized vector to store an expression type.
 * @return bool.
 */
static bool Check_unary_compatability(dynstring_t *type,
                                      op_list_t op,
                                      dynstring_t *expression_type) {
    char result_type;

    switch (op) {
        // integer -> # string
        case OP_HASH:
            result_type = 'i';

            if (Dynstring.cmp_c_str(type, "s") == 0) {
                goto ret;
            }
            break;

        // boolean -> not boolean
        case OP_NOT:
            result_type = 'b';

            if (Dynstring.cmp_c_str(type, "b") == 0) {
                goto ret;
            }
            break;

        // integer -> - integer
        // number -> - number
        // number -> - integer
        case OP_MINUS_UNARY:
            if (Dynstring.cmp_c_str(type, "i") == 0) {
                result_type = 'i';
                goto ret;
            }

            if (Dynstring.cmp_c_str(type, "f") == 0) {
                result_type = 'f';
                goto ret;
            }
            break;

        // other
        default:
            break;
    }

    Errors.set_error(ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY);
    return false;

    ret:
    Dynstring.append(expression_type, result_type);
    return true;
}

/**
 * @brief Check semantics of single operand.
 *
 * @param operand
 * @param expression_type initialized vector to store an expression type.
 * @return bool.
 */
static bool Check_operand(token_t operand, dynstring_t *expression_type) {
    char result_type;
    symbol_t *sym;

    // Check if operand is identifier
    if (operand.type != TOKEN_ID) {
        result_type = Semantics.of_id_type(Semantics.token_to_id_type(operand.type));
        goto ret;
    }

    // Check if identifier is in symbol table
    if (Symstack.get_local_symbol(symstack, operand.attribute.id, &sym)) {
        result_type = Semantics.of_id_type(sym->type);
        goto ret;
    }

    Errors.set_error(ERROR_DEFINITION);
    return false;

    ret:
    Dynstring.append(expression_type, result_type);
    return true;
}

/**
 * @brief Check semantics of two types.
 *
 * @param expected_type
 * @param received_char
 * @param r_type variable to store a type of recast.
 * @return bool.
 */
static bool Check_type_compatibility (const char expected_type,
                                      const char received_char,
                                      type_recast_t *r_type) {
    if (expected_type == received_char) {
        goto ret;
    }

    if (received_char == 'n') {
        goto ret;
    }

    if (expected_type == 'f' && received_char == 'i') {
        *r_type = TYPE_RECAST_SECOND;
        goto ret;
    }


    return false;
    ret:
    return true;
}

const struct semantics_interface_t Semantics = {
        .ctor = Ctor,
        .dtor = Dtor,
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
        .check_signatures_compatibility = Check_signatures_compatibility,
        .of_id_type = of_id_type,
        .token_to_id_type = token_to_type,

        .check_binary_compatibility = Check_binary_compatibility,
        .check_unary_compatibility = Check_unary_compatability,
        .check_operand = Check_operand,
        .trunc_signature = Trunc_signature,
        .check_type_compatibility = Check_type_compatibility,
};
