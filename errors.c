
#include "errors.h"
#include <stdio.h>

static int error;

static int Get_error() {
    return error;
}

/**
 * @brief Print error_interface message to stderr.
 *
 * @param errcode Errors codes are defined in errors.h
 * @return return errorcode number that is specified in macros.h
 *
 */
static void Set_error(int errcode) {

    char *errmsg = "BLASPHEMOUS RUMORS!";
    switch (errcode) {
        case ERROR_LEXICAL:
            // 1 - lexical error_interface
            errmsg = "Lexical error_interface";
            break;
        case ERROR_SYNTAX:
            // 2 - syntactic error_interface
            errmsg = "Syntax error_interface";
            break;
        case ERROR_UNDEF_FUN_OR_VAR:
            // todo: does it want to add more error_interface messages? i.e undefined function, undefined variable ...
            // case ERROR_UNDEFINED_FUNCTION:
            // 3 - semantic error_interface in the program - undefined function/variable, attempt of redefining the variable,. ..
            errmsg = "Undefined function/variable!";
            break;
        case ERROR_TYPE_MISSMATCH:
            // 4 - type missmatch in assignment expression
            errmsg = "Type missmatch!";
            break;
        case ERROR_SEMANTICS_NUMBER_PARAMETERS:
            // 5 - semantic error_interface - wrong number/type of parameters(function calling/return) or return values
            errmsg = "Wrong number of parameters/return values!";
            break;
        case ERROR_SEMANTICS_TYPE_INCOMPATABLE:
            // 6 - semantic error_interface of type compatibility in arithmetic, string and relation operators.
            errmsg = "Type incompatibility error_interface!";
            break;
        case ERROR_SEMANTICS_OTHER:
            // 7 - other semantic errors.
            errmsg = "Semantic error_interface!";
            break;
        case ERROR_RUNTIME_NIL:
            // 8 - runtime error_interface while working with nil constant.
            errmsg = "nil assignment error_interface!";
            break;
        case ERROR_RUNTIME_DIV_BY_ZERO:
            // 9 - runtime error_interface of division by 0.
            errmsg = "Division by zero error_interface";
            break;
        case ERROR_INTERNAL:
            // 99 - internal compiler error_interface - not affected by program input (memory management error_interface, ...).
            errmsg = "Internal error_interface";
            break;
        default:
            break;
    }
    fprintf(stderr, "ERROR: %s\n", errmsg);

    error = errcode;
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct error_interface Errors = {
        .set_error = Set_error,
        .get_error = Get_error,
};
