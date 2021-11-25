/**
 * @file errors.c
 *
 * @brief
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#include "errors.h"
#include "debug.h"

static int error = 0;
static char *errmsg = "";

/** Error getter.
 *
 * @return return errorcode defined in error.h, which has been set or 0.
 */
static int Get_error() {
    return error;
}

/** Error setter.
 *
 * @param errcode Errors codes are defined in errors.h.
 */
static void Set_error(int errcode) {
    if (errcode != ERROR_NOERROR) {
        debug_msg("\n#######################################################\n"
                  "################ Error will be set NOW! ###############\n"
                  "#######################################################\n");
    } else {
        errmsg = "";
    }

    switch (errcode) {
        case ERROR_LEXICAL:
            // 1 - lexical error
            errmsg = "Lexical error";
            break;
        case ERROR_SYNTAX:
            // 2 - syntactic error
            errmsg = "Syntax error";
            break;
        case ERROR_DEFINITION:
            // todo: does it want to add more error messages? i.e undefined function, undefined variable ...
            // case ERROR_UNDEFINED_FUNCTION:
            // 3 - semantic error in the program - undefined function/variable, attempt of redefining the variable,. ..
            errmsg = "Undefined function/variable!";
            break;
        case ERROR_TYPE_MISSMATCH:
            // 4 - type missmatch in assignment expression
            errmsg = "Type missmatch!";
            break;
        case ERROR_FUNCTION_SEMANTICS:
            // 5 - semantic error - wrong number/type of parameters(function calling/return) or return values
            errmsg = "Wrong number of parameters/return values!";
            break;
        case ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY:
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
    //fprintf(stderr, "ERROR: %s\n", errmsg);

    if (errcode != ERROR_NOERROR) {
        debug_msg("\n################################################################################\n"
                  "######## ERROR = '%s' #########\n"
                  "######## CODE  = '%d' #########\n"
                  "##################################################################################\n", errmsg,
                  errcode);
        (void) (2 + 2 == 5);
    }

    error = errcode;
}

static char *Get_error_msg() {
    return errmsg;
}

/** Functions are in struct so we can use them in different files.
 */
const struct error_interface Errors = {
        .get_errmsg = Get_error_msg,
        .set_error = Set_error,
        .get_error = Get_error,
};
