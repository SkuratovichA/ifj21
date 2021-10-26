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
static const int f[19] = {10, 0, 10, 8, 8, 8, 8, 6, 6, 2, 2, 2, 2, 2, 2, 4, 9, 0, 0};

/**
 * g - represents columns.
 */
static const int g[19] = {9, 9, 0, 9, 7, 7, 7, 3, 3, 1, 1, 1, 1, 1, 1, 5, 9, 0, 0};

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
        case TOKEN_LT:      return OP_LT;
        case TOKEN_LE:      return OP_LE;
        case TOKEN_GT:      return OP_GT;
        case TOKEN_GE:      return OP_GE;
        case TOKEN_EQ:      return OP_EQ;
        case TOKEN_NE:      return OP_NE;
        case TOKEN_STRCAT:  return OP_STRCAT;
        case TOKEN_COMMA:   return OP_COMMA;
        case TOKEN_FUNC:    return OP_FUNC;
        default: return OP_DOLLAR;
    }
}

/**
 * @brief Convert operator to string
 *
 * @param op
 * @return char *.
 */
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
        case OP_LT:         return "<";
        case OP_LE:         return "<=";
        case OP_GT:         return ">";
        case OP_GE:         return ">=";
        case OP_EQ:         return "==";
        case OP_NE:         return "~=";
        case OP_STRCAT:     return "..";
        case OP_FUNC:       return "func";
        case OP_COMMA:      return ",";
        case OP_DOLLAR:     return "$";
        default: return "unrecognized operator";
    }
}

/**
 * @brief Convert stack item to string.
 * Process all items except ITEM_TYPE_TOKEN.
 *
 * @param type
 * @return char *.
 */
static char * item_to_string(item_type_t type) {
    switch(type) {
        case ITEM_TYPE_EXPR:     return "E"; break;
        case ITEM_TYPE_LT:       return "<"; break;
        case ITEM_TYPE_GT:       return ">"; break;
        case ITEM_TYPE_EQ:       return "="; break;
        case ITEM_TYPE_DOLLAR:   return "$"; break;
        default: return "unrecognized type";
    }
}

/**
 * @brief Convert stack item to string.
 * Process all items including ITEM_TYPE_TOKEN.
 *
 * @param item
 * @return char *.
 */
static char * to_str (stack_item_t * item) {
    return (item->type == ITEM_TYPE_TOKEN) ? op_to_string(get_op(item->token)) : item_to_string(item->type);
}

/**
 * @brief Allocate memory for the stack item.
 *
 * @return stack_item_t *.
 */
static stack_item_t * stack_item_alloc () {
    stack_item_t * new_item = calloc(1, sizeof(stack_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    return new_item;
}

/**
 * @brief
 *
 * Create stack item with current token,
 * if item type is equal to ITEM_TYPE_TOKEN.
 *
 * @param type
 * @return stack_item_t *.
 */
static stack_item_t * stack_item_ctor (item_type_t type) {
    stack_item_t * new_item = stack_item_alloc();
    new_item->type = type;

    if (type == ITEM_TYPE_TOKEN) {
        new_item->token = Scanner.get_curr_token();
    }

    debug_msg("Created: \"%s\"\n", to_str(new_item));
    return new_item;
}

/**
 * @brief Copy stack item. Original item is not freed.
 *
 * @param type
 * @return stack_item_t.
 */
static stack_item_t * stack_item_copy (stack_item_t * item) {
    stack_item_t * new_item = stack_item_alloc();
    new_item->type = item->type;

    if (new_item->type == ITEM_TYPE_TOKEN) {
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
static void stack_item_dtor (void * item) {
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
 * @param r_stack
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
 * !rule <operator> -> * | / | // | + | - | .. | < | <= | > | >= | == | ~=
 *
 * @param r_stack
 * @return
 */
static bool operator(sstack_t * r_stack) {
    debug_msg("EXPECTED: * | / | // | + | - | .. | < | <= | > | >= | == | ~=\n");

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
        default:
            break;
    }

    return true;
}

/**
 * !rule <other_arguments> -> , <expr> <other_arguments> | )
 *
 * @param r_stack
 * @return
 */
static bool other_arguments (sstack_t * r_stack) {
    debug_msg("EXPECTED: , <expr> <other_arguments> | )\n");

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
 * @param r_stack
 * @return
 */
static bool arguments (sstack_t * r_stack) {
    debug_msg("EXPECTED: <expr> <other_arguments> | )\n");

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
    debug_msg("EXPECTED: <expr> | # | ( | id | func\n");

    stack_item_t * item = Stack.peek(r_stack);
    if (!item) {
        return false;
    }

    if (item->type == ITEM_TYPE_EXPR) {
        Stack.pop(r_stack, stack_item_dtor);
        return operator(r_stack);
    }

    if (item->type == ITEM_TYPE_TOKEN) {
        switch(get_op(item->token)) {
            case OP_HASH:
                Stack.pop(r_stack, stack_item_dtor);
                return expression(r_stack);
            case OP_LPAREN:
                Stack.pop(r_stack, stack_item_dtor);
                return expression(r_stack) && rparen(r_stack);
            case OP_ID:
                Stack.pop(r_stack, stack_item_dtor);
                return true;
            case OP_FUNC:
                Stack.pop(r_stack, stack_item_dtor);
                return lparen(r_stack) && arguments(r_stack);
            default:
                break;
        }
    }

    return false;
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
        // Peek item from the stack
        stack_item_t * top = (stack_item_t *) Stack.peek(stack);

        debug_msg("Next: \"%s\"\n", op_to_string(get_op(Scanner.get_curr_token())));

        // Check if we have an expression on the top of the stack
        stack_item_t * expr = NULL;
        stack_item_t * tmp;
        if (top->type == ITEM_TYPE_EXPR) {
            debug_msg("Expression popped\n");
            // We need to pop the expression, because we don't want to compare non-terminals
            tmp = (stack_item_t *) Stack.peek(stack);
            expr = stack_item_copy(tmp);
            Stack.pop(stack, stack_item_dtor);
            // Update top
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

            // If first_op has a lower precedence, then push less than symbol
            if (cmp < 0) {
                debug_msg("SHIFT <\n");
                Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_LT));
            }

            // Push expression if exists
            if (expr) {
                debug_msg("Expression pushed\n");
                Stack.push(stack, expr);
            }

            // Push token from the input
            Stack.push(stack, (stack_item_t *) stack_item_ctor(ITEM_TYPE_TOKEN));

            Scanner.get_next_token(pfile);
        } else {
            debug_msg("REDUCE >\n");

            // Push expression if exists
            if (expr) {
                Stack.push(stack, expr);
                // Update top
                top = Stack.peek(stack);
            }

            // Reduce rule
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

            // Delete less than symbol
            if (top->type == ITEM_TYPE_LT) {
                Stack.pop(stack, stack_item_dtor);
            }

            // Push an expression
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

    // Push $ on stack
    stack_item_t * dollar = (stack_item_t *) stack_item_ctor(ITEM_TYPE_DOLLAR);
    Stack.push(stack, dollar);

    // Parsing process
    bool parse_result = parse(stack, pfile);

    // If we have an error due parsing process */
    if (!parse_result) {
        // Read the whole expression
        while (get_op(Scanner.get_curr_token()) != OP_DOLLAR) {
            Scanner.get_next_token(pfile);
        }
    }

    // Delete $ from stack
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

    unsigned num_of_tests_valid = 35; // don't forget to add 1 (bcs counting from 0)
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

    unsigned num_of_tests = num_of_tests_valid + 32;    // the number in the last s[i + num_of_tests]
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
