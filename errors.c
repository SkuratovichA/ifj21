//
// Created by xskura01 on 23.09.2021.
//

#include "errors.h"
#include <stdio.h>

static int Return_error(int errcode) {
    char *errmsg = "Undefined error code!";
    switch (errcode) {
        case ERROR_LEXICAL:
            // 1 - lexical error
            errmsg = "Lexical error";
            break;
        case ERROR_SYNTAX:
            // 2 - syntactic error
            errmsg = "Syntax error";
            break;
        case ERROR_UNDEF_FUN_OR_VAR:
            // todo: does it want to add more error messages? i.e undefined function, undefined variable ...
        // case ERROR_UNDEFINED_FUNCTION:
            // 3 - semantic error in the program - undefined function/variable, attempt of redefining the variable,. ..
            errmsg = "Undefined function/variable!";
            break;
        case ERROR_TYPE_MISSMATCH:
            // 4 - type missmatch in assignment expression
            errmsg = "Type missmatch!";
            break;
        case ERROR_SEMANTICS_NUMBER_PARAMETERS:
            // 5 - semantic error - wrong number/type of parameters(function calling/return) or return values
            errmsg = "Wrong number of parameters/return values!";
            break;
        case ERROR_SEMANTICS_TYPE_INCOMPATABLE:
            // 6 - semantic error of type compatibility in arithmetic, string and relation operators.
            errmsg = "Type incompatibility error!";
            break;
        case ERROR_SEMANTICS_OTHER:
            // 7 - other semantic errors.
            errmsg = "Semantic error!";
            break;
        case ERROR_RUNTIME_NIL:
            // 8 - runtime error while working with nil constant.
            errmsg = "nil assignment error!";
            break;
        case ERROR_RUNTIME_DIV_BY_ZERO:
            // 9 - runtime error of division by 0.
            errmsg = "Division by zero error";
            break;
        case ERROR_INTERNAL:
            // 99 - internal compiler error - not affected by program input (memory management error, ...).
            errmsg = "Internal error";
            break;
        default:
            break;
    }
    fprintf(stderr, "ERROR: %s\n", errmsg);
    return errcode;
}

const struct error Errors= {
        .return_error = Return_error
};
