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

/**
 * @brief Checks if identifier is a function.
 *
 * @param id_name identifier name.
 * @return bool.
 */
static inline bool check_function(dynstring_t *id_name) {
    symbol_t *sym;
    return Symtable.get_symbol(global_table, id_name, &sym);
}

/**
 * @brief Set token for new created/copied item.
 *
 * @param item
 * @param tok token to set (can be NULL).
 */
static void stack_item_set_token(stack_item_t *item, token_t *tok) {
    if (tok != NULL && (item->type == ITEM_TYPE_TOKEN || item->type == ITEM_TYPE_EXPR)) {
        item->token = *tok;
        if (tok->type == TOKEN_ID) {
            item->token.attribute.id = Dynstring.dup(tok->attribute.id);
        }
    }
}

/**
 * @brief Create stack item.
 *
 * @param type stack item type.
 * @param tok token to set (can be NULL).
 * @return new stack item.
 */
static stack_item_t *stack_item_ctor(item_type_t type, token_t *tok) {
    stack_item_t *new_item = calloc(1, sizeof(stack_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->type = type;
    stack_item_set_token(new_item, tok);

    return new_item;
}

/**
 * @brief Copy stack item.
 *
 * @param item
 * @return new stack item.
 */
static stack_item_t *stack_item_copy(stack_item_t *item) {
    stack_item_t *new_item = calloc(1, sizeof(stack_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->type = item->type;
    stack_item_set_token(new_item, &(item->token));

    return new_item;
}

/**
 * @brief Delete stack item.
 *
 * @param item
 */
static void stack_item_dtor(void *item) {
    stack_item_t *s_item = (stack_item_t *) item;

    if (s_item->type == ITEM_TYPE_EXPR || s_item->type == ITEM_TYPE_TOKEN) {
        if (s_item->token.type == TOKEN_ID) {
            Dynstring.dtor(s_item->token.attribute.id);
        }
    }

    free(s_item);
}

/**
 * @brief Expression parsing.
 *
 * @param stack stack for precedence analyse.
 * @param received_signature is an initialized empty vector.
 * @param is_func_param true if expression is a parameter of function.
 * @param hard_reduce reduce without precedence analyse.
 * @return bool.
 */
static bool parse(sstack_t *stack, dynstring_t *received_signature, bool is_func_param, bool hard_reduce) {
    // Precedence comparison value
    int cmp;

    // CHECK ON FUNCTION CALL

    // Peek item from the stack
    stack_item_t *top = (stack_item_t *) Stack.peek(stack);

    // Pop expression if we have it on the top of the stack
    // POP EXPRESSION

    op_list_t first_op;
    op_list_t second_op;

    // CHECK ON EXPRESSION END
    // CHECK IF SUCCESS

    // PRECEDENCE COMPARISON

    if (!hard_reduce && cmp <= 0) {
        // SHIFT
    } else {
        // REDUCE
    }

    return parse(stack, received_signature, is_func_param, hard_reduce);
}

/**
 * @brief Expression parsing initialization.
 *
 * @param received_signature is an initialized empty vector.
 * @param is_func_param true if expression is a parameter of function.
 * @return bool.
 */
static bool parse_init(dynstring_t *received_signature, bool is_func_param) {
    sstack_t *stack = Stack.ctor();
    bool parse_result;

    // Push $ on stack
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_DOLLAR, NULL));

    // Parse expression
    parse_result = parse(stack, received_signature, is_func_param, false);

    // Delete $ from stack
    Stack.dtor(stack, stack_item_dtor);
    return parse_result;
}

/** Function call other expressions.
 *
 * !rule <fc_other_expr> -> , expr <fc_other_expr> | )
 *
 * @param received_signature is an initialized empty vector.
 * @param params_cnt counter of function parameters.
 * @return bool.
 */
static bool fc_other_expr(dynstring_t *received_signature, int params_cnt) {
    debug_msg("<fc_other_expr> ->\n");

    // | )
    EXPECTED_OPT(TOKEN_RPAREN);
    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    if (!parse_init(received_signature, true)) {
        goto err;
    }

    params_cnt++;

    // <fc_other_expr>
    if (!fc_other_expr(received_signature, params_cnt)) {
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
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool fc_expr(dynstring_t *received_signature) {
    debug_msg("<fc_expr> ->\n");

    int params_cnt = 0;

    // | )
    EXPECTED_OPT(TOKEN_RPAREN);

    // expr
    if (!parse_init(received_signature, true)) {
        goto err;
    }

    params_cnt++;

    // <fc_other_expr>
    if (!fc_other_expr(received_signature, params_cnt)) {
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

    dynstring_t *received_signature = Dynstring.ctor("");

    // FUNCTION CALL START

    // (
    EXPECTED(TOKEN_LPAREN);

    // <fc_expr>
    if (!fc_expr(received_signature)) {
        goto err;
    }

    // FUNCTION CALL END

    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
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
    if (!parse_init(received_signature, false)) {
        goto err;
    }

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
    if (!parse_init(received_signature, false)) {
        return false;
    }

    // TODO: | e (check received signature on empty)

    // <r_other_expr>
    if (!r_other_expr(received_signature)) {
        return false;
    }

    return true;
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

    dynstring_t *received_signature = Dynstring.ctor("");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    if (!parse_init(received_signature, false)) {
        goto err;
    }

    // <a_other_expr>
    if (!a_other_expr(ids_list)) {
        goto err;
    }

    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
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

    dynstring_t *received_signature = Dynstring.ctor("");

    // expr
    if (!parse_init(received_signature, false)) {
        goto err;
    }

    // <a_other_expr>
    if (!a_other_expr(ids_list)) {
        goto err;
    }

    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
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
        // =
        EXPECTED(TOKEN_ASSIGN);

        // <a_expr>
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
    List.append(ids_list, Dynstring.dup(id_name));

    // <a_other_id>
    if (!a_other_id(ids_list)) {
        goto err;
    }

    noerr:
    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
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
    List.append(ids_list, Dynstring.dup(id_name));

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

    // <r_expr>
    return r_expr(received_signature);
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

    // expr
    return parse_init(received_signature, false);
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

    if (!check_function(id_name)) {
        goto err;
    }

    // <func_call>
    if (!func_call(id_name)) {
        goto err;
    }

    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
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

    if (check_function(id_name)) {
        // <func_call>
        if (!func_call(id_name)) {
            goto err;
        }
    } else {
        // <assign_id>
        if (!assign_id(id_name)) {
            goto err;
        }
    }

    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
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
