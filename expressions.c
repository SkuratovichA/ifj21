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
#define STACK_ITEM_PEEK(stack, item)          \
    do {                                      \
        (item) = Stack.peek((stack));         \
        if ((item) == NULL) {                 \
            Errors.set_error(ERROR_INTERNAL); \
            goto err;                         \
        }                                     \
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
#define CHECK_DEFINITION(id_name)                                       \
    do {                                                                \
        if (!Symstack.get_local_symbol(symstack, (id_name), NULL)) {    \
            Errors.set_error(ERROR_DEFINITION);                         \
            goto err;                                                   \
        }                                                               \
    } while (0)

/**
 * @brief Get function params and returns (types).
 *
 * @param id_name name of the function.
 * @param func_params is a vector for params.
 * @param func_returns is a vector for returns.
 */
#define GET_FUNCTION_SIGNATURES(id_name, func_params, func_returns)                         \
    do {                                                                                    \
        symbol_t *sym;                                                                      \
                                                                                            \
        if (!Symtable.get_symbol(global_table, id_name, &sym)) {                             \
            goto err;                                                                       \
        }                                                                                   \
                                                                                            \
        if ((func_params) != NULL) {                                                        \
            Dynstring.cat((func_params), Semantics.is_defined(sym->function_semantics) ?    \
                                    sym->function_semantics->definition.params :            \
                                    sym->function_semantics->declaration.params);           \
        }                                                                                   \
                                                                                            \
        if ((func_returns) != NULL) {                                                       \
            Dynstring.cat((func_returns), Semantics.is_defined(sym->function_semantics) ?   \
                                    sym->function_semantics->definition.returns :           \
                                    sym->function_semantics->declaration.returns);          \
        }                                                                                   \
    } while(0)

/**
 * @brief Checks if signature is empty.
 *
 * @param sgt
 */
#define CHECK_EMPTY_SIGNATURE(sgt)                      \
    do {                                                \
        if (Dynstring.len((sgt)) == 0) {                \
            Errors.set_error(ERROR_SYNTAX);             \
            goto err;                                   \
        }                                               \
    } while(0)

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
 * A = {*, /, //, %},
 * B = {+, -},
 * C = {<, <=, >, >=, ==, ~=}
 * D = {#, not, - (unary)}
 *
 *  |   |  id |   ^ |   A |   B |   C |   D |  .. | and |  or |   $ |
 *  | f |  12 |  12 |  10 |   8 |   6 |  10 |   6 |   4 |   2 |   0 |
 *  | g |  13 |  11 |   9 |   7 |   5 |  11 |   7 |   3 |   1 |   0 |
 */

/**
 * f - represents rows of the precedence table.
 */
static const int f[21] = {12, 12, 10, 10, 10, 10, 8, 8, 6, 6, 6, 6, 6, 6, 10, 10, 10, 6, 4, 2, 0};

/**
 * g - represents columns.
 */
static const int g[21] = {13, 11, 9, 9, 9, 9, 7, 7, 5, 5, 5, 5, 5, 5, 11, 11, 11, 7, 3, 1, 0};

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
        case TOKEN_CARET:
            return OP_CARET;
        case TOKEN_PERCENT:
            return OP_PERCENT;
        case TOKEN_MINUS_UNARY:
            return OP_MINUS_UNARY;
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
        case OP_CARET:
            return "^";
        case OP_PERCENT:
            return "%";
        case OP_MINUS_UNARY:
            return "- (unary)";
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

static bool is_binary_op(op_list_t op) {
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
        case OP_PERCENT:
        case OP_CARET:
            return true;
        default:
            break;
    }

    return false;
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
    if (!is_binary_op(op)) {
        goto err;
    }

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
        case OP_MINUS_UNARY:
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
        Generator.expression_binary(op, r_type);

        goto noerr;
    }

    switch (get_op(item->token.type)) {
        // unary_op expr
        case OP_HASH:
        case OP_NOT:
        case OP_MINUS_UNARY:
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

static void check_unary_minus(op_list_t first_op, op_list_t *second_op, stack_item_t *expr) {
    if (*second_op != OP_SUB || expr != NULL) {
        return;
    }

    // $ minus_unary
    if (first_op == OP_DOLLAR) {
        goto ret;
    }

    // binary_op
    // minus_unary minus_unary
    if (first_op == OP_MINUS_UNARY || is_binary_op(first_op)) {
        goto ret;
    }

    return;
    ret:
    *second_op = OP_MINUS_UNARY;
}

/** Function call declaration for parse_function.
 *
 * @param id_name function identifier name.
 * @param function_returns is an initialized empty vector.
 * @return bool.
 */
static bool func_call(dynstring_t *, dynstring_t *);

/**
 * @brief Parse function if identifier is in global scope.
 *
 * @param stack stack for precedence analyse.
 * @param function_parsed true if function was parsed.
 * @return int.
 */
static bool parse_function(sstack_t *stack, bool *function_parsed) {
    debug_msg("parse_function\n");

    stack_item_t *top;
    dynstring_t *id_name = NULL;
    stack_item_t *new_expr = stack_item_ctor(ITEM_TYPE_EXPR, NULL);

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
    if (!func_call(id_name, new_expr->expression_type)) {
        goto err;
    }

    *function_parsed = true;

    // Push an expression
    Stack.push(stack, stack_item_copy(new_expr));

    // generate get return values assigment
    for (size_t i = 0; i < Dynstring.len(new_expr->expression_type); i++) {
        Generator.func_call_return_value(i);
    }

    noerr:
    Dynstring.dtor(id_name);
    stack_item_dtor(new_expr);
    return true;
    err:
    Dynstring.dtor(id_name);
    stack_item_dtor(new_expr);
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

    stack_item_t *new_expr = stack_item_ctor(ITEM_TYPE_EXPR, NULL);

    // (
    if (Scanner.get_curr_token().type != TOKEN_LPAREN) {
        goto noerr;
    }

    EXPECTED(TOKEN_LPAREN);

    // expr
    if (!parse_init(new_expr->expression_type)) {
        goto err;
    }

    // )
    EXPECTED(TOKEN_RPAREN);

    *parents_parsed = true;

    // Push an expression
    Stack.push(stack, stack_item_copy(new_expr));

    noerr:
    stack_item_dtor(new_expr);
    return true;
    err:
    stack_item_dtor(new_expr);
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
    stack_item_t *top;
    stack_item_t *expr = NULL;
    bool function_parsed = false;
    bool parents_parsed = false;

    // Try parse parents
    if (!parse_parents(stack, &parents_parsed)) {
        goto err;
    }

    // Try parse a function
    if (!parents_parsed && !hard_reduce) {
        if (!parse_function(stack, &function_parsed)) {
            goto err;
        }
    }

    // Pop expression if we have it on the top of the stack
    STACK_ITEM_PEEK_EXPR(stack, expr);

    // Peek top item from the stack
    STACK_ITEM_PEEK(stack, top);

    op_list_t first_op = (top->type == ITEM_TYPE_DOLLAR) ? OP_DOLLAR : get_op(top->token.type);
    op_list_t second_op = get_op(Scanner.get_curr_token().type);

    check_unary_minus(first_op, &second_op, expr);

    debug_msg("Top: { %s } Next: { %s%s }\n",
              op_to_string(first_op),
              Scanner.to_string(Scanner.get_curr_token().type),
              (second_op == OP_MINUS_UNARY) ? " (unary)" : "");

    // Check an expression end
    if (!hard_reduce) {
        hard_reduce = expression_end(first_op, function_parsed, parents_parsed);
    }

    // Check a success parsing
    if (parse_success(first_op, second_op, hard_reduce)) {
        debug_msg("Successful parsing\n");

        if (expr == NULL) {
            goto noerr;
        }

        // Append nil if expression type is empty
        if (Dynstring.len(expr->expression_type) == 0) {
            Dynstring.append(expr->expression_type, 'n');
        }

        if (received_signature != NULL) {
            // Set return types
            Dynstring.cat(received_signature, expr->expression_type);
        }
        goto noerr;
    }

    // Truncate expression type and clear generator stack
    // if there is expression on top of the precedence stack
    if (expr != NULL && expr->type == ITEM_TYPE_EXPR) {
        // Append nil if expression type is empty
        if (Dynstring.len(expr->expression_type) == 0) {
            Dynstring.append(expr->expression_type, 'n');
        }

        Semantics.trunc_signature(expr->expression_type);
        // TODO: clear stack in generated code
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

        // If second operand was unary minus,
        // then change token type of top stack element
        // from TOKEN_SUB to TOKEN_UNARY_MINUS
        if (second_op == OP_MINUS_UNARY) {
            STACK_ITEM_PEEK(stack, top);
            top->token.type = TOKEN_MINUS_UNARY;
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
 * @param expected_params
 * @param last_expression
 * @param func_name
 * @param params_cnt counter of function parameters.
 * @return bool.
 */
static bool fc_other_expr(dynstring_t *expected_params,
                          dynstring_t *last_expression,
                          dynstring_t *func_name,
                          size_t params_cnt) {
    debug_msg("[fc_other_expr] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");
    dynstring_t *curr_expected_param = Dynstring.ctor("");
    type_recast_t r_type = NO_RECAST;

    // | )
    if (Scanner.get_curr_token().type == TOKEN_RPAREN) {
        for (size_t i = 0; i < Dynstring.len(last_expression); i++) {
            if (Dynstring.cmp_c_str(func_name, "write") == 0) {
                // generate code for write
                Generator.expression_pop();
                Generator.func_call("write");
                continue;
            }

            if (params_cnt == Dynstring.len(expected_params) ||
                !Semantics.check_type_compatibility(Dynstring.c_str(expected_params)[params_cnt],
                                                    Dynstring.c_str(last_expression)[i],
                                                    &r_type)) {
                Errors.set_error(ERROR_FUNCTION_SEMANTICS);
                goto err;
            }

            // recast if needed and assign parameter
            if (r_type != NO_RECAST) {
                Generator.recast_int_to_number();
            }
            Generator.expression_pop();
            Generator.func_call_pass_param(params_cnt);

            r_type = NO_RECAST;
            params_cnt++;
        }

        EXPECTED(TOKEN_RPAREN);
        goto noerr;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // Truncate signature if it has multiple return types
    // and located not at the end of statement
    Semantics.trunc_signature(last_expression);

    if (Dynstring.cmp_c_str(func_name, "write") == 0) {
        // generate code for write
        Generator.expression_pop();
        Generator.func_call("write");
    } else {
        if (params_cnt == Dynstring.len(expected_params) ||
            !Semantics.check_type_compatibility(Dynstring.c_str(expected_params)[params_cnt],
                                                Dynstring.c_str(last_expression)[0],
                                                &r_type)) {
            Errors.set_error(ERROR_FUNCTION_SEMANTICS);
            goto err;
        }

        // recast if needed and assign parameter
        if (r_type != NO_RECAST) {
            Generator.recast_int_to_number();
        }
        Generator.expression_pop();
        Generator.func_call_pass_param(params_cnt);
    }

    params_cnt++;

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // [fc_other_expr]
    if (!fc_other_expr(expected_params, received_signature, func_name, params_cnt)) {
        goto err;
    }

    noerr:
    Dynstring.dtor(received_signature);
    Dynstring.dtor(curr_expected_param);
    return true;
    err:
    Dynstring.dtor(received_signature);
    Dynstring.dtor(curr_expected_param);
    return false;
}

/** Function call expression.
 *
 * !rule [fc_expr] -> expr [fc_other_expr] | )
 *
 * @param expected_params
 * @param func_name
 * @return bool.
 */
static bool fc_expr(dynstring_t *expected_params, dynstring_t *func_name) {
    debug_msg("[fc_expr] ->\n");

    size_t params_cnt = 0;
    dynstring_t *received_signature = Dynstring.ctor("");

    // | )
    if (Scanner.get_curr_token().type == TOKEN_RPAREN) {
        // Check if function does not have any parameters
        if (Dynstring.len(expected_params) > 0) {
            Errors.set_error(ERROR_FUNCTION_SEMANTICS);
            goto err;
        }

        EXPECTED(TOKEN_RPAREN);
        goto noerr;
    }

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // [fc_other_expr]
    if (!fc_other_expr(expected_params, received_signature, func_name, params_cnt)) {
        goto err;
    }

    noerr:
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
    return false;
}

/** Function call.
 *
 * !rule [func_call] -> ( [fc_expr]
 *
 * @param id_name function identifier name.
 * @param function_returns is an initialized empty vector.
 * @return bool.
 */
static bool func_call(dynstring_t *id_name, dynstring_t *function_returns) {
    debug_msg("[func_call] ->\n");

    dynstring_t *expected_params = Dynstring.ctor("");
    GET_FUNCTION_SIGNATURES(id_name, expected_params, function_returns);


    // generate code for function call start
    if (Dynstring.cmp_c_str(id_name, "write") != 0) {
        Generator.func_createframe();
    }

    // (
    EXPECTED(TOKEN_LPAREN);

    // [fc_expr]
    if (!fc_expr(expected_params, id_name)) {
        goto err;
    }

    // generate code for function call
    if (Dynstring.cmp_c_str(id_name, "write") != 0) {
        Generator.func_call(Dynstring.c_str(id_name));
    }

    Dynstring.dtor(expected_params);
    return true;
    err:
    Dynstring.dtor(expected_params);
    return false;
}

/** Return other expressions.
 *
 * !rule [r_other_expr] -> , expr [r_other_expr] | e
 *
 * @param received_rets is an initialized empty vector.
 * @param last_expression is an initialized vector for last expression type/s.
 * @param func_rets
 * @param return_cnt
 * @return bool.
 */
static bool r_other_expr(dynstring_t *received_rets,
                         dynstring_t *last_expression,
                         size_t func_rets,
                         size_t return_cnt) {
    debug_msg("[r_other_expr] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        Dynstring.cat(received_rets, last_expression);

        Generator.return_last(Dynstring.len(received_rets), return_cnt);
        goto noerr;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // Truncate signature if it has multiple return types
    // and located not at the end of expression
    size_t to_pop = Dynstring.len(last_expression);
    Semantics.trunc_signature(last_expression);
    Dynstring.cat(received_rets, last_expression);

    Generator.return_not_last(to_pop, return_cnt);

    return_cnt++;

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    CHECK_EMPTY_SIGNATURE(received_signature);

    // [r_other_expr]
    if (!r_other_expr(received_rets, received_signature, func_rets, return_cnt)) {
        goto err;
    }

    noerr:
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
    return false;
}

/**
 * Return expression.
 *
 * !rule [r_expr] -> expr [r_other_expr]
 *
 * @param received_rets is an initialized empty vector.
 * @param func_rets
 * @return bool.
 */
static bool r_expr(dynstring_t *received_rets, size_t func_rets) {
    debug_msg("[r_expr] ->\n");

    size_t return_cnt = 0;
    dynstring_t *received_signature = Dynstring.ctor("");

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    // Check if expression was empty
    if (Dynstring.len(received_signature) == 0) {
        goto noerr;
    }

    // [r_other_expr]
    if (!r_other_expr(received_rets, received_signature, func_rets, return_cnt)) {
        goto err;
    }

    noerr:
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(received_signature);
    return false;
}

/**
 * @brief Check multiple assignment.
 *
 * @param ids_list list of identifiers.
 * @param rhs_expressions is an initialized vector with expression types.
 * @return bool.
 */
static bool check_multiple_assignment(list_t *ids_list, dynstring_t *rhs_expressions) {
    list_item_t *id = ids_list->head;
    char *rhs_expr_str = Dynstring.c_str(rhs_expressions);
    size_t id_cnt = 0;
    type_recast_t r_type = NO_RECAST;

    while (id != NULL) {
        // there is no more expressions
        if (id_cnt >= Dynstring.len(rhs_expressions)) {
            // TODO: assign nil to other variables
            id_cnt++;
            id = id->next;
            continue;
        }

        symbol_t *sym;
        if (!Symstack.get_local_symbol(symstack, id->data, &sym)) {
            Errors.set_error(ERROR_DEFINITION);
            goto err;
        }

        if (!Semantics.check_type_compatibility(Semantics.of_id_type(sym->type),
                                                rhs_expr_str[id_cnt],
                                                &r_type)) {
            Errors.set_error(ERROR_TYPE_MISSMATCH);
            goto err;
        }

        // TODO: recast in generator

        r_type = NO_RECAST;
        id_cnt++;
        id = id->next;
    }

    return true;
    err:
    return false;
}

/** Assignment other expressions.
 *
 * !rule [a_other_expr] -> , expr [a_other_expr] | e
 *
 * @param rhs_expressions is an initialized vector for rhs expression types.
 * @param last_expression is an initialized vector for last expression type/s.
 * @return bool.
 */
static bool a_other_expr(dynstring_t *rhs_expressions, dynstring_t *last_expression) {
    debug_msg("[a_other_expr] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // | e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        Dynstring.cat(rhs_expressions, last_expression);
        goto noerr;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // Truncate signature if it has multiple return types
    // and located not at the end of expression
    size_t expr_num = Dynstring.len(last_expression);
    Semantics.trunc_signature(last_expression);
    Dynstring.cat(rhs_expressions, last_expression);

    // clear generator stack
    for(size_t i = 0; i < expr_num - 1; i++) {
        Generator.expression_pop();
    }


    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    CHECK_EMPTY_SIGNATURE(received_signature);

    // [a_other_expr]
    if (!a_other_expr(rhs_expressions, received_signature)) {
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
 * @param rhs_expressions is an initialized vector for rhs expression types.
 * @return bool.
 */
static bool a_expr(dynstring_t *rhs_expressions) {
    debug_msg("[a_expr] ->\n");

    dynstring_t *received_signature = Dynstring.ctor("");

    // expr
    if (!parse_init(received_signature)) {
        goto err;
    }

    CHECK_EMPTY_SIGNATURE(received_signature);

    // [a_other_expr]
    if (!a_other_expr(rhs_expressions, received_signature)) {
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
    dynstring_t *rhs_expressions = Dynstring.ctor("");

    // | = [a_expr]
    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        // =
        EXPECTED(TOKEN_ASSIGN);

        // [a_expr]
        if (!a_expr(rhs_expressions)) {
            goto err;
        }

        check_multiple_assignment(ids_list, rhs_expressions);
        Generator.assignment(ids_list, rhs_expressions);
        goto noerr;
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
    Dynstring.dtor(rhs_expressions);
    return true;
    err:
    Dynstring.dtor(id_name);
    Dynstring.dtor(rhs_expressions);
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
 * @param func_rets
 * @return true if successive parsing performed.
 */
static bool Return_expressions(pfile_t *pfile_, dynstring_t *received_signature, size_t func_rets) {
    debug_msg("Return_expression\n");

    pfile = pfile_;

    // [r_expr]
    return r_expr(received_signature, func_rets);
}

/**
 * @brief Default expression after = in the local assignment
 *        Or an expression in the conditional statement in cycles, conditions.
 *        semantic controls are performed inside parser.c
 *
 * @param pfile_
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
        goto err;
    }

    CHECK_EMPTY_SIGNATURE(received_signature);

    Generator.expression_pop();
    // TODO: CLEARS stack?

    if (type_expr_statement == TYPE_EXPR_DEFAULT) {
        goto noerr;
    }

    if (Dynstring.cmp_c_str(received_signature, "b") != 0) {
        // recast type of an expression to boolean, if it is not empty.
        Generator.comment("recast expression to bool");
        Generator.recast_expression_to_bool();
    }

    // clear Dynstring and append a new type means expression was typecasted.
    Dynstring.clear(received_signature);
    Dynstring.append(received_signature, 'b');

    noerr:
    return true;
    err:
    return false;
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
    if (!func_call(id_name, NULL)) {
        goto err;
    }

    // TODO: CLEARS stack?

    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/**
 * @brief Function calling or assignments in the local scope.
 * !rule [function_expression] -> id [func_call] | id [assign_id]
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
        if (!func_call(id_name, NULL)) {
            goto err;
        }
        // TODO: CLEARS stack?
    } else {
        // [assign_id]
        if (!assign_id(id_name)) {
            goto err;
        }
        // TODO: CLEARS stack?
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
