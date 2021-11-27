/**
 * @file expressions.c
 *
 * @brief Parser of expressions.
 *
 * @author Evgeny Torbin <xtorbi00@vutbr.cz>
 */

#include "progfile.h"
#include "dynstring.h"
#include "expressions.h"
#include "parser.h"
#include "symtable.h"
#include "stack.h"
#include "code_generator.h"

static pfile_t *pfile;

/** Function call other expressions.
 *
 * !rule <fc_other_expr> -> , expr <fc_other_expr> | )
 *
 * @param params_cnt counter of function parameters.
 * @return bool.
 */
static bool fc_other_expr(int params_cnt) {
    debug_msg("<fc_other_expr> ->\n");

    // | )
    EXPECTED_OPT(TOKEN_RPAREN);
    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    // PARSE EXPRESSION

    params_cnt++;

    // <fc_other_expr>
    if (!fc_other_expr(params_cnt)) {
        goto err;
    }

    noerr:
    return true;
    err:
    return false;
}

/** Function call expression.
 *
 * !rule <fc_expr> -> expr <fc_other_expr> | )
 *
 * @return bool.
 */
static bool fc_expr() {
    debug_msg("<fc_expr> ->\n");

    int params_cnt = 0;

    // | )
    EXPECTED_OPT(TOKEN_RPAREN);

    // expr
    // PARSE EXPRESSION

    params_cnt++;

    // <fc_other_expr>
    if (!fc_other_expr(params_cnt)) {
        goto err;
    }

    noerr:
    return true;
    err:
    return false;
}

/** Function call.
 *
 * !rule <func_call> -> ( <fc_expr>
 *
 * @param id_name function identifier name.
 * @return bool.
 */
static bool func_call(dynstring_t *id_name) {
    debug_msg("<func_call> ->\n");

    // FUNCTION CALL START

    // (
    EXPECTED(TOKEN_LPAREN);

    // <fc_expr>
    if (!fc_expr()) {
        goto err;
    }

    // FUNCTION CALL END

    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** Return other expressions.
 *
 * !rule <r_other_expr> -> , expr <r_other_expr> | e
 *
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool r_other_expr(dynstring_t *received_signature) {
    debug_msg("<r_other_expr> ->\n");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    // PARSE EXPRESSION

    // <r_other_expr>
    if (!r_other_expr(received_signature)) {
        goto err;
    }

    return true;
    err:
    return false;
}

/**
 * Return expression.
 *
 * !rule <r_expr> -> expr <r_other_expr> | e
 *
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool r_expr(dynstring_t *received_signature) {
    debug_msg("<r_expr> ->\n");

    // expr
    // PARSE EXPRESSION

    // TODO: | e (check received signature on empty)

    // <r_other_expr>
    if (!r_other_expr(received_signature)) {
        goto err;
    }

    return true;
    err:
    return false;
}

/** Assignment other expressions.
 *
 * !rule <a_other_expr> -> , expr <a_other_expr> | e
 *
 * @param ids_list list of identifiers.
 * @return bool.
 */
static bool a_other_expr(list_t *ids_list) {
    debug_msg("<a_other_expr> ->\n");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    // PARSE EXPRESSION

    // <a_other_expr>
    if (!a_other_expr(ids_list)) {
        goto err;
    }

    return true;
    err:
    return false;
}

/** Assignment expression.
 *
 * !rule <a_expr> -> expr <a_other_expr>
 *
 * @param ids_list list of identifiers.
 * @return bool.
 */
static bool a_expr(list_t *ids_list) {
    debug_msg("<a_expr> ->\n");

    // expr
    // PARSE EXPRESSION

    // <a_other_expr>
    if (!a_other_expr(ids_list)) {
        goto err;
    }

    return true;
    err:
    return false;
}

/** Assignment other identifiers.
 *
 * !rule <a_other_id> -> , id <a_other_id> | = <a_expr>
 *
 * @param ids_list list of identifiers.
 * @return bool.
 */
static bool a_other_id(list_t *ids_list) {
    debug_msg("<a_other_id> ->\n");

    dynstring_t *id_name = NULL;

    // | = <a_expr>
    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        if (!a_expr(ids_list)) {
            goto err;
        } else {
            goto noerr;
        }
    }

    // ,
    EXPECTED(TOKEN_COMMA);
    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);

    // Append next identifier
    List.append(ids_list, id_name);

    // <a_other_id>
    if (!a_other_id(ids_list)) {
        goto err;
    }

    noerr:
    return true;
    err:
    return false;
}

/** Assignment identifier.
 *
 * !rule <assign_id> -> <a_other_id>
 *
 * @param id_name identifier name.
 * @return bool.
 */
static bool assign_id(dynstring_t *id_name) {
    debug_msg("<assign_id> ->\n");

    // Create a list of identifiers and append first
    list_t *ids_list = List.ctor();
    List.append(ids_list, id_name);

    // <a_other_id>
    if (!a_other_id(ids_list)) {
        goto err;
    }

    List.dtor(ids_list, (void (*)(void *)) Dynstring.dtor);
    return true;
    err:
    List.dtor(ids_list, (void (*)(void *)) Dynstring.dtor);
    return false;
}

/**
 * @brief Expression in the return statement.
 *        semantic control of <return signature> x <its function signature> are performed in parser.c
 *
 * @param pfile_
 * @param received_signature is an initialized empty vector.
 * @return true if successive parsing performed.
 */
static bool Return_expressions(pfile_t *pfile_, dynstring_t *received_signature) {
    debug_msg("Return_expression\n");

    pfile = pfile_;

    if (!r_expr(received_signature)) {
        return false;
    }

    return true;
}

/**
 * @brief Default expression after = in the local assignment
 *        Or an expression in the conditional statement in cycles, conditions.
 *        semantic controls are performed inside parser.c
 *
 * @param received_signature is an initialized empty vector.
 * @return true if successive parsing performed.
 */
static bool Default_expression(pfile_t *pfile_, dynstring_t *received_signature) {
    debug_msg("Default_expression\n");

    pfile = pfile_;

    // PARSE EXPRESSION

    return true;
}

/**
 * @brief Function calling in the global scope. `id( ...`
 *
 * @param pfile_
 * @return true if successive parsing and semantic analysis of expressions performed.
 */
static bool Global_expression(pfile_t *pfile_) {
    debug_msg("Global_expression\n");

    pfile = pfile_;
    dynstring_t *id_name = NULL;

    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);

    // CHECK IF ID IS A FUNCTION

    if (!func_call(id_name)) {
        goto err;
    }

    return true;
    err:
    return false;
}

/**
 * @brief Function calling or assignments in the local scope.
 *
 * @param pfile_
 * @return true if successive parsing and semantic analysis of expressions performed.
 */
static bool Function_expression(pfile_t *pfile_) {
    debug_msg("Function_expression\n");

    pfile = pfile_;
    dynstring_t *id_name = NULL;

    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);

    // CHECK IF ID IS A FUNCTION

    // IF ID IS A FUNCTION
//    if (!func_call(id_name)) {
//        goto err;
//    }

    // IF ID IS NOT A FUNCTION
//    if (!assign_id(id_name)) {
//        goto err;
//    }

    return true;
    err:
    return false;
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .function_expression = Function_expression,
        .global_expression = Global_expression,
        .default_expression = Default_expression,
        .return_expressions = Return_expressions,
};
