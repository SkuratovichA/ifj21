/**
 * @file expressions.c
 *
 * @brief
 *
 * @author Evgeny Torbin
 */

#include "expressions.h"
#include "scanner.h"
#include "stack.h"
#include "errors.h"
#include "stdbool.h"
#include "code_generator.h"
#include "semantics.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"
// undef debug macros
//#define debug_err(...)
//#define debug_msg(...)
//#define debug_msg_stdout(...)
//#define debug_msg_stderr(...)
//#define debug_todo(...)
//#define debug_assert(cond)
//#define debug_msg_s(...)
//#define DEBUG_SEP
#pragma GCC diagnostic pop


#define IF_STACK_HEAD_IS(_item, _stack, _type)  \
    (_item) = Stack.peek(_stack);               \
    if (!(_item)) {                             \
        return false;                           \
    }                                           \
    if ((_item)->type == (_type)) {

/**
 * Precedence function table.
 * f, g - precedence functions.
 *
 * A = {*, /, //},
 * B = {+, -},
 * C = {<, <=, >, >=, ==, ~=}
 * D = {#, not}
 *
 *  |   |  id |   ( |   ) |   A |   B |   C |   D |  .. | and |  or |   f |   , |   $ |
 *  | f |  12 |   0 |  12 |  10 |   8 |   6 |  12 |   6 |   4 |   2 |  13 |   0 |   0 |
 *  | g |  13 |  13 |   0 |  9 |   7 |   5 |  11 |   7 |   3 |   1 |  13 |   0 |   0 |
 */

/**
 * f - represents rows of the precedence table.
 */
static const int f[22] = {12, 0, 12, 10, 10, 10, 8, 8, 6, 6, 6, 6, 6, 6, 12, 12, 6, 4, 2, 13, 0, 0};

/**
 * g - represents columns.
 */
static const int g[22] = {13, 13, 0, 9, 9, 9, 7, 7, 5, 5, 5, 5, 5, 5, 11, 11, 7, 3, 1, 13, 0, 0};

/**
 * @brief
 *
 * Return operator from the precedence table
 * using token information.
 *
 * @param token
 * @return op_list_t.
 */
static op_list_t get_op(token_t token) {
    switch (token.type) {
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
        case TOKEN_COMMA:
            return OP_COMMA;
        default:
            return OP_DOLLAR;
    }
}

/**
 * @brief Convert operator to string
 *
 * @param op
 * @return char *.
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
        case OP_FUNC:
            return "func";
        case OP_COMMA:
            return ",";
        case OP_DOLLAR:
            return "$";
        default:
            return "unrecognized operator";
    }
}

/**
 * @brief Convert stack item to string.
 * Process all items except ITEM_TYPE_TOKEN.
 *
 * @param type
 * @return char *.
 */
static char *item_to_string(item_type_t type) {
    switch (type) {
        case ITEM_TYPE_EXPR:
            return "expr";
        case ITEM_TYPE_LT:
            return "<";
        case ITEM_TYPE_GT:
            return ">";
        case ITEM_TYPE_EQ:
            return "=";
        case ITEM_TYPE_DOLLAR:
            return "$";
        default:
            return "unrecognized type";
    }
}

/**
 * @brief Convert stack item to string.
 * Process all items including ITEM_TYPE_TOKEN.
 *
 * @param item
 * @return char *.
 */
static char *to_str(stack_item_t *item) {
    return (item->type == ITEM_TYPE_TOKEN) ? op_to_string(get_op(item->token)) : item_to_string(item->type);
}

/**
 * @brief
 *
 * Create stack item with current token or token from argument,
 * if item type is equal to ITEM_TYPE_TOKEN.
 *
 * @param type
 * @param token
 * @return stack_item_t *.
 */
static stack_item_t *stack_item_ctor(item_type_t type, token_t *token) {
    stack_item_t *new_item = calloc(1, sizeof(stack_item_t));
    soft_assert(new_item, ERROR_INTERNAL);
    new_item->type = type;

    if (type == ITEM_TYPE_TOKEN || type == ITEM_TYPE_EXPR) {
        new_item->token = (token) ? *token : Scanner.get_curr_token();
    }

    debug_msg("Created: \"%s\"\n", to_str(new_item));
    return new_item;
}

/**
 * @brief Copy stack item. Original item is not freed.
 *
 * @param item
 * @return stack_item_t.
 */
static stack_item_t *stack_item_copy(stack_item_t *item) {
    stack_item_t *new_item = calloc(1, sizeof(stack_item_t));
    soft_assert(new_item, ERROR_INTERNAL);
    new_item->type = item->type;

    if (new_item->type == ITEM_TYPE_TOKEN || new_item->type == ITEM_TYPE_EXPR) {
        new_item->token = item->token;
    }

    debug_msg("Copied: \"%s\"\n", to_str(item));
    return new_item;
}

/**
 * @brief Free stack item.
 *
 * @param item
 */
static void stack_item_dtor(void *item) {
    debug_msg("Deleted: \"%s\"\n", to_str(item));
    free(item);
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
    if (first_op == OP_ID || first_op == OP_RPAREN) {
        if (second_op == OP_ID || second_op == OP_LPAREN || second_op == OP_FUNC) {
            return false;
        }
    } else if (first_op == OP_LPAREN || first_op == OP_COMMA) {
        if (second_op == OP_DOLLAR) {
            return false;
        }
    } else if (first_op == OP_FUNC) {
        if (second_op != OP_LPAREN && second_op != OP_COMMA) {
            return false;
        }
    } else if (first_op == OP_DOLLAR) {
        if (second_op == OP_RPAREN || second_op == OP_COMMA || second_op == OP_DOLLAR) {
            return false;
        }
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
    if (precedence_check(first_op, second_op)) {
        *cmp = f[first_op] - g[second_op];
        return true;
    }

    return false;
}

/**
 * @brief Check if top item is an expression.
 *
 * @param r_stack stack with handle (rule).
 * @param expr_sem expr semantics.
 * @return bool.
 */
static bool expression(sstack_t *r_stack, expr_semantics_t *expr_sem) {
    debug_msg("EXPECTED: expr\n");

    stack_item_t *item;

    IF_STACK_HEAD_IS(item, r_stack, ITEM_TYPE_EXPR)
        Semantics.add_operand(expr_sem, item->token);
        Stack.pop(r_stack, stack_item_dtor);
        return true;
    }

    return false;
}

/**
 * @brief Check if top item is an expression (without semantic).
 *
 * @param r_stack stack with handle (rule).
 * @return bool.
 */
static bool expression_f(sstack_t *r_stack) {
    debug_msg("EXPECTED: expr\n");

    stack_item_t *item;

    IF_STACK_HEAD_IS(item, r_stack, ITEM_TYPE_EXPR)
        Stack.pop(r_stack, stack_item_dtor);
        return true;
    }

    return false;
}

/**
 * @brief Check if top item is an expected operator.
 *
 * @param r_stack stack with handle (rule).
 * @param exp_op expected operator.
 * @return
 */
static bool single_op(sstack_t *r_stack, op_list_t exp_op) {
    debug_msg("EXPECTED: %s\n", op_to_string(exp_op));

    stack_item_t *item;

    IF_STACK_HEAD_IS(item, r_stack, ITEM_TYPE_TOKEN)
        if (get_op(item->token) == exp_op) {
            Stack.pop(r_stack, stack_item_dtor);
            return true;
        }
    }

    return false;
}

/**
 * @brief
 *
 * !rule <operator> -> + | - | * | / | // | .. | < | <= | > | >= | == | ~=
 *
 * @param r_stack stack with handle (rule).
 * @param expr_sem expr semantics.
 * @return bool.
 */
static bool operator(sstack_t *r_stack, expr_semantics_t *expr_sem) {
    debug_msg("EXPECTED: binary operator\n");

    stack_item_t *item;

    IF_STACK_HEAD_IS(item, r_stack, ITEM_TYPE_TOKEN)
        op_list_t op = get_op(item->token);
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
                Semantics.add_operator(expr_sem, op);
                Stack.pop(r_stack, stack_item_dtor);
                return expression(r_stack, expr_sem);
            default:
                break;
        }
    }

    return false;
}

/**
 * @brief
 *
 * !rule <other_arguments> -> , expr <other_arguments> | )
 *
 * @param r_stack stack with handle (rule).
 * @param func_entries count of entries to functions.
 * @return bool.
 */
static bool other_arguments(sstack_t *r_stack, int *func_entries) {
    debug_msg("rule: , expr <other_arguments> | )\n");

    // ,
    if (single_op(r_stack, OP_COMMA)) {
        // expr <other_arguments>
        return expression_f(r_stack) && other_arguments(r_stack, func_entries);
    } else {
        // )
        if (single_op(r_stack, OP_RPAREN)) {
            (*func_entries)--;
            return true;
        } else {
            return false;
        }
    }
}

/**
 * @brief
 *
 * !rule <arguments> -> expr <other_arguments> | )
 *
 * @param r_stack stack with handle (rule).
 * @param func_entries count of entries to functions.
 * @return bool.
 */
static bool arguments(sstack_t *r_stack, int *func_entries) {
    debug_msg("EXPECTED: expr <other_arguments> | )\n");

    // expr
    if (expression_f(r_stack)) {
        // <other_arguments>
        return other_arguments(r_stack, func_entries);
    } else {
        // )
        if (single_op(r_stack, OP_RPAREN)) {
            (*func_entries)--;
            return true;
        } else {
            return false;
        }
    }
}

/**
 * @brief
 *
 * !rule expr -> expr <operator> | # expr | ( expr ) | id | id ( <arguments>
 *
 * @param r_stack stack with handle (rule).
 * @param func_entries count of entries to functions.
 * @param expr_sem expr semantics.
 * @return bool.
 */
static bool check_rule(sstack_t *r_stack, int *func_entries, expr_semantics_t *expr_sem) {
    debug_msg("EXPECTED: expr <operator> | # expr | ( expr ) | id | id ( <arguments>\n");

    stack_item_t *item = Stack.peek(r_stack);

    if (!item) {
        return false;
    }

    // expr <operator>
    if (item->type == ITEM_TYPE_EXPR) {
        Semantics.add_operand(expr_sem, item->token);
        Stack.pop(r_stack, stack_item_dtor);
        return operator(r_stack, expr_sem);
    }

    if (item->type == ITEM_TYPE_TOKEN) {
        op_list_t op = get_op(item->token);
        switch (get_op(item->token)) {
            // # expr | not expr
            case OP_HASH:
            case OP_NOT:
                Semantics.add_operator(expr_sem, op);
                Stack.pop(r_stack, stack_item_dtor);
                return expression(r_stack, expr_sem);
            // ( expr )
            case OP_LPAREN:
                expr_sem->sem_state = SEMANTIC_DISABLED;
                Stack.pop(r_stack, stack_item_dtor);
                return expression(r_stack, expr_sem) && single_op(r_stack, OP_RPAREN);
            // id | id ( <arguments>
            // i need to declare a variable in this case, so I wrote this commadot
            // never ask why it works like that :D
            case OP_ID:;
                token_t tmp_token = item->token;
                Stack.pop(r_stack, stack_item_dtor);

                if (Stack.peek(r_stack) != NULL) {
                    expr_sem->sem_state = SEMANTIC_DISABLED;
                    return single_op(r_stack, OP_LPAREN) && arguments(r_stack, func_entries);
                }

                Semantics.add_operand(expr_sem, tmp_token);
                return true;
            default:
                break;
        }
    }

    // Otherwise
    return false;
}

/**
 * @brief Pop and return expression if it is on top of the stack.
 * Otherwise return NULL.
 *
 * @param stack stack to compare precedence and analyze an expression.
 * @param top pointer to pointer to item on top of the stack (double pointer for rewriting).
 * @return stack_item_t *
 */
static stack_item_t *pop_expr(sstack_t *stack, stack_item_t **top) {
    stack_item_t *expr = NULL;
    stack_item_t *tmp;

    if ((*top)->type == ITEM_TYPE_EXPR) {
        debug_msg("Expression popped\n");
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
 * @brief Shift current token to the stack.
 *
 * @param pfile program file to pass in to scanner.
 * @param stack stack to compare precedence and analyze an expression.
 * @param expr pointer to expression.
 * @param cmp comparison result.
 */
static void shift(pfile_t *pfile, sstack_t *stack, stack_item_t *expr, int const cmp) {
    debug_msg("SHIFT < | SHIFT =\n");

    // If first_op has a lower precedence, then push less than symbol
    if (cmp < 0) {
        debug_msg("SHIFT <\n");
        Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_LT, NULL));
    }

    // Push expression if exists
    if (expr) {
        debug_msg("Expression pushed\n");
        Stack.push(stack, expr);
    }

    // Push token from the input
    Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_TOKEN, NULL));

    Scanner.get_next_token(pfile);
}

/**
 * @brief Reduce rule.
 *
 * @param stack stack to compare precedence and analyze an expression.
 * @param expr pointer to expression.
 * @param func_entries count of entries to functions.
 * @return bool.
 */
static bool reduce(sstack_t *stack, stack_item_t *expr, int *func_entries) {
    debug_msg("REDUCE >\n");

    // Push expression if exists
    if (expr) {
        Stack.push(stack, expr);
    }

    // Peek item from top of the stack
    stack_item_t *top = Stack.peek(stack);

    // Create expressions semantics structure
    expr_semantics_t *expr_sem = Semantics.ctor_expr();

    // Reduce rule
    sstack_t *r_stack = Stack.ctor();
    while (top->type != ITEM_TYPE_LT && top->type != ITEM_TYPE_DOLLAR) {
        Stack.push(r_stack, (stack_item_t *) stack_item_copy(top));
        Stack.pop(stack, stack_item_dtor);
        top = Stack.peek(stack);
    }

    if (!check_rule(r_stack, func_entries, expr_sem) || Stack.peek(r_stack) != NULL) {
        debug_msg("Reduction error!\n");
        Stack.dtor(r_stack, stack_item_dtor);
        Semantics.dtor_expr(expr_sem);
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }

    Stack.dtor(r_stack, stack_item_dtor);

    // Delete less than symbol
    if (top->type == ITEM_TYPE_LT) {
        Stack.pop(stack, stack_item_dtor);
    }

    debug_msg("\n");
    if (expr_sem->sem_state != SEMANTIC_DISABLED && expr_sem->sem_state != SEMANTIC_IDLE) {
        debug_msg("-- EXPRESSION SEMANTICS --\n");
        if (expr_sem->sem_state == SEMANTIC_OPERAND) {
            debug_msg("Operand #1 = \"%s\"\n", Scanner.to_string(expr_sem->first_operand.type));
        } else if (expr_sem->sem_state == SEMANTIC_UNARY) {
            debug_msg("Operation = \"%s\"\n", op_to_string(expr_sem->op));
            debug_msg("Operand #1 = \"%s\"\n", Scanner.to_string(expr_sem->first_operand.type));
        } else if (expr_sem->sem_state == SEMANTIC_BINARY) {
            debug_msg("Operation = \"%s\"\n", op_to_string(expr_sem->op));
            debug_msg("Operand #1 = \"%s\"\n", Scanner.to_string(expr_sem->first_operand.type));
            debug_msg("Operand #2 = \"%s\"\n", Scanner.to_string(expr_sem->second_operand.type));
        }
        debug_msg("-- EXPRESSION SEMANTICS --\n");
    }
    debug_msg("\n");

    // Semantic controls
    if (!Semantics.check_expression(expr_sem)) {
        // Return code will be set by semantics
        Semantics.dtor_expr(expr_sem);
        return false;
    }

    // Generate code here
    if (expr_sem->sem_state != SEMANTIC_DISABLED && expr_sem->sem_state != SEMANTIC_IDLE) {
        Generator.expression(expr_sem);
    }

    // Push an expression
    token_t expr_tok = { .type = expr_sem->result_type };

    Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_EXPR, &expr_tok));
    Semantics.dtor_expr(expr_sem);
    return true;
}

/**
 * @brief Check on function end.
 *
 * @param expr_type type of expression.
 * @param func_cnt count of function entries.
 * @return
 */
static bool is_function_end(expr_type_t expr_type, int func_cnt) {
    return (expr_type == EXPR_GLOBAL || expr_type == EXPR_FUNC) && func_cnt == 0;
}

/**
 * @brief Check on expression end.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @param func_cnt count of function entries.
 * @return bool.
 */
static bool is_expr_end(op_list_t first_op, op_list_t second_op, int func_cnt) {
    return  (first_op == OP_ID && Scanner.get_curr_token().type == TOKEN_ID) ||
            (first_op == OP_RPAREN && Scanner.get_curr_token().type == TOKEN_ID) ||
            (second_op == OP_COMMA && (func_cnt == 0 || func_cnt == -1));
}

/**
 * @brief Check on success parsing of the expression
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @param hard_reduce flag to reduce without comparison.
 * @return bool.
 */
static bool is_parse_success(op_list_t first_op, op_list_t second_op, bool hard_reduce) {
    return  (first_op == OP_DOLLAR && second_op == OP_DOLLAR) ||
            (first_op == OP_DOLLAR && hard_reduce);
}

/**
 * @brief Expression parsing.
 *
 * @param pfile program file to pass in to scanner.
 * @param stack stack to compare precedence and analyze an expression.
 * @param expr_type type of expression to parse.
 * @return bool.
 */
static bool parse(pfile_t *pfile, sstack_t *stack, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    bool hard_reduce = false;
    bool is_func = false;
    int func_entries = -1;
    int cmp;

    while (Scanner.get_curr_token().type != TOKEN_DEAD) {
        // Peek item from the stack
        stack_item_t *top = (stack_item_t *) Stack.peek(stack);

        debug_msg("Function entries: %d\n", func_entries);
        debug_msg("Next: \"%s\"\n", Scanner.to_string(Scanner.get_curr_token().type));

        // Pop expression if we have it on the top of the stack
        stack_item_t *expr = pop_expr(stack, &top);

        debug_msg("Top: \"%s\"\n", to_str(top));

        token_t curr_tok = Scanner.get_curr_token();
        op_list_t first_op = (top->type == ITEM_TYPE_DOLLAR) ? OP_DOLLAR : get_op(top->token);
        op_list_t second_op = get_op(curr_tok);

//        debug_msg("First op: \"%s\"\n", op_to_string(first_op));
//        debug_msg("Second op: \"%s\"\n", op_to_string(second_op));
        // Check on expression end
        if (!hard_reduce && is_expr_end(first_op, second_op, func_entries)) {
//            debug_msg("SET HARD REDUCE\n");
            hard_reduce = true;
        }

        // Check if success
        if (is_parse_success(first_op, second_op, hard_reduce) || is_function_end(expr_type, func_entries)) {
            if (expr) {
                Generator.expression_pop();
                stack_item_dtor(expr);
            } else if (expr_type == EXPR_DEFAULT) {
                Errors.set_error(ERROR_SYNTAX);
                return false;
            }
            // expr_type = {integer, number, nil, boolean, string}
            if (vector_expr_types != NULL) {
                Dynstring.append(vector_expr_types, Semantics.of_id_type(Semantics.token_to_id_type(expr->token.type)));
            }
            debug_msg("Successful parsing!\n");
            return true;
        }

        if (is_func) {
            debug_msg("UNSET IS FUNC\n");
            // TODO move this
            if (second_op == OP_LPAREN) {
                if (func_entries == -1) {
                    func_entries = 0;
                }

                func_entries++;
            }
            first_op = OP_FUNC;
            is_func = false;
        }

        // Check if identifier is a function
        if (curr_tok.type == TOKEN_ID && !hard_reduce) {
            symbol_t *symbol;

            if (Symtable.get_symbol(global_table, curr_tok.attribute.id, &symbol)) {
                if (symbol->type == ID_TYPE_func_def) {
                    debug_msg("SET IS FUNC\n");
                    second_op = OP_FUNC;
                    is_func = true;
                }
            }
        }

        // Precedence comparison
        if (!hard_reduce && !precedence_cmp(first_op, second_op, &cmp)) {
            if (expr) {
                stack_item_dtor(expr);
            }
            Errors.set_error(ERROR_SYNTAX);
            debug_msg("Precedence error!\n");
            return false;
        }

        debug_msg("\n");
        if (!hard_reduce && cmp <= 0) {
            shift(pfile, stack, expr, cmp);
        } else {
            if (!reduce(stack, expr, &func_entries)) {
                return false;
            }
        }
        debug_msg("\n");
    }

    // DEAD TOKEN
    Errors.set_error(ERROR_LEXICAL);
    return false;
}

/**
 * @brief Expression parsing initialization.
 *
 * @param pfile program file to pass in to scanner.
 * @param expr_type type of expression to parse.
 * @param prev_token previous token.
 * @return bool.
 */
static bool parse_init(pfile_t *pfile, expr_type_t expr_type, dynstring_t *vector_expr_type) {
    sstack_t *stack = Stack.ctor();

    // Push $ on stack
    stack_item_t *dollar = stack_item_ctor(ITEM_TYPE_DOLLAR, NULL);
    Stack.push(stack, dollar);

    // Parsing process
    bool parse_result = parse(pfile, stack, expr_type, vector_expr_type);

    // Delete $ from stack
    Stack.dtor(stack, stack_item_dtor);
    return parse_result;
}

/**
 * @brief
 *
 * !rule <other_expr> -> , expr <other_expr>
 *
 * @param pfile program file to pass in to scanner.
 * @return bool.
 */
static bool other_expr(pfile_t *pfile, dynstring_t *vector_expr_types) {
    debug_msg("EXPECTED: , expr <other_expr>\n");

    // DEAD TOKEN
    if (Scanner.get_curr_token().type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
        return false;
    }

    // ,
    if (Scanner.get_curr_token().type == TOKEN_COMMA) {
        Scanner.get_next_token(pfile);

        if (Scanner.get_curr_token().type == TOKEN_COMMA) {
            // Empty expression
            Errors.set_error(ERROR_SYNTAX);
            return false;
        }

        // expr
        if (parse_init(pfile, EXPR_DEFAULT, vector_expr_types)) {
            // <other_expr>
            return other_expr(pfile, vector_expr_types);
        } else {
            return false;
        }
    }

    // Otherwise
    return true;
}

/**
 * @brief
 *
 * !rule <expr_list> -> expr <other_expr>
 *
 * @param pfile program file to pass in to scanner.
 * @return bool.
 */
static bool Expr_list(pfile_t *pfile, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    debug_msg("EXPECTED: expr <other_expr>\n");

    // DEAD TOKEN
    if (Scanner.get_curr_token().type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
        return false;
    }

    // ,
    if (Scanner.get_curr_token().type == TOKEN_COMMA) {
        // Empty expression
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }

    // expr
    if (!parse_init(pfile, expr_type, vector_expr_types)) {
        return false;
    }

    // <other_expr>
    return other_expr(pfile, vector_expr_types);
}

/**
 * @brief
 *
 * !rule <id_list> -> , id <id_list> | = <expr_list>
 *
 * @param pfile program file to pass in to scanner.
 * @return bool.
 */
static bool id_list(pfile_t *pfile) {
    debug_msg("EXPECTED: , id <id_list> | = <expr_list>\n");

    // DEAD TOKEN
    if (Scanner.get_curr_token().type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
        return false;
    }

    // ,
    if (Scanner.get_curr_token().type == TOKEN_COMMA) {
        Scanner.get_next_token(pfile);
        // id
        if (Scanner.get_curr_token().type == TOKEN_ID) {
            Scanner.get_next_token(pfile);
            return id_list(pfile);
        }
    }

    // =
    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        Scanner.get_next_token(pfile);
        return Expr_list(pfile, EXPR_DEFAULT, NULL);
    }

    // Otherwise
    Errors.set_error(ERROR_SYNTAX);
    return false;
}

/**
 * @brief
 *
 * !rule <expr_stmt_next> -> = expr | <id_list>
 *
 * @param pfile program file to pass in to scanner.
 * @param prev_token previous token.
 * @return bool.
 */
static bool expr_stmt_next(pfile_t *pfile, dynstring_t *vector_expr_types) {
    debug_msg("EXPECTED: = expr | <id_list>\n");

    // DEAD TOKEN
    if (Scanner.get_curr_token().type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
        return false;
    }

    // =
    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        Scanner.get_next_token(pfile);
        return parse_init(pfile, EXPR_DEFAULT, vector_expr_types);
    }

    // <id_list>
    return id_list(pfile);
}

/**
 * @brief
 *
 * !rule <expr_stmt> -> id ( expr ) | <expr_stmt_next>
 *
 * @param pfile program file to pass in to scanner.
 * @return bool.
 */
static bool expr_stmt(pfile_t *pfile, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    debug_msg("rule: id <expr_stmt_next>\n");

    // DEAD TOKEN
    if (Scanner.get_curr_token().type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
        return false;
    }

    // id or name of a builtin function
    if (Scanner.get_curr_token().type != TOKEN_ID) {
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }

    symbol_t *symbol;
    if (Symtable.get_symbol(global_table, Scanner.get_curr_token().attribute.id, &symbol)) {
        if (symbol->type == ID_TYPE_func_def) {
            return parse_init(pfile, expr_type, vector_expr_types);
        }
    }

    // Assignment in global scope is not valid
    if (expr_type == EXPR_GLOBAL) {
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }

    // <expr_stmt_next>
    Scanner.get_next_token(pfile);
    return expr_stmt_next(pfile, vector_expr_types);
}

/**
 * @brief Expression parsing driven by a precedence table.
 *
 *
 * @param pfile program file to pass in to scanner.
 * @param inside_stmt true when the function is called from parser.
 * @return bool.
 */
static bool Parse_expression(pfile_t *pfile, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    if (expr_type == EXPR_FUNC || expr_type == EXPR_GLOBAL) {
        return expr_stmt(pfile, expr_type, vector_expr_types);
    } else {
        return parse_init(pfile, expr_type, vector_expr_types);
    }
}


/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .parse = Parse_expression,
        .parse_expr_list = Expr_list
};

#ifdef SELFTEST_expressions
#include "tests/tests.h"
#include "parser.h"

int main() {

    pfile_t *pf;
    char *req = "require \"ifj21\"\n function main()\n local foo : integer = ";
    char *end = " end\n";
    char fin[1000];
    char *s[200];

    s[0] = "a";
    s[1] = "a + b - c + d - e";
    s[2] = "a * b / c * a * c";
    s[3] = "a + b + c + d - a - b - c - d * a * b * c * d / a / b / c / d // a // b // c // d";
    s[4] = "a + c // b - d * a - d / e // f";
    s[5] = "5 + (a - 3 - (3 * a // b)) - c";
    s[6] = "1 + 39 - 23 // 23881 / 23 * 1342";
    s[7] = "a + 2 * 19228 - b";
    s[8] = "12.3 + a - 6.3e7";
    s[9] = "x // 143.11E2";
    s[10] = "21 + 231 - abc * z / 23 // 345.3 + 13e93";
    s[11] = "(a + d) ";
    s[12] = "a + (b + (c + (d + (e + f))))";
    s[13] = "a * (23 - e) / (7 * a)";
    s[14] = "a + ((a // (a - a)) * (a / (a + a - a))) + a";
    s[15] = "(a - (b * c))";
    s[16] = "#a + #b";
    s[17] = "a .. b + c * #d";
    s[18] = "#a \"goto hell\"";
    s[19] = "a + #\"hello\" - 5";
    s[20] = "naaa .. bbb + a .. b";
    s[21] = "f(a + b * c)";
    s[22] = "a * c()";
    s[23] = "f(g())";
    s[24] = "a + f(x, y(a + b)) - z * d";
    s[25] = "a(b(c(d(e, f(g, h , i()))), j(), k(l(m))))";
    s[26] = "a < b";
    s[27] = "a >= b + c - d";
    s[28] = "b / (a < b >= c) - d < e";
    s[29] = "c <= d >= a";
    s[30] = "#\"xyz\" > b";
    s[31] = "a .. c > #e";
    s[32] = "f() > g(a + b)";
    s[33] = "2 < 5 < 8 < q";
    s[34] = "a >= b <= 3";
    s[35] = "a\nd = 3";
    s[36] = "a + b / c\n d = a + b";
    s[37] = "(a)\nb=3";
    s[38] = "(a + b / c)/nb = (a + b)";
    s[39] = "(3 + a\n+ b + d)";
    s[40] = "a\na, b = 1";
    s[41] = "a\na, b = 1, 2";
    s[42] = "a + b\na, b, c = c, b, a";
    s[43] = "1\na = a";
    s[44] = "a + 1\na, b, c, d = 1, a+b, 2, (d*e)";
    s[45] = "(a + 1)\na, b, c = (a + b + c)";
    s[46] = "(a) + 1\n(a) = (a) + 2";
    s[47] = "#a\na = #a + #b";
    s[48] = "a + b .. c\nd, e, f = a .. b";
    s[49] = "1\na, b, c, d, e = a .. b + #c + d .. e * #d // 2";
    s[50] = "a\na, b = foo(), bar()";
    s[51] = "foo()\na = bar()";
    s[52] = "a() + b(a + c)\na = a + 1";
    s[53] = "foo(a)";
    s[54] = "a\na, b, c, d, e = foo(x + y)";

    unsigned num_of_tests_valid = 55; // don't forget to add 1 (bcs counting from 0)
                    // eg. number_of_tests_valid = 11 when the last assignment is s[10]

    fprintf(stdout, "\x1b[33m" "TESTS - VALID INPUT\n" "\x1b[0m");

    unsigned i = 0;
    while (i < num_of_tests_valid) {
        strcat(strcat(strcat(fin, req), s[i]), end);
        pf = Pfile.ctor(fin);
        fprintf(stdout, "[%d] ", i);
        TEST_EXPECT(Parser.analyse(pf), true, s[i]);
        Pfile.dtor(pf);
        memcpy(fin, "\0", 1000);
        i++;
    }

    s[i] = "-";
    s[i+1] = "a - - a";
    s[i+2] = "a + b + / c";
    s[i+3] = "na /";
    s[i+4] = "// a";
    s[i+5] = "a + c // b - d * a - d / e /// f";
    s[i+6] = "1 + - 23 // 23881 /* 23 * 1342";
    s[i+7] = "a a + 2 * 19228 b";
    s[i+8] = "12.3 + a 3 - 6.3e7";
    s[i+9] = "* x /* / 143.11E4";
    s[i+10] = "21 + + + 231 - abc * z z / 23 // 345.3 + 13e93";
    s[i+11] = "a + (b * d * * d)";
    s[i+12] = "a + (b + (c + (d + (e + f)))))";
    s[i+13] = "a * ((23 - e) / (7 * a)";
    s[i+14] = "a + ((a // (a - a)) * (a / (a + a - a))) + a(";
    s[i+15] = "(a - (b * c)) -";
    s[i+16] = "#a + ##b";
    s[i+17] = "a .. .. b + c * #d";
    s[i+18] = "#a hey";
    s[i+19] = "a + \"hello\" - 5";
    s[i+20] = "n * b .. + bbb";
    s[i+21] = "f(a + b * c) g()";
    s[i+22] = "f(g()";
    s[i+23] = "a * c())";
    s[i+24] = "\"a + f f(x, y(a + b)) - z * d\"";
    s[i+25] = "a <+ b";
    s[i+26] = "a >< b + d";
    s[i+27] = "a <> b";
    s[i+28] = "a >>> b";
    s[i+29] = "a + b <> c - d";
    s[i+30] = "a < b # < c";
    s[i+31] = "<";
    s[i+32] = "< a >";
    s[i+33] = "a\nd d";
    s[i+34] = "a a";
    s[i+35] = "a\nd = d = d";
    s[i+36] = "a = a\nd";
    s[i+37] = "a\na, a, a";
    s[i+38] = "a, #a, #a";
    s[i+39] = "a = 3";
    s[i+40] = "a .. b\n= 3";
    s[i+41] = "3\n3 = a";
    s[i+42] = "a\na\na = n";
    s[i+43] = "()";
    s[i+44] = "(a = b)";
    s[i+45] = "a + (b + c)\nd + 1";
    s[i+46] = "d + 1 = 4";
    s[i+47] = "(a + b * c)\nd * e = e * f";
    s[i+48] = "a\na() = b";
    s[i+49] = "a\na, b() = 1, 2";
    s[i+50] = "a() = a()";

    unsigned num_of_tests = num_of_tests_valid + 50;    // the number in the last s[i + num_of_tests]
                                // (eg. num_of_tests = 10 when the last s assignment is s[i + 10])

    fprintf(stdout, "\x1b[33m" "TESTS - INVALID INPUT\n" "\x1b[0m");
    while (i < num_of_tests + 1) {
        strcat(strcat(strcat(fin, req), s[i]), end);
        pf = Pfile.ctor(fin);
        fprintf(stdout, "[%d] ", i);
        TEST_EXPECT(Parser.analyse(pf), false, s[i]);
        Pfile.dtor(pf);
        memcpy(fin, "\0", 1000);
        i++;
    }

    return 0;
}
#endif
