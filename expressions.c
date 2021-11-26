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
 * @brief Convert operator to string for debugging.
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

// TODO: reimplement
// how to implement a function:
//      1: Scanner.get_next_token() ONLY IN THE MACRO `EXPECTED()`

//      2: Scanner.get_curr_token() ONLY IF THERE IS A DECISION, e.g.
//          switch(Scanner.get_curr_token().type) {
//              case TOKEN_COMMA:
//                  ...
//                  break;
//
//              case TOKEN_GE: ...
//                  ...
//                  break;
//              ...
//              default:
//                  break;

//      3: If there's AT LEAST ONE HEAP allocation in the function:
//
//      static bool function() {
//          DECLARATIONS OF POINTERS AT THE TOP OF THE FUNCTION
//          dynstring_t *one = NULL; // = NULL is necessary!!!
//          dynstring_t *twy = NULL;
//          dynstring_t *three = NULL;
//
//          if (!cond) {
//              goto err:
//          }
//          if (cond_we_want_to_return_true_after) {
//              goto noerr:
//          }
//          ...
//          DONT return rule(...);
//          DO if (!rule()) {
//              goto err;
//          }
//      noerr:
//          Dynstring.dtor(one);
//          Dynstring.dtor(two);
//          Dynstring.dtor(three);
//          return true;
//      err:
//          Dynstring.dtor(one);
//          Dynstring.dtor(two);
//          Dynstring.dtor(three);
//          return false;
//     }

//      4:  DONT!
//              some_pointer_t *pointer;
//              ...
//              if (pointer) ...
//          DO:
//              some_pointer_t *pointer;
//              ...
//              if (pointer != NULL) ...

//      5: All declarations of variables are at the beginning of the function

//     6: DONT WRITE LONG FUNCTIONS, try to separate them into smaller ones.

//     7: AT MOST 3 LEVELS OF NESTING. Also, see 8 for more details;
//          DONT:
//          if (a) {
//              .. code
//              if (b) {
//                  .. code
//                  if (c) {
//                      .. code
//                  }
//              }
//          }
//          DO:
//          if (a) {
//              .. code
//              if (!func()) {
//                  goto err;
//              }
//              or
//              func();
//          }

//      8: DONT!
//          if (cond) {
//              ... many lines
//              return some;
//          }
//          return false|true;
//         DO!
//          if (!cond) {
//              return false|true;
//          }
//          ... many lines
//          return some;




// APIs:
//      1: <return_stmt> is called outside parser.
//         @param received_signature is an INITIALIZED empty vector.
//         API: bool parse_return_stmt(pfile, dynstring_t *received_signature)
//         semantic control of <return signature> x <its function signature> are performed in parser.c

//      2: <id_list> = <expr_list>
//         id()
//         API: bool parse_functino_expr(pfile);
//         parsed, checked, controlled, generated inside Expr module.

//      3: assignment
//         <expr>
//         @param received_signature is an INITIALIZED empty vector.
//         API: bool parse_assignment(pfile, dynstring_t *received_signature)
//         semantic controls are performed inside parser.c


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

    return expr_type == EXPR_FUNC || expr_type == EXPR_GLOBAL ?
           expr_stmt(expr_type) :
           parse_init(expr_type, vector_expr_types);
}


/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .parse = Parse_expression,
        .parse_expr_list = Expr_list
};
