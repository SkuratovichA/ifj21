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
 * @brief Safely peek item from top of the stack.
 *
 * @param stack
 * @param item
 */
#define STACK_ITEM_PEEK(stack, item)        \
    do {                                    \
        (item) = Stack.peek((stack));       \
        if ((item) == NULL) {               \
            goto err;                       \
        }                                   \
    } while (0)

/**
 * @brief Peek expression if it is in top of the stack.
 *
 * @param stack
 * @param item
 */
#define STACK_ITEM_PEEK_EXPR(stack, item)               \
    do {                                                \
        stack_item_t *tmp;                              \
        STACK_ITEM_PEEK((stack), tmp);                  \
        if (tmp->type == ITEM_TYPE_EXPR) {              \
            (item) = stack_item_copy(tmp);              \
            Stack.pop((stack), stack_item_dtor);        \
        }                                               \
    } while (0)

/**
 * @brief Checks if identifier is defined.
 *
 * @param id_name identifier name.
 */
#define CHECK_DEFINITION(id_name)                                             \
    do {                                                                \
        if (!Symstack.get_local_symbol(symstack, (id_name), NULL)) {    \
            Errors.set_error(ERROR_DEFINITION);                         \
            goto err;                                                   \
        }                                                               \
    } while (0)

/**
 * @brief Checks if identifier is a function.
 *
 * @param id_name identifier name.
 * @return bool.
 */
static inline bool is_a_function(dynstring_t *id_name) {
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
 *  |   |  id |   A |   B |   C |   D |  .. | and |  or |   $ |
 *  | f |  12 |  10 |   8 |   6 |  12 |   6 |   4 |   2 |   0 |
 *  | g |  13 |   9 |   7 |   5 |  11 |   7 |   3 |   1 |   0 |
 */

/**
 * f - represents rows of the precedence table.
 */
static const int f[18] = {12, 10, 10, 10, 8, 8, 6, 6, 6, 6, 6, 6, 12, 12, 6, 4, 2, 0};

/**
 * g - represents columns.
 */
static const int g[18] = {13, 9, 9, 9, 7, 7, 5, 5, 5, 5, 5, 5, 11, 11, 7, 3, 1, 0};

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
    new_item->expression_type = Dynstring.ctor("");
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
    new_item->expression_type = Dynstring.dup(item->expression_type);
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

    Dynstring.dtor(s_item->expression_type);

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
    // id id
    if (first_op == OP_ID && second_op == OP_ID) {
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
 * @brief Check binary operator.
 *
 * @param r_stack stack with handle (rule).
 * @param result_op variable to set operator.
 * @return bool.
 */
static bool binary_op(sstack_t *r_stack, op_list_t *result_op) {
    debug_msg("binary_op ->\n");

    stack_item_t *item;
    op_list_t op;

    STACK_ITEM_PEEK(r_stack, item);

    if (item->type != ITEM_TYPE_TOKEN) {
        goto err;
    }

    op = get_op(item->token.type);
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV_I:
        case OP_DIV_F:
        case OP_STRCAT:
        case OP_AND:
        case OP_OR:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_EQ:
        case OP_NE:
            goto noerr;
        default:
            goto err;
    }

    noerr:
    *result_op = op;
    Stack.pop(r_stack, stack_item_dtor);
    return true;
    err:
    Errors.set_error(ERROR_SYNTAX);
    return false;
}

/**
 * @brief Check unary operator.
 *
 * @param r_stack stack with handle (rule).
 * @param result_op variable to set operator.
 * @return bool.
 */
static bool unary_op(sstack_t *r_stack, op_list_t *result_op) {
    debug_msg("unary_op ->\n");

    stack_item_t *item;
    op_list_t op;

    STACK_ITEM_PEEK(r_stack, item);

    if (item->type != ITEM_TYPE_TOKEN) {
        goto err;
    }

    op = get_op(item->token.type);
    switch (op) {
        case OP_HASH:
        case OP_NOT:
            goto noerr;
        default:
            goto err;
    }

    noerr:
    *result_op = op;
    Stack.pop(r_stack, stack_item_dtor);
    return true;
    err:
    Errors.set_error(ERROR_SYNTAX);
    return false;
}

/**
 * @brief Check expression.
 *
 * @param r_stack stack with handle (rule).
 * @param type initialized vector to store an expression type.
 * @return bool.
 */
static bool expr(sstack_t *r_stack, dynstring_t *type) {
    debug_msg("expr ->\n");

    stack_item_t *item;
    STACK_ITEM_PEEK(r_stack, item);

    if (item->type != ITEM_TYPE_EXPR) {
        goto err;
    }

    Dynstring.cat(type, item->expression_type);
    Stack.pop(r_stack, stack_item_dtor);
    return true;
    err:
    Errors.set_error(ERROR_SYNTAX);
    return false;
}

/**
 * @brief Check reduced rule.
 *
 * !rule expr -> expr binary_op expr
 * !rule expr -> unary_op expr
 * !rule expr -> id
 *
 * @param r_stack stack with handle (rule).
 * @param expression_type initialized vector to store an expression type.
 * @return bool.
 */
static bool check_rule(sstack_t *r_stack, dynstring_t *expression_type) {
    debug_msg("check_rule ->\n");

    stack_item_t *item;
    dynstring_t *first_type = Dynstring.ctor("");
    dynstring_t *second_type = Dynstring.ctor("");
    op_list_t op;
    type_recast_t r_type = NO_RECAST;

    STACK_ITEM_PEEK(r_stack, item);

    // expr binary_op expr
    if (item->type == ITEM_TYPE_EXPR) {
        // expr
        if (!expr(r_stack, first_type)) {
            goto err;
        }

        // binary_op
        if (!binary_op(r_stack, &op)) {
            goto err;
        }

        // expr
        if (!expr(r_stack, second_type)) {
            goto err;
        }

        if (!Semantics.check_binary_compatibility(first_type, second_type, op, expression_type, &r_type)) {
            goto err;
        }

        // generate code for binary operation
        Generator.expression_binary(op);

        goto noerr;
    }

    switch (get_op(item->token.type)) {
        // unary_op expr
        case OP_HASH:
        case OP_NOT:
            // unary_op
            if (!unary_op(r_stack, &op)) {
                goto err;
            }

            // expr
            if (!expr(r_stack, first_type)) {
                goto err;
            }

            if (!Semantics.check_unary_compatibility(first_type, op, expression_type)) {
                goto err;
            }

            // generate code for unary operation
            Generator.expression_unary(op);

            goto noerr;

        // id
        case OP_ID:
            if (!Semantics.check_operand(item->token, expression_type)) {
                goto err;
            }

            // generate code for operand
            Generator.expression_operand(item->token);

            Stack.pop(r_stack, stack_item_dtor);
            goto noerr;

        default:
            goto err;
    }

    noerr:
    Dynstring.dtor(first_type);
    Dynstring.dtor(second_type);
    return true;
    err:
    Dynstring.dtor(first_type);
    Dynstring.dtor(second_type);
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
    sstack_t *r_stack = NULL;
    stack_item_t *new_expr = stack_item_ctor(ITEM_TYPE_EXPR, NULL);

    // Push expression if exists
    if (expr != NULL) {
        Stack.push(stack, stack_item_copy(expr));
    }

    // Peek item from top of the stack
    STACK_ITEM_PEEK(stack, top);

    // Reduce rule
    r_stack = Stack.ctor();
    while (top->type != ITEM_TYPE_LT && top->type != ITEM_TYPE_DOLLAR) {
        Stack.push(r_stack, stack_item_copy(top));
        Stack.pop(stack, stack_item_dtor);
        STACK_ITEM_PEEK(stack, top);
    }

    /**
     * | check_rule | stack     | state  |
     * | true       | empty     | ok     |
     * | true       | not empty | not ok |
     * | false      | empty     | not ok |
     * | false      | not empty | not ok |
     */
    if (!check_rule(r_stack, new_expr->expression_type) || !Stack.is_empty(r_stack)) {
        debug_msg("Reduction error!\n");
        goto err;
    }

    // Delete less than symbol
    if (top->type == ITEM_TYPE_LT) {
        Stack.pop(stack, stack_item_dtor);
    }

    // Push an expression
    Stack.push(stack, stack_item_copy(new_expr));

    Stack.dtor(r_stack, stack_item_dtor);
    stack_item_dtor(new_expr);
    return true;
    err:
    Stack.dtor(r_stack, stack_item_dtor);
    stack_item_dtor(new_expr);
    return false;
}

/**
 * @brief Check an expression end.
 *
 * @param first_op first operator.
 * @param function_parsed true if function was parsed.
 * @param parents_parsed true if parents were parsed.
 * @return bool.
 */
static bool expression_end(op_list_t first_op,
                           bool function_parsed,
                           bool parents_parsed) {
    bool is_token_id = (Scanner.get_curr_token().type == TOKEN_ID);
    return  (first_op == OP_ID && is_token_id) ||
            (function_parsed && is_token_id) ||
            (parents_parsed && is_token_id);
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
    bool is_first_dollar = (first_op == OP_DOLLAR);
    bool is_second_dollar = (second_op == OP_DOLLAR);
    return  (is_first_dollar && is_second_dollar) ||
            (is_first_dollar && hard_reduce);
}

/** Function call declaration for parse_function.
 *
 * @param id_name function identifier name.
 * @return bool.
 */
static bool func_call(dynstring_t *);

/**
 * @brief Parse function if identifier is in global scope.
 *
 * @param stack stack for precedence analyse.
 * @param function_parsed true if function was parsed.
 * @param parents_parsed true if parents were parsed.
 * @return int.
 */
static bool parse_function(sstack_t *stack, bool *function_parsed, bool parents_parsed) {
    debug_msg("parse_function\n");

    dynstring_t *id_name = NULL;
    stack_item_t *top = NULL;

    // If parents were parsed, continue expression parsing
    if (parents_parsed) {
        goto noerr;
    }

    if (Scanner.get_curr_token().type != TOKEN_ID) {
        goto noerr;
    }

    // If first operand is id/string/number/integer, continue expression parsing
    STACK_ITEM_PEEK(stack, top);
    if (top->type == ITEM_TYPE_TOKEN && get_op(top->token.type) == OP_ID) {
        goto noerr;
    }

    GET_ID_SAFE(id_name);

    if (!is_a_function(id_name)) {
        goto noerr;
    }

    // id
    EXPECTED(TOKEN_ID);

    // [func_call]
    if (!func_call(id_name)) {
        goto err;
    }

    *function_parsed = true;

    // Push an expression
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_EXPR, NULL));

    noerr:
    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** Parse initialize declaration for parse_parents.
 *
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool parse_init(dynstring_t *);

/**
 * @brief Parse parents.
 *
 * @param stack for precedence analyse.
 * @param parents_parsed true if parents were parsed.
 * @return
 */
static bool parse_parents(sstack_t *stack, bool *parents_parsed) {
    debug_msg("parse_parents \n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // (
    if (Scanner.get_curr_token().type != TOKEN_LPAREN) {
        goto noerr;
    }

    EXPECTED(TOKEN_LPAREN);

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    debug_msg("rparen\n");

    // )
    EXPECTED(TOKEN_RPAREN);

    *parents_parsed = true;

    // Push an expression
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_EXPR, NULL));

    noerr:
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
    return false;
}

/**
 * @brief Expression parsing.
 *
 * @param stack stack for precedence analyse.
 * @param received_signature is an initialized empty vector.
 * @param hard_reduce reduce without precedence analyse.
 * @return bool.
 */
static bool parse(sstack_t *stack, dynstring_t *received_signature, bool hard_reduce) {
    debug_msg("parse ->\n");

    int cmp;
    stack_item_t *expr = NULL;
    stack_item_t *top = NULL;
    bool function_parsed = false;
    bool parents_parsed = false;

    // Try parse parents
    if (!parse_parents(stack, &parents_parsed)) {
        goto err;
    }

    // Try parse a function
    if (!parse_function(stack, &function_parsed, parents_parsed)) {
        goto err;
    }

    // Pop expression if we have it on the top of the stack
    STACK_ITEM_PEEK_EXPR(stack, expr);

    // Peek top item from the stack
    STACK_ITEM_PEEK(stack, top);

    op_list_t first_op = (top->type == ITEM_TYPE_DOLLAR) ? OP_DOLLAR : get_op(top->token.type);
    op_list_t second_op = get_op(Scanner.get_curr_token().type);

    debug_msg("Top: { %s } Next: { %s }\n",
              op_to_string(first_op),
              Scanner.to_string(Scanner.get_curr_token().type));

    // Check an expression end
    if (!hard_reduce) {
        hard_reduce = expression_end(first_op, function_parsed, parents_parsed);
    }

    // Check a success parsing
    if (parse_success(first_op, second_op, hard_reduce)) {
        if (expr != NULL && received_signature != NULL) {
            // Set return types
            Dynstring.cat(received_signature, expr->expression_type);
        }

        debug_msg("Successful parsing\n");
        if (expr != NULL) {
            Generator.comment(" ---------- expression end? --------------");
            Generator.expression_pop();
        }
        goto noerr;
    }

    // Precedence comparison
    if (!hard_reduce && !precedence_cmp(first_op, second_op, &cmp)) {
        debug_msg("Precedence error\n");
        Errors.set_error(ERROR_SYNTAX);
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

    if (!parse(stack, received_signature, hard_reduce)) {
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
 * @return bool.
 */
static bool parse_init(dynstring_t *received_signature) {
    debug_msg("parse_init ->\n");

    sstack_t *stack = Stack.ctor();

    // Push $ on stack
    Stack.push(stack, stack_item_ctor(ITEM_TYPE_DOLLAR, NULL));

    // Parse expression
    if (!parse(stack, received_signature, false)) {
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
 * !rule [fc_other_expr] -> , expr [fc_other_expr] | )
 *
 * @param received_signature is an initialized empty vector.
 * @param params_cnt counter of function parameters.
 * @return bool.
 */
static bool fc_other_expr(dynstring_t *received_signature, int params_cnt) {
    debug_msg("[fc_other_expr] ->\n");

    // | )
    EXPECTED_OPT(TOKEN_RPAREN);
    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // generate code for a function parameter
    Generator.func_call_pass_param(params_cnt++);

    // [fc_other_expr]
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
 * !rule [fc_expr] -> expr [fc_other_expr] | )
 *
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool fc_expr(dynstring_t *received_signature) {
    debug_msg("[fc_expr] ->\n");

    int params_cnt = 0;

    // | )
    EXPECTED_OPT(TOKEN_RPAREN);

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // generate code for a function parameter
    Generator.func_call_pass_param(params_cnt++);

    // [fc_other_expr]
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
 * !rule [func_call] -> ( [fc_expr]
 *
 * @param id_name function identifier name.
 * @return bool.
 */
static bool func_call(dynstring_t *id_name) {
    debug_msg("[func_call] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // generate code for function call start
    Generator.func_createframe();

    // (
    EXPECTED(TOKEN_LPAREN);

    // [fc_expr]
    if (!fc_expr(received_signature)) {
        goto err;
    }

    // TODO: check function parameters signature

    // generate code for function call
    Generator.func_call(Dynstring.c_str(id_name));
    // TODO: generate get return values assigment
    //Generator.func_call_return_value(0);

    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
    return false;
}

/** Return other expressions.
 *
 * !rule [r_other_expr] -> , expr [r_other_expr] | e
 *
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool r_other_expr(dynstring_t *received_signature, size_t *return_cnt) {
    debug_msg("[r_other_expr] ->\n");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }
    (*return_cnt)--;

    // TODO: check if expression was not empty

    // [r_other_expr]
    if (!r_other_expr(received_signature, return_cnt)) {
        goto err;
    }

    return true;
    err:
    return false;
}

/**
 * Return expression.
 *
 * !rule [r_expr] -> expr [r_other_expr] | e
 *
 * @param received_signature is an initialized empty vector.
 * @return bool.
 */
static bool r_expr(dynstring_t *received_signature, size_t return_cnt) {
    debug_msg("[r_expr] ->\n");

    // expr
    if (!parse_init(received_signature)) {
        return false;
    }
    return_cnt--;

    // TODO: check if expression was empty

    // [r_other_expr]
    if (!r_other_expr(received_signature, &return_cnt)) {
        return false;
    }
    return true;
}

/** Assignment other expressions.
 *
 * !rule [a_other_expr] -> , expr [a_other_expr] | e
 *
 * @param ids_list list of identifiers.
 * @return bool.
 */
static bool a_other_expr(list_t *ids_list) {
    debug_msg("[a_other_expr] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        goto noerr;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // TODO: check if expression was not empty
    // TODO: check if id is exist for an expression
    // TODO: check types compatability of id and expression
    // TODO: generate code for assignment

    // [a_other_expr]
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
 * !rule [a_expr] -> expr [a_other_expr]
 *
 * @param ids_list list of identifiers.
 * @return bool.
 */
static bool a_expr(list_t *ids_list) {
    debug_msg("[a_expr] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // TODO: check if expression was not empty
    // TODO: check types compatability of id and expression
    // TODO: generate code for assignment
    Generator.var_assignment(ids_list->head->data);

    // [a_other_expr]
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
 * !rule [a_other_id] -> , id [a_other_id] | = [a_expr]
 *
 * @param ids_list list of identifiers.
 * @return bool.
 */
static bool a_other_id(list_t *ids_list) {
    debug_msg("[a_other_id] ->\n");

    dynstring_t *id_name = NULL;

    // | = [a_expr]
    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        // =
        EXPECTED(TOKEN_ASSIGN);

        // [a_expr]
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

    CHECK_DEFINITION(id_name);

    // Append next identifier
    List.append(ids_list, Dynstring.dup(id_name));

    // [a_other_id]
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
 * !rule [assign_id] -> [a_other_id]
 *
 * @param id_name identifier name.
 * @return bool.
 */
static bool assign_id(dynstring_t *id_name) {
    debug_msg("[assign_id] ->\n");

    // Create a list of identifiers
    list_t *ids_list = List.ctor();

    CHECK_DEFINITION(id_name);

    // Append first identifier
    List.append(ids_list, Dynstring.dup(id_name));

    // [a_other_id]
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
static bool Return_expressions(pfile_t *pfile_, dynstring_t *received_signature, size_t ret_cnt) {
    debug_msg("Return_expression\n");

    pfile = pfile_;

    // [r_expr]
    return r_expr(received_signature, ret_cnt);
}

/**
 * @brief Default expression after = in the local assignment
 *        Or an expression in the conditional statement in cycles, conditions.
 *        semantic controls are performed inside parser.c
 *
 * @param received_signature is an initialized empty vector.
 * @param type_expr_statement TYPE_EXPR_DEFAULT will not be casted to boolean.
 *                            TYPE_EXPR_CONDITIONAL will be casted to boolean.
 * @return true if successive parsing performed.
 */
static bool Default_expression(pfile_t *pfile_,
                               dynstring_t *received_signature,
                               type_expr_statement_t type_expr_statement) {
    debug_msg("Default_expression\n");

    pfile = pfile_;

    // expr
    if (!parse_init(received_signature)) {
        return false;
    }

    if (type_expr_statement == TYPE_EXPR_DEFAULT) {
        goto ret;
    }

    // don't need to recast an empty expression,
    // error will be handled in parser
    if (Dynstring.cmp_c_str(received_signature, "") == 0) {
        goto ret;
    }

    // recast type of an expression to boolean, if it is not empty.
    Generator.comment("recast expression to bool");
    Generator.recast_expression_to_bool();

    // clear Dynstring and append a new type means expression was typecasted.
    Dynstring.clear(received_signature);
    Dynstring.append(received_signature, 'b');

    ret:
    return true;
}

/**
 * @brief Function calling in the global scope. `id( ...`
 * !rule [global_expression] -> id [func_call]
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

    if (!is_a_function(id_name)) {
        Errors.set_error(ERROR_DEFINITION);
        goto err;
    }

    // [func_call]
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
 * !rule [function_expression] -> id FUCK ME IN THE BRAIN I CAN STAND THIS PROJECT ANYMORE I WANNA DIE
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

    if (is_a_function(id_name)) {
        // [func_call]
        if (!func_call(id_name)) {
            goto err;
        }
    } else {
        // [assign_id]
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
