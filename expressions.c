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

#define CHECK_DEAD_TOKEN()                                    \
    do {                                                      \
        if (Scanner.get_curr_token().type == TOKEN_DEAD) { \
            Errors.set_error(ERROR_LEXICAL);                  \
            return false;                                     \
        }                                                     \
    } while(0)

#define IF_STACK_HEAD_IS(_item, _stack, _type)  \
    (_item) = Stack.peek(_stack);               \
    if (!(_item)) {                             \
        return false;                           \
    }                                           \
    if ((_item)->type == (_type)) {

#define CHECK_STACK_TOP(_item, _stack)  \
    do {                                \
        (_item) = Stack.peek(_stack);   \
        if (!(_item)) {                 \
            return false;               \
        }                               \
    } while(0)

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
        case ITEM_TYPE_TOKEN:
            return "token";
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
        if (new_item->token.type == TOKEN_ID) {
            new_item->token.attribute.id = Dynstring.dup(new_item->token.attribute.id);
        }
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
        if (new_item->token.type == TOKEN_ID) {
            new_item->token.attribute.id = Dynstring.dup(new_item->token.attribute.id);
        }
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

    stack_item_t *s_item = (stack_item_t *) item;
    if (s_item->type == ITEM_TYPE_TOKEN || s_item->type == ITEM_TYPE_EXPR) {
        if (s_item->token.type == TOKEN_ID) {
            Dynstring.dtor(s_item->token.attribute.id);
        }
    }

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
    CHECK_STACK_TOP(item, r_stack);

    if (item->type == ITEM_TYPE_EXPR) {
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
static bool expression_f(sstack_t *r_stack, expr_semantics_t *expr_sem) {
    debug_msg("EXPECTED: expr\n");

    stack_item_t *item;
    CHECK_STACK_TOP(item, r_stack);

    if (item->type == ITEM_TYPE_EXPR) {
        Semantics.add_operand(expr_sem, item->token);
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
    CHECK_STACK_TOP(item, r_stack);

    if (item->type == ITEM_TYPE_TOKEN) {
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
    CHECK_STACK_TOP(item, r_stack);

    if (item->type == ITEM_TYPE_TOKEN) {
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
static bool other_arguments(sstack_t *r_stack, expr_semantics_t *expr_sem, int *func_entries) {
    debug_msg("rule: , expr <other_arguments> | )\n");

    // ,
    if (single_op(r_stack, OP_COMMA)) {
        // expr <other_arguments>
        return expression_f(r_stack, expr_sem) && other_arguments(r_stack, expr_sem, func_entries);
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
static bool arguments(sstack_t *r_stack, expr_semantics_t *expr_sem, int *func_entries) {
    debug_msg("EXPECTED: expr <other_arguments> | )\n");

    // expr
    if (expression_f(r_stack, expr_sem)) {
        // <other_arguments>
        return other_arguments(r_stack, expr_sem, func_entries);
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
                expr_sem->sem_state = SEMANTIC_PARENTS;
                Stack.pop(r_stack, stack_item_dtor);
                return expression(r_stack, expr_sem) && single_op(r_stack, OP_RPAREN);
                // id | id ( <arguments>
            case OP_ID:
                Semantics.add_operand(expr_sem, item->token);
                Stack.pop(r_stack, stack_item_dtor);

                if (Stack.peek(r_stack) != NULL) {
                    expr_sem->sem_state = SEMANTIC_FUNCTION;
                    return single_op(r_stack, OP_LPAREN) && arguments(r_stack, expr_sem, func_entries);
                }

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
static void shift(sstack_t *stack, stack_item_t *expr, int const cmp) {
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
    if (expr_sem->sem_state != SEMANTIC_IDLE) {
        debug_msg("-- EXPRESSION SEMANTICS --\n");
        if (expr_sem->sem_state == SEMANTIC_FUNCTION) {
            debug_msg("Operand #1 = \"%s\"\n", Scanner.to_string(expr_sem->first_operand.type));
            debug_msg("Types = \"%s\"\n", Dynstring.c_str(expr_sem->func_types));
        }
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
    if (expr_sem->sem_state != SEMANTIC_IDLE &&
        expr_sem->sem_state != SEMANTIC_PARENTS &&
        expr_sem->sem_state != SEMANTIC_FUNCTION) {
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
 * @param stack stack to compare precedence and analyze an expression.
 * @param expr_type type of expression to parse.
 * @param vector_expr_types result expression type/s.
 * @return bool.
 */
static bool parse(sstack_t *stack, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    bool hard_reduce = false;
    bool is_func = false;
    bool is_write = false;
    bool inside_func = false;
    dynstring_t *func_name = NULL;
    int func_entries = -1;
    int params_cnt = 0;
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

        // Check on expression end
        if (!hard_reduce && is_expr_end(first_op, second_op, func_entries)) {
            hard_reduce = true;
        }

        // Check if success
        if (is_parse_success(first_op, second_op, hard_reduce) || is_function_end(expr_type, func_entries)) {
            if (expr) {
                if (expr_type != EXPR_GLOBAL && expr_type != EXPR_FUNC) {
                    Generator.expression_pop();
                }
                stack_item_dtor(expr);
            } else if (expr_type == EXPR_DEFAULT) {
                Errors.set_error(ERROR_SYNTAX);
                return false;
            } else if (expr_type == EXPR_RETURN) {
                return true;
            }

            // id =
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
            inside_func = true;
            debug_msg("func starts here\n");
            Generator.func_createframe();
        }

        // Check if identifier is a function
        if (curr_tok.type == TOKEN_ID && !hard_reduce) {
            symbol_t *symbol;
            if (Symtable.get_symbol(global_table, curr_tok.attribute.id, &symbol)) {
                if (symbol->type == ID_TYPE_func_decl || symbol->type == ID_TYPE_func_def) {
                    debug_msg("SET IS FUNC\n");
                    second_op = OP_FUNC;
                    is_func = true;
                    func_name = Dynstring.dup(curr_tok.attribute.id);

                    dynstring_t *str_cmp = Dynstring.ctor("write");
                    if (Dynstring.cmp(curr_tok.attribute.id, str_cmp) == 0) {
                        is_write = true;
                    }
                    Dynstring.dtor(str_cmp);
                }
            }
        }

        if (first_op == OP_RPAREN && inside_func) {
            if (is_write) {
                is_write = false;
            }

            params_cnt = 0;
            inside_func = false;
            Generator.func_call(Dynstring.c_str(func_name));
            Dynstring.dtor(func_name);
            debug_msg("function ends here\n");
        }

        if (top->type == ITEM_TYPE_TOKEN && expr &&
            (top->token.type == TOKEN_COMMA || top->token.type == TOKEN_LPAREN)) {
            Generator.expression_pop();
            if (!is_write) {
                Generator.func_pass_param(params_cnt++);
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
            shift(stack, expr, cmp);
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
 * @param expr_type type of expression to parse.
 * @param vector_expr_types result expression type/s.
 * @return bool.
 */
static bool parse_init(expr_type_t expr_type, dynstring_t *vector_expr_type) {
    sstack_t *stack = Stack.ctor();

    // Push $ on stack
    stack_item_t *dollar = stack_item_ctor(ITEM_TYPE_DOLLAR, NULL);
    Stack.push(stack, dollar);

    // Parsing process
    bool parse_result = parse(stack, expr_type, vector_expr_type);

    // Delete $ from stack
    Stack.dtor(stack, stack_item_dtor);
    return parse_result;
}

/**
 * @brief
 *
 * !rule <other_expr> -> , expr <other_expr>
 *
 * @param vector_expr_types result expression type/s.
 * @return bool.
 */
static bool other_expr(dynstring_t *vector_expr_types) {
    debug_msg("<other_expr> ->\n");

    CHECK_DEAD_TOKEN();

    // ,
    if (Scanner.get_curr_token().type == TOKEN_COMMA) {
        Scanner.get_next_token(pfile);

        // expr
        if (parse_init(EXPR_DEFAULT, vector_expr_types)) {
            // <other_expr>
            return other_expr(vector_expr_types);
        }

        return false;
    }

    // Otherwise
    return true;
}

/**
 * @brief
 *
 * !rule <expr_list> -> expr <other_expr>
 *
 * @param pfile_ program file to pass in to scanner.
 * @param expr_type type of expression.
 * @param vector_expr_types result expression type/s.
 * @return bool.
 */
static bool Expr_list(pfile_t *pfile_, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    debug_msg("<expr_list> ->\n");
    pfile = pfile_;

    CHECK_DEAD_TOKEN();

    // expr
    if (!parse_init(expr_type, vector_expr_types)) {
        return false;
    }

    // <other_expr>
    return other_expr(vector_expr_types);
}

/**
 * @brief
 *
 * !rule <other_id> -> , id <other_id> | = <expr_list>
 *
 * @param expr_type type of expression.
 * @return bool.
 */
static bool other_id() {
    debug_msg("<other_id> ->\n");
    debug_msg("TOKEN - %s\n", Scanner.to_string(Scanner.get_curr_token().type));

    CHECK_DEAD_TOKEN();

    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        Scanner.get_next_token(pfile);
        return Expr_list(pfile, EXPR_DEFAULT, NULL);
    }

    // ,
    if (Scanner.get_curr_token().type == TOKEN_COMMA) {
        Scanner.get_next_token(pfile);

        CHECK_DEAD_TOKEN();

        if (Scanner.get_curr_token().type == TOKEN_ID) {
            // <other_id>
            Scanner.get_next_token(pfile);
            return other_id();
        }
    }

    Errors.set_error(ERROR_SYNTAX);
    return false;
}

/**
 * @brief
 *
 * !rule <id_list> -> id <other_id> | = expr
 *
 * @param expr_type type of expression.
 * @return bool.
 */
static bool id_list() {
    debug_msg("<id_list> ->\n");
    debug_msg("TOKEN - %s\n", Scanner.to_string(Scanner.get_curr_token().type));

    dynstring_t *id_name = NULL;
    GET_ID_SAFE(id_name);

    // expr_stmt was processed an ID which is not function,
    // so it is not necessary to check current token on ID here
    Scanner.get_next_token(pfile);

    // =
    if (Scanner.get_curr_token().type == TOKEN_ASSIGN) {
        Scanner.get_next_token(pfile);
        dynstring_t *received_rets = Dynstring.ctor("");
        if (!parse_init(EXPR_DEFAULT, received_rets)) {
            goto err;
        }

        // semantics of assignment
        symbol_t * symbol;
        dynstring_t *req_rets = Dynstring.ctor("");
        Symtable.get_symbol(local_table, id_name, &symbol);
        Dynstring.append(req_rets, Semantics.of_id_type(symbol->type));
        debug_msg("requested - %s, received - %s\n", Dynstring.c_str(req_rets), Dynstring.c_str(received_rets));
        if (Dynstring.cmp(req_rets, received_rets) != 0) {
            if (strcmp(Dynstring.c_str(req_rets), "f") == 0 &&
                strcmp(Dynstring.c_str(received_rets), "i") == 0) {
                // convert int -> float
                Generator.retype_expr_result();
            } else if (strcmp(Dynstring.c_str(received_rets), "n") != 0) {
                Errors.set_error(ERROR_TYPE_MISSMATCH);
                return false;
            }
        }
        Dynstring.dtor(req_rets);
        // semantics of assignment end

        Generator.var_assignment(id_name);
        Dynstring.dtor(id_name);
        return true;
    }
    Dynstring.dtor(id_name);
    // <other_id>
    return other_id();

    err:
    Dynstring.dtor(id_name);
    return false;
}

/**
 * @brief
 *
 * !rule <expr_stmt> -> id ( expr ) | <id_list>
 *
 * @param expr_type type of expression.
 * @return bool.
 */
static bool expr_stmt(expr_type_t expr_type) {
    debug_msg("<expr_stmt> ->\n");

    // DEAD TOKEN
    if (Scanner.get_curr_token().type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
        return false;
    }

    // ID or name of a builtin function
    if (Scanner.get_curr_token().type != TOKEN_ID) {
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }

    // Parse expression if ID is a function
    symbol_t *symbol;
    if (Symtable.get_symbol(global_table, Scanner.get_curr_token().attribute.id, &symbol)) {
        if (symbol->type == ID_TYPE_func_decl || symbol->type == ID_TYPE_func_def) {
            return parse_init(expr_type, NULL);
        }
    }

    // Assignment in global scope is not valid
    if (expr_type == EXPR_GLOBAL) {
        Errors.set_error(ERROR_DEFINITION);
        return false;
    }

    // <id_list>
    return id_list();
}

/**
 * @brief Expression parsing driven by a precedence table.
 *
 * @param pfile_ program file to pass in to scanner.
 * @param expr_type type of expression.
 * @param vector_expr_types result expression type/s.
 * @return bool.
 */
static bool Parse_expression(pfile_t *pfile_, expr_type_t expr_type, dynstring_t *vector_expr_types) {
    pfile = pfile_;
    if (expr_type == EXPR_FUNC || expr_type == EXPR_GLOBAL) {
        return expr_stmt(expr_type);
    } else {
        return parse_init(expr_type, vector_expr_types);
    }
}


/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .parse = Parse_expression,
        .parse_expr_list = Expr_list
};
