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
    return Symtable.get_symbol(global_table, id_name, NULL);
}

/**
 * Precedence function table.
 * f, g - precedence functions.
 *
 * A = {*, /, //},
 * B = {+, -},
 * C = {<, <=, >, >=, ==, ~=}
 * D = {#, not}
 *
 *  |   |  id |   ( |   ) |   A |   B |   C |   D |  .. | and |  or |   $ |
 *  | f |  12 |   0 |  12 |  10 |   8 |   6 |  12 |   6 |   4 |   2 |   0 |
 *  | g |  13 |  13 |   0 |   9 |   7 |   5 |  11 |   7 |   3 |   1 |   0 |
 */

/**
 * f - represents rows of the precedence table.
 */
static const int f[20] = {12, 0, 12, 10, 10, 10, 8, 8, 6, 6, 6, 6, 6, 6, 12, 12, 6, 4, 2, 0};

/**
 * g - represents columns.
 */
static const int g[20] = {13, 13, 0, 9, 9, 9, 7, 7, 5, 5, 5, 5, 5, 5, 11, 11, 7, 3, 1, 0};

/**
 * @brief
 *
 * Return operator from the precedence table
 * using token information.
 *
 * @param tok_type type of the token.
 * @return enum value of operator.
 */
static op_list_t get_op(int tok_type) {
    switch (tok_type) {
        case TOKEN_STR:
        case TOKEN_NUM_F:
        case TOKEN_NUM_I:
        case KEYWORD_nil:
        case KEYWORD_0:
        case KEYWORD_1:
        case TOKEN_ID:
            return OP_ID;
        case TOKEN_LPAREN:
            return OP_LPAREN;
        case TOKEN_RPAREN:
            return OP_RPAREN;
        case TOKEN_HASH:
            return OP_HASH;
        case KEYWORD_not:
            return OP_NOT;
        case TOKEN_MUL:
            return OP_MUL;
        case TOKEN_DIV_F:
            return OP_DIV_F;
        case TOKEN_DIV_I:
            return OP_DIV_I;
        case TOKEN_ADD:
            return OP_ADD;
        case TOKEN_SUB:
            return OP_SUB;
        case TOKEN_LT:
            return OP_LT;
        case TOKEN_LE:
            return OP_LE;
        case TOKEN_GT:
            return OP_GT;
        case TOKEN_GE:
            return OP_GE;
        case TOKEN_EQ:
            return OP_EQ;
        case TOKEN_NE:
            return OP_NE;
        case TOKEN_STRCAT:
            return OP_STRCAT;
        case KEYWORD_and:
            return OP_AND;
        case KEYWORD_or:
            return OP_OR;
        default:
            return OP_DOLLAR;
    }
}

/**
 * @brief Convert operator to string.
 *
 * @param op
 * @return char representation of operator in precedence table.
 */
static char *op_to_string(op_list_t op) {
    switch (op) {
        case OP_ID:
            return "id";
        case OP_LPAREN:
            return "(";
        case OP_RPAREN:
            return ")";
        case OP_HASH:
            return "#";
        case OP_NOT:
            return "not";
        case OP_MUL:
            return "*";
        case OP_DIV_I:
            return "//";
        case OP_DIV_F:
            return "/";
        case OP_ADD:
            return "+";
        case OP_SUB:
            return "-";
        case OP_LT:
            return "<";
        case OP_LE:
            return "<=";
        case OP_GT:
            return ">";
        case OP_GE:
            return ">=";
        case OP_EQ:
            return "==";
        case OP_NE:
            return "~=";
        case OP_STRCAT:
            return "..";
        case OP_AND:
            return "and";
        case OP_OR:
            return "or";
        case OP_DOLLAR:
            return "$";
        default:
            return "unrecognized operator";
    }
}

/**
 * @brief Convert stack item to string.
 *
 * @param type
 * @return char representation of stack item.
 */
static char *stack_item_to_string(item_type_t item_type) {
    switch (item_type) {
        case ITEM_TYPE_TOKEN:
            return "token";
        case ITEM_TYPE_EXPR:
            return "expr";
        case ITEM_TYPE_LT:
            return "<";
        case ITEM_TYPE_DOLLAR:
            return "$";
        default:
            return "unrecognized stack item";
    }
}

/**
 * @brief Set token for new created/copied item.
 *
 * @param item
 * @param tok token to set (can be NULL).
 */
static void stack_item_set_token(stack_item_t *item, token_t *tok) {
    if (tok == NULL || item == NULL) {
        goto noerr;
    }

    item->token = *tok;

    if (item->type != ITEM_TYPE_TOKEN) {
        goto noerr;
    }

    if (tok->type == TOKEN_ID) {
        item->token.attribute.id = Dynstring.dup(tok->attribute.id);
    }

    noerr:;
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
    if (item == NULL) {
        return NULL;
    }

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
    if (item == NULL) {
        return;
    }

    stack_item_t *s_item = (stack_item_t *) item;

    if (s_item->type != ITEM_TYPE_TOKEN) {
        goto noerr;
    }

    if (s_item->token.type == TOKEN_ID) {
        Dynstring.dtor(s_item->token.attribute.id);
    }

    noerr:
    free(s_item);
}

/**
 * @brief Pop and return expression if it is on top of the stack.
 * Otherwise return NULL.
 *
 * @param stack stack to compare precedence and analyze an expression.
 * @param top pointer to pointer to item on top of the stack (double pointer for rewriting).
 * @return stack_item_t *
 */
static stack_item_t *pop_expression(sstack_t *stack, stack_item_t **top) {
    stack_item_t *expr = NULL;
    stack_item_t *tmp;

    if ((*top)->type == ITEM_TYPE_EXPR) {
        // We need to pop the expression, because we don't want to compare non-terminals
        tmp = (stack_item_t *) Stack.peek(stack);
        expr = stack_item_copy(tmp);
        Stack.pop(stack, stack_item_dtor);
        // Update top
        *top = (stack_item_t *) Stack.peek(stack);
    }

    return expr;
}

/**
 * @brief Precedence functions error handling.
 * Check existence of relation between two operators.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @return bool.
 */
static bool precedence_check(op_list_t first_op, op_list_t second_op) {
    switch (first_op) {
        // id id
        // id (
        // ) id
        // ) (
        case OP_ID:
        case OP_RPAREN:
            return second_op != OP_ID && second_op != OP_LPAREN;
        // ( $
        case OP_LPAREN:
            return second_op != OP_DOLLAR;
        // $ )
        case OP_DOLLAR:
            return second_op != OP_RPAREN;
        default:
            break;
    }

    return true;
}

/**
 * @brief Compare two operators using precedence functions.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @param cmp result of comparison.
 * >0 if first_op has a higher precedence,
 * =0 if operators have a similar precedence,
 * <0 if first_op has a lower precedence.
 * @return bool.
 */
static bool precedence_cmp(op_list_t first_op, op_list_t second_op, int *cmp) {
    if (!precedence_check(first_op, second_op)) {
        return false;
    }

    *cmp = f[first_op] - g[second_op];
    return true;
}

/**
 * @brief Shift current token to the stack.
 *
 * @param stack stack to compare precedence and analyze an expression.
 * @param expr expression on top of the stack.
 * @param cmp comparison result.
 * @return bool.
 */
static bool shift(sstack_t *stack, stack_item_t *expr, int const cmp) {
    debug_msg("SHIFT ->\n");

    token_t tok;

    // If first_op has a lower precedence, then push less than symbol
    if (cmp < 0) {
        Stack.push(stack, stack_item_ctor(ITEM_TYPE_LT, NULL));
    }

    // Push expression if exists
    if (expr != NULL) {
        Stack.push(stack, stack_item_copy(expr));
    }

    // Push token from the input
    tok = Scanner.get_curr_token();
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_TOKEN, &tok));

    // Get next token
    EXPECTED(tok.type);

    return true;
    err:
    return false;
}

/**
 * @brief Reduce expression.
 *
 * @param stack stack to compare precedence and analyze an expression.
 * @param expr expression on top of the stack.
 * @return bool.
 */
static bool reduce(sstack_t *stack, stack_item_t *expr) {
    debug_msg("REDUCE ->\n");

    stack_item_t *top;
    sstack_t *r_stack;

    // Push expression if exists
    if (expr != NULL) {
        Stack.push(stack, stack_item_copy(expr));
    }

    // Peek item from top of the stack
    top = Stack.peek(stack);

    // Reduce rule
    r_stack = Stack.ctor();
    while (top->type != ITEM_TYPE_LT && top->type != ITEM_TYPE_DOLLAR) {
        Stack.push(r_stack, stack_item_copy(top));
        Stack.pop(stack, stack_item_dtor);
        top = Stack.peek(stack);
    }

    // CHECK RULE

    // Delete less than symbol
    if (top->type == ITEM_TYPE_LT) {
        Stack.pop(stack, stack_item_dtor);
    }

    // Push an expression
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_EXPR, NULL));

    Stack.dtor(r_stack, stack_item_dtor);
    return true;
}

/**
 * @brief Check an expression end.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @param is_func_param true if expression is a parameter of function.
 * @return bool.
 */
static bool expression_end(op_list_t first_op, op_list_t second_op, bool is_func_param) {
    return  (first_op == OP_ID && Scanner.get_curr_token().type == TOKEN_ID) ||
            (first_op == OP_RPAREN && Scanner.get_curr_token().type == TOKEN_ID) ||
            (second_op == OP_RPAREN && is_func_param);
}

/**
 * @brief Check a success parsing of the expression.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @param hard_reduce reduce without precedence analyse.
 * @return bool.
 */
static bool parse_success(op_list_t first_op, op_list_t second_op, bool hard_reduce) {
    return  (first_op == OP_DOLLAR && second_op == OP_DOLLAR) ||
            (first_op == OP_DOLLAR && hard_reduce);
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
    int cmp;
    stack_item_t *expr = NULL;

    // CHECK A FUNCTION CALL

    // Peek item from the stack
    stack_item_t *top = (stack_item_t *) Stack.peek(stack);

    // Pop expression if we have it on the top of the stack
    expr = pop_expression(stack, &top);

    op_list_t first_op = (top->type == ITEM_TYPE_DOLLAR) ? OP_DOLLAR : get_op(top->token.type);
    op_list_t second_op = get_op(Scanner.get_curr_token().type);

    debug_msg("Top: { %s } Next: { %s } Expr: { %s }\n",
              op_to_string(first_op),
              Scanner.to_string(Scanner.get_curr_token().type),
              (expr != NULL) ? "yes" : "no");

    // Check an expression end
    if (!hard_reduce) {
        hard_reduce = expression_end(first_op, second_op, is_func_param);
    }

    // Check a success parsing
    if (parse_success(first_op, second_op, hard_reduce)) {
        if (expr == NULL) {
            goto err;
        }

        if (received_signature != NULL) {
            // TODO: set return types
        }

        debug_msg("Successful parsing\n");
        goto noerr;
    }

    // Precedence comparison
    if (!hard_reduce && !precedence_cmp(first_op, second_op, &cmp)) {
        debug_msg("Precedence error\n");
        goto err;
    }

    debug_msg("\n");
    if (!hard_reduce && cmp <= 0) {
        if (!shift(stack, expr, cmp)) {
            goto err;
        }
    } else {
        if (!reduce(stack, expr)) {
            goto err;
        }
    }
    debug_msg("\n");

    if (!parse(stack, received_signature, is_func_param, hard_reduce)) {
        goto err;
    }

    noerr:
    stack_item_dtor(expr);
    return true;
    err:
    stack_item_dtor(expr);
    return false;
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

    // Push $ on stack
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_DOLLAR, NULL));

    // Parse expression
    if (!parse(stack, received_signature, is_func_param, false)) {
        goto err;
    }

    Stack.dtor(stack, stack_item_dtor);
    return true;
    err:
    Stack.dtor(stack, stack_item_dtor);
    return false;
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

    // TODO: | e (check if received signature is empty)

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
        goto noerr;
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

    noerr:
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
