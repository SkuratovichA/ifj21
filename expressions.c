#include "expressions.h"
#include "scanner.h"
#include "stack.h"
#include "errors.h"
#include "stdbool.h"

/**
 * Precedence functions.
 */

/**
 * f - represents rows of the precedence table.
 */
static const int f[13] = {8, 0, 8, 6, 6, 6, 6, 4, 4, 2, 1, 0, 0};

/**
 * g - represents columns.
 */
static const int g[13] = {7, 7, 0, 7, 7, 7, 5, 5, 1, 3, 7, 0, 0};

/**
 * @brief
 *
 * Return operator from the precedence table
 * using token information.
 *
 * @param token
 * @return op_list_t.
 */
static op_list_t get_op (token_t token) {
    switch (token.type) {
        case TOKEN_STR:
        case TOKEN_NUM_F:
        case TOKEN_NUM_I:
        case TOKEN_ID:      return OP_ID;
        case TOKEN_LPAREN:  return OP_LPAREN;
        case TOKEN_RPAREN:  return OP_RPAREN;
        case TOKEN_HASH:    return OP_HASH;
        case TOKEN_MUL:     return OP_MUL;
        case TOKEN_DIV_F:   return OP_DIV_F;
        case TOKEN_DIV_I:   return OP_DIV_I;
        case TOKEN_ADD:     return OP_ADD;
        case TOKEN_SUB:     return OP_SUB;
        case TOKEN_STRCAT:  return OP_STRCAT;
        case TOKEN_COMMA:   return OP_COMMA;
        case TOKEN_FUNC:    return OP_FUNC;
        default: return OP_DOLLAR;
    }
}

static char * op_to_string(op_list_t op) {
    switch(op) {
        case OP_ID:         return "id";
        case OP_LPAREN:     return "(";
        case OP_RPAREN:     return ")";
        case OP_HASH:       return "#";
        case OP_MUL:        return "*";
        case OP_DIV_I:      return "//";
        case OP_DIV_F:      return "/";
        case OP_ADD:        return "+";
        case OP_SUB:        return "-";
        case OP_STRCAT:     return "..";
        case OP_FUNC:       return "func";
        case OP_COMMA:      return ",";
        case OP_DOLLAR:     return "$";
        default: return "unrecognized operator";
    }
}

static char * item_to_string(item_type_t type) {
    switch(type) {
        case ITEM_TYPE_EXPR:     return "expr"; break;
        case ITEM_TYPE_LT:       return "<"; break;
        case ITEM_TYPE_GT:       return ">"; break;
        case ITEM_TYPE_EQ:       return "="; break;
        case ITEM_TYPE_DOLLAR:   return "$"; break;
        default: return "unrecognized type";
    }
}

/**
 * @brief Precedence functions error handling.
 * Check existence of relation between two operators.
 *
 * @param first_op first operator.
 * @param second_op second operator.
 * @return bool.
 */
static bool precedence_check (op_list_t first_op, op_list_t second_op) {
    if (first_op == OP_ID || first_op == OP_RPAREN) {
        if (second_op == OP_ID || second_op == OP_LPAREN || second_op == OP_FUNC) {
            return false;
        }
    }

    if (first_op == OP_LPAREN || first_op == OP_COMMA) {
        if (second_op == OP_DOLLAR) {
            return false;
        }
    }

    if (first_op == OP_FUNC) {
        if (second_op != OP_LPAREN && second_op != OP_COMMA) {
            return false;
        }
    }

    if (first_op == OP_DOLLAR) {
        if (second_op == OP_LPAREN || second_op == OP_COMMA || second_op == OP_DOLLAR) {
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
static bool precedence_cmp (op_list_t first_op, op_list_t second_op, int *cmp) {
    if (precedence_check(first_op, second_op)) {
        *cmp = f[first_op] - g[second_op];
        return true;
    }

    return false;
}

static char * to_str (stack_item_t * item) {
    return (item->type == ITEM_TYPE_TOKEN) ? op_to_string(get_op(item->token)) : item_to_string(item->type);
}

static stack_item_t * stack_item_ctor (item_type_t type) {
    stack_item_t * new_item = calloc(1, sizeof(stack_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->type = type;

    if (type == ITEM_TYPE_TOKEN) {
        new_item->token = Scanner.get_curr_token();
    }

    debug_msg("Created: \"%s\"\n", to_str(new_item));
    return new_item;
}

static void stack_item_dtor (void * item) {
    debug_msg("Deleted: \"%s\"\n", to_str(item));
    free(item);
}

static stack_item_t * stack_item_copy (stack_item_t * item) {
    debug_msg("-- COPY --\n");
    stack_item_t * new_item = stack_item_ctor(item->type);

    if (new_item->type == ITEM_TYPE_TOKEN) {
        new_item->token = item->token;
    }

    debug_msg("Copied: \"%s\"\n", to_str(item));
    debug_msg("-- COPY --\n");

    return new_item;
}

static bool comma (sstack_t * r_stack) {
    debug_msg("EXPECTED: ,\n");
    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }

    if (item->type != ITEM_TYPE_TOKEN) {
        return false;
    }

    if (get_op(item->token) != OP_COMMA) {
        return false;
    }

    Stack.pop(r_stack, stack_item_dtor);
    return true;
}

static bool lparen (sstack_t * r_stack) {
    debug_msg("EXPECTED: (\n");
    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }

    if (item->type != ITEM_TYPE_TOKEN) {
        return false;
    }

    if (get_op(item->token) != OP_LPAREN) {
        return false;
    }

    Stack.pop(r_stack, stack_item_dtor);
    return true;
}

static bool rparen (sstack_t * r_stack) {
    debug_msg("EXPECTED: )\n");
    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }

    if (item->type != ITEM_TYPE_TOKEN) {
        return false;
    }

    if (get_op(item->token) != OP_RPAREN) {
        return false;
    }

    Stack.pop(r_stack, stack_item_dtor);
    return true;
}

/**
 * @param list
 * @return
 */
static bool expression (sstack_t * r_stack) {
    debug_msg("EXPECTED: expression\n");
    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }

    if (item->type != ITEM_TYPE_EXPR) {
        return false;
    }

    Stack.pop(r_stack, stack_item_dtor);
    return true;
}

/**
 *
 * !rule <operator> -> * | / | // | + | - | ..
 *
 * @param list
 * @return
 */
static bool operator(sstack_t * r_stack) {
    debug_msg("EXPECTED: operator\n");
    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }

    if (item->type != ITEM_TYPE_TOKEN) {
        return false;
    }

    switch (get_op(item->token)) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV_I:
        case OP_DIV_F:
        case OP_STRCAT:
            Stack.pop(r_stack, stack_item_dtor);
            return expression(r_stack);
    }

    return true;
}

/**
 * !rule <other_arguments> -> , <expr> <other_arguments> | )
 *
 * @param list
 * @return
 */
static bool other_arguments (sstack_t * r_stack) {
    debug_msg("EXPECTED: , <expr> <other_params> )\n");

    if (rparen(r_stack)) {
        return true;
    }

    if (!comma(r_stack)) {
        return false;
    }

    if (!expression(r_stack)) {
        return false;
    }

    return other_arguments(r_stack);
}

/**
 * !rule <arguments> -> <expr> <other_arguments> | )
 *
 * @param list
 * @return
 */
static bool arguments (sstack_t * r_stack) {
    debug_msg("EXPECTED: <expr> <other_params> | )\n");

    if (expression(r_stack)) {
        return other_arguments(r_stack);
    } else {
        return rparen(r_stack);
    }
}

/**
 * @brief
 *
 * !rule <expr> -> <expr> <operator> <expr>
 * !rule <expr> -> # <expr>
 * !rule <expr> -> ( <expr> )
 * !rule <expr> -> id
 * !rule <expr> -> func ( <arguments>
 *
 * @param r_stack
 * @return
 */
static bool reduce(sstack_t * r_stack) {
    debug_msg("EXPECTED: expression | ( | id\n");
    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }
    switch(item->type) {
        case ITEM_TYPE_EXPR:
            Stack.pop(r_stack, stack_item_dtor);
            return operator(r_stack);
        case ITEM_TYPE_TOKEN:
            switch(get_op(item->token)) {
                case OP_HASH:
                    Stack.pop(r_stack, stack_item_dtor);
                    return expression(r_stack);
                case OP_LPAREN:
                    Stack.pop(r_stack, stack_item_dtor);
                    if (!expression(r_stack)) {
                        return false;
                    }
                    return rparen(r_stack);
                case OP_ID:
                    Stack.pop(r_stack, stack_item_dtor);
                    return true;
                case OP_FUNC:
                    Stack.pop(r_stack, stack_item_dtor);
                    if (!lparen(r_stack)) {
                        return false;
                    }
                    return arguments(r_stack);
            }
            break;
        default: return false;
    }
}

/**
 * @brief Expression parsing.
 *
 * @param stack stack to compare precedence and process an expression.
 * @param pfile program file to pass in to scanner.
 * @return bool.
 */
static bool parse(sstack_t * stack, pfile_t * pfile) {
    while (true) {
        /* Peek item from the stack */
        stack_item_t * top = (stack_item_t *) Stack.peek(stack);

        debug_msg("Next: \"%s\"\n", op_to_string(get_op(Scanner.get_curr_token())));

        /* Check if we have an expression on stack top */
        stack_item_t * expr = NULL;
        stack_item_t * tmp;
        if (top->type == ITEM_TYPE_EXPR) {
            debug_msg("Expression popped\n");
            /* We need to pop the expression, because we don't want to compare non-terminals */
            tmp = (stack_item_t *) Stack.peek(stack);
            expr = stack_item_copy(tmp);
            Stack.pop(stack, stack_item_dtor);
            /* Update top */
            top = (stack_item_t *) Stack.peek(stack);
        }

        debug_msg("Top: \"%s\"\n", to_str(top));

        int cmp;
        op_list_t first_op = (top->type == ITEM_TYPE_DOLLAR) ? OP_DOLLAR : get_op(top->token);
        op_list_t second_op = get_op(Scanner.get_curr_token());

        if (first_op == OP_DOLLAR && second_op == OP_DOLLAR) {
            if (expr) {
                stack_item_dtor(expr);
            }
            debug_msg("Successful parsing!\n");
            break;
        }

        if (!precedence_cmp(first_op, second_op, &cmp)) {
            if (first_op == OP_ID && second_op == OP_LPAREN) {
                top->token.type = TOKEN_FUNC;
                cmp = 0;
            } else {
                if (expr) {
                    stack_item_dtor(expr);
                }
                debug_msg("Precedence error!\n");
                return false;
            }
        }

        debug_msg("\n");
        if (cmp <= 0) {
            debug_msg("SHIFT < | SHIFT =\n");

            /* If first_op has a lower precedence, then push less than symbol */
            if (cmp < 0) {
                debug_msg("SHIFT <\n");
                Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_LT));
            }

            /* Push expression if exists */
            if (expr) {
                debug_msg("Expression pushed\n");
                Stack.push(stack, expr);
            }

            /* Push token from the input */
            Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_TOKEN));

            Scanner.get_next_token(pfile);
        } else {
            debug_msg("REDUCE >\n");

            /* Push expression if exists */
            if (expr) {
                Stack.push(stack, expr);
                /* Update top */
                top = Stack.peek(stack);
            }

            /* Reduce rule */
            sstack_t * r_stack = Stack.ctor();
            while (top->type != ITEM_TYPE_LT && top->type != ITEM_TYPE_DOLLAR) {
                Stack.push(r_stack, (stack_item_t *) stack_item_copy(top));
                Stack.pop(stack, stack_item_dtor);
                top = Stack.peek(stack);
            }

            if (!reduce(r_stack) || Stack.peek(r_stack) != NULL) {
                debug_msg("Reduction error!\n");
                Stack.dtor(r_stack, stack_item_dtor);
                return false;
            }

            Stack.dtor(r_stack, stack_item_dtor);

            /* Delete less than symbol */
            if (top->type == ITEM_TYPE_LT) {
                Stack.pop(stack, stack_item_dtor);
            }

            /* Push an expression */
            Stack.push(stack, stack_item_ctor(ITEM_TYPE_EXPR));
        }
        debug_msg("\n");
    }

    return true;
}

/**
 * @brief Expression parsing driven by a precedence table.
 *
 * @param pifle program file to pass in to scanner.
 * @return bool.
 */
static bool Parse_expression(pfile_t *pfile) {
    sstack_t * stack = Stack.ctor();

    /* Push $ on stack */
    stack_item_t * dollar = (stack_item_t *) stack_item_ctor(ITEM_TYPE_DOLLAR);
    Stack.push(stack, dollar);

    /* Parsing process */
    bool parse_result = parse(stack, pfile);

    /* If we have an error due parsing process */
    if (!parse_result) {
        /* Read the whole expression */
        while (get_op(Scanner.get_curr_token()) != OP_DOLLAR) {
            Scanner.get_next_token(pfile);
        }
    }

    /* Delete $ from stack */
    Stack.dtor(stack, stack_item_dtor);
    return parse_result;
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .parse = Parse_expression,
};

#ifdef SELFTEST_expressions
#include "tests/tests.h"
#include "parser.h"

int main() {
    // create test inputs.

    pfile_t *pf1  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a end\n");
    pfile_t *pf2  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + b - c + d - e end\n");
    pfile_t *pf3  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a * b / c * a * c end\n");
    pfile_t *pf4  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + b + c + d - a - b - c - d * a * b * c * d / a / b / c / d // a // b // c // d end\n");
    pfile_t *pf5  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + c // b - d * a - d / e // f end\n");

    pfile_t *pf6  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = 1 + 39 - 23 // 23881 / 23 * 1342 end\n");
    pfile_t *pf7  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + 2 * 19228 - b end\n");
    pfile_t *pf8  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = 12.3 + a - 6.3e7 end\n");
    pfile_t *pf9  = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = x // 143.11E2 end\n");
    pfile_t *pf10 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = 21 + 231 - abc * z / 23 // 345.3 + 13e93 end\n");

    pfile_t *pf11 = Pfile.ctor("require \"ifj21\"\n"
                               "function main()\n"
                               "    local foo : integer = (a + d)\n"
                               "end\n");
    pfile_t *pf12 = Pfile.ctor("require \"ifj21\"\n"
                                "function main()\n"
                                "    local foo : integer = a + (b + (c + (d + (e + f))))\n"
                                 "end\n");
    pfile_t *pf13 = Pfile.ctor("require \"ifj21\"\n"
                                "function main()\n"
                                "    local foo : integer = a * (23 - e) / (7 * a)\n"
                                 "end\n");
    pfile_t *pf14 = Pfile.ctor("require \"ifj21\"\n"
                                "function main()\n"
                                "    local foo : integer = a + ((a // (a - a)) * (a / (a + a - a))) + a\n"
                                 "end\n");
    pfile_t *pf15 = Pfile.ctor("require \"ifj21\"\n"
                                "function main()\n"
                                "    local foo : integer = (a - (b * c))\n"
                                 "end\n");

    pfile_t *pf16 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = #a + #b end\n");
    pfile_t *pf17 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a .. b + c * #d end\n");
    pfile_t *pf18 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = #a \"goto hell\" end\n");
    pfile_t *pf19 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + #\"hello\" - 5 end\n");
    pfile_t *pf20 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = naaa .. bbb + a .. b end\n");

    pfile_t *pf21 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = f(a + b * c) end\n");
    pfile_t *pf22 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a * c() end\n");
    pfile_t *pf23 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = f(g()) end\n");
    pfile_t *pf24 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + f(x, y(a + b)) - z * d end\n");
    pfile_t *pf25 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a(b(c(d(e, f(g, h , i()))), j(), k(l(m)))) end\n");

    // tests.
    printf("\x1b[33m" "TESTS - VALID INPUT\n" "\x1b[0m");

    printf("\x1b[33m" "-------------------\n" "\x1b[0m");
    fprintf(stdout, "Test 1 - id, operators +, -, *, /, //\n");
    TEST_EXPECT(Parser.analyse(pf1), true, "[1] \"a\"");
    TEST_EXPECT(Parser.analyse(pf2), true, "[2] \"a + b - c + d - e\"");
    TEST_EXPECT(Parser.analyse(pf3), true, "[3] \"a * b / c * a * c\"");
    TEST_EXPECT(Parser.analyse(pf5), true, "[4] \"a + b + c + d - a - b - c - d * a * b * c * d / a / b / c / d // a // b // c // d\"");
    TEST_EXPECT(Parser.analyse(pf4), true, "[5] \"a + c // b - d * a - d / e // f\"");

    printf("Test 2 - constants, id, operators +, -, *, /, //\n");
    TEST_EXPECT(Parser.analyse(pf6), true, "[6] \"1 + 39 - 23 // 23881 / 23 * 1342\"");
    TEST_EXPECT(Parser.analyse(pf7), true, "[7] \"a + 2 * 19228 - b\"");
    TEST_EXPECT(Parser.analyse(pf8), true, "[8] \"12.3 + a - 6.3e7\"");
    TEST_EXPECT(Parser.analyse(pf9), true, "[9] \"x // 143.11E\"");
    TEST_EXPECT(Parser.analyse(pf10), true, "[10] \"21 + 231 - abc * z / 23 // 345.3 + 13e93\"");

    printf("Test 3 - id, paren, constants, operators +, -, *, /, //\n");
    TEST_EXPECT(Parser.analyse(pf11), true, "[11] \"(a + d)\"");
    TEST_EXPECT(Parser.analyse(pf12), true, "[12] \"a + (b + (c + (d + (e + f))))\"");
    TEST_EXPECT(Parser.analyse(pf13), true, "[13] \"a * (23 - e) / (7 * a)\"");
    TEST_EXPECT(Parser.analyse(pf14), true, "[14] (\"a + ((a // (a - a)) * (a / (a + a - a))) + a\")");
    TEST_EXPECT(Parser.analyse(pf15), true, "[15] \"(a - (b * c))\"");

    printf("Test 4 - id, constants, operators +, -, *, /, //, #, ..\n");
    TEST_EXPECT(Parser.analyse(pf16), true, "[16] \"#a + #b\"");
    TEST_EXPECT(Parser.analyse(pf17), true, "[17] \"a .. b + c * #d\"");
    TEST_EXPECT(Parser.analyse(pf18), true, "[18] \"#a \\\"goto hell\\\"\"");
    TEST_EXPECT(Parser.analyse(pf19), true, "[19] \"a + #\\\"hello\\\" - 5\"");
    TEST_EXPECT(Parser.analyse(pf20), true, "[20] \"naaa .. bbb + a .. b\"");

    printf("Test 5 - functions, id, constants, operators +, -, *, /, //, #, ..\n");
    TEST_EXPECT(Parser.analyse(pf21), true, "[21] \"f(a + b * c)\"");
    TEST_EXPECT(Parser.analyse(pf22), true, "[22] \"a * c()\"");
    TEST_EXPECT(Parser.analyse(pf23), true, "[23] \"f(g())\"");
    TEST_EXPECT(Parser.analyse(pf24), true, "[24] \"a + f(x, y(a + b)) - z * d\"");
    TEST_EXPECT(Parser.analyse(pf25), true, "[25] \"a(b(c(d(e, f(g, h , i()))), j(), k(l(m))))\"");

    // destructors
    Pfile.dtor(pf1);
    Pfile.dtor(pf2);
    Pfile.dtor(pf3);
    Pfile.dtor(pf4);
    Pfile.dtor(pf5);
    Pfile.dtor(pf6);
    Pfile.dtor(pf7);
    Pfile.dtor(pf8);
    Pfile.dtor(pf9);
    Pfile.dtor(pf10);
    Pfile.dtor(pf11);
    Pfile.dtor(pf12);
    Pfile.dtor(pf13);
    Pfile.dtor(pf14);
    Pfile.dtor(pf15);
    Pfile.dtor(pf16);
    Pfile.dtor(pf17);
    Pfile.dtor(pf18);
    Pfile.dtor(pf19);
    Pfile.dtor(pf20);
    Pfile.dtor(pf21);
    Pfile.dtor(pf22);
    Pfile.dtor(pf23);
    Pfile.dtor(pf24);
    Pfile.dtor(pf25);

    // tests - input invalid **************************************************************

    // create test inputs.
    pfile_t *pf26 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a - - a end\n");
    pfile_t *pf27 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + b + / c end\n");
    pfile_t *pf28 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = na / end\n");
    pfile_t *pf29 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = // a end\n");
    pfile_t *pf30 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + c // b - d * a - d / e /// f end\n");

    pfile_t *pf31 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = 1 + - 23 // 23881 /* 23 * 1342 end\n");
    pfile_t *pf32 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a a + 2 * 19228 b end\n");
    pfile_t *pf33 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = 12.3 + a 3 - 6.3e7 end\n");
    pfile_t *pf34 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = * x /* / 143.11E4 end\n");
    pfile_t *pf35 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = 21 + + + 231 - abc * z z / 23 // 345.3 + 13e93 end\n");

    pfile_t *pf36 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + (b * d * * d) end\n");
    pfile_t *pf37 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + (b + (c + (d + (e + f))))) end\n");
    pfile_t *pf38 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a * ((23 - e) / (7 * a) end\n");
    pfile_t *pf39 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + ((a // (a - a)) * (a / (a + a - a))) + a( end\n");
    pfile_t *pf40 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = (a - (b * c)) -  end\n");

    pfile_t *pf41 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = #a + ##b end\n");
    pfile_t *pf42 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a ... b + c * #d end\n");
    pfile_t *pf43 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = #a hey end\n");
    pfile_t *pf44 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a + \"hello\" - 5 end\n");
    pfile_t *pf45 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = n * b .. + bbb end\n");

    pfile_t *pf46 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = f(a + b * c) g() end\n");;
    pfile_t *pf47 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = f(g() end\n");
    pfile_t *pf48 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a * c()) end\n");
    pfile_t *pf49 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = \"a + f f(x, y(a + b)) - z * d\" end\n");
    pfile_t *pf50 = Pfile.ctor("require \"ifj21\"\n function main()\n local foo : integer = a(b(c(d(e, f(g, h , i()))), j(), k(l(m l)))) end\n");

    // tests.
    printf("\x1b[33m" "TESTS - INVALID INPUT\n" "\x1b[0m");
    printf("\x1b[33m" "---------------------\n" "\x1b[0m");
    printf("Test 6 - ids, operators +, -, *, /, //\n");
    TEST_EXPECT(Parser.analyse(pf26), false, "[26] \"a - - b\"");
    TEST_EXPECT(Parser.analyse(pf27), false, "[27] \"a + b + / c\"");
    TEST_EXPECT(Parser.analyse(pf28), false, "[28] \"a /\"");
    TEST_EXPECT(Parser.analyse(pf29), false, "[29] \"// a\"");
    TEST_EXPECT(Parser.analyse(pf30), false, "[30] \"a + c // b - d * a - d / e /// f\"");

    printf("Test 7 - constants, id, operators +, -, *, /, //\n");
    TEST_EXPECT(Parser.analyse(pf31), false, "[31] \"1 + - 23 // 23881 /* 23 * 1342\"");
    TEST_EXPECT(Parser.analyse(pf32), false, "[32] \"a a + 2 * 19228 b\"");
    TEST_EXPECT(Parser.analyse(pf33), false, "[33] \"12.3 + a 3 - 6.3e7\"");
    TEST_EXPECT(Parser.analyse(pf34), false, "[34] \"* x /* / 143.11E\"");
    TEST_EXPECT(Parser.analyse(pf35), false, "[35] \"21 + + + 231 - abc * z z / 23 // 345.3 + 13e93\"");

    printf("Test 8 - id, paren, constants, operators +, -, *, /, //\n");
    TEST_EXPECT(Parser.analyse(pf36), false, "[36] \"a + (b * d * * d)\"");
    TEST_EXPECT(Parser.analyse(pf37), false, "[37] \"a + (b + (c + (d + (e + f)))))\"");
    TEST_EXPECT(Parser.analyse(pf38), false, "[38] \"a * ((23 - e) / (7 * a)\"");
    TEST_EXPECT(Parser.analyse(pf39), false, "[39] \"a + ((a // (a - a)) * (a / (a + a - a))) + a(\"");
    TEST_EXPECT(Parser.analyse(pf40), false, "[40] \"(a - (b * c)) - \"");

    printf("Test 9 - id, constants, operators +, -, *, /, //, #, ..\n");
    TEST_EXPECT(Parser.analyse(pf41), false, "[41] \"#a + ##b\"");
    //TEST_EXPECT(Parser.analyse(pf42), false, "[42] \"a ... b + c * #d\"");
    TEST_EXPECT(Parser.analyse(pf43), false, "[43] \"#a hey\"");
    TEST_EXPECT(Parser.analyse(pf44), false, "[44] \"a + \\\"hello\\\" - 5\"");
    TEST_EXPECT(Parser.analyse(pf45), false, "[45] \"n * b .. + bbb\"");

    printf("Test 10 - functions, id, constants, operators +, -, *, /, //, #, ..\n");
    TEST_EXPECT(Parser.analyse(pf46), false, "[46] \"f(a + b * c) g()\"");
    TEST_EXPECT(Parser.analyse(pf47), false, "[47] \"f(g()\"");
    TEST_EXPECT(Parser.analyse(pf48), false, "[48] \"a * c())\"");
    TEST_EXPECT(Parser.analyse(pf49), false, "[49] \"\\\"a + f f(x, y(a + b)) - z * d\\\"\"");
    TEST_EXPECT(Parser.analyse(pf50), false, "[50] \"a(b(c(d(e, f(g, h , i()))), j(), k(l(m l))))\"");

    // destructors
    Pfile.dtor(pf1);
    Pfile.dtor(pf2);
    Pfile.dtor(pf3);
    Pfile.dtor(pf4);
    Pfile.dtor(pf5);
    Pfile.dtor(pf6);
    Pfile.dtor(pf7);
    Pfile.dtor(pf8);
    Pfile.dtor(pf9);
    Pfile.dtor(pf10);
    Pfile.dtor(pf11);
    Pfile.dtor(pf12);
    Pfile.dtor(pf13);
    Pfile.dtor(pf14);
    Pfile.dtor(pf15);
    Pfile.dtor(pf16);
    Pfile.dtor(pf17);
    Pfile.dtor(pf18);
    Pfile.dtor(pf19);
    Pfile.dtor(pf20);
    Pfile.dtor(pf21);
    Pfile.dtor(pf22);
    Pfile.dtor(pf23);
    Pfile.dtor(pf24);
    Pfile.dtor(pf25);

    return 0;
}
#endif
