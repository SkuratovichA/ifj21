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

//      5: All declarations of variables are at the beginning of the function if there's a possibility to.

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


/**
 * @brief Expression in return statement.
 *        semantic control of <return signature> x <its function signature> are performed in parser.c
 *
 * @param pfile_
 * @param received_signature is an initialized empty vector.
 * @return true if successive parsing performed.
 */
static bool Return_expressions(pfile_t *pfile_, dynstring_t *received_signature) {
    pfile = pfile_;
    assert(false);
}

/**
 * @brief Assignment expression after = in the local assignment
 *        semantic controls are performed inside parser.c
 * @param received_signature is an initialized empty vector.
 * @return true if successive parsing performed.
 */
static bool Assignment_expression(pfile_t *pfile_, dynstring_t *received_signature) {
    pfile = pfile_;
    assert(false);
}

/**
 * @brief Function calling in the global scope. `id( ...`
 *
 * @param pfile_
 * @return true if successive parsing and semantic analysis of expressions performed.
 */
static bool Global_expression(pfile_t *pfile_) {
    pfile = pfile_;
    assert(false);
}

/**
 * @brief Function calling or assignments in the local scope.
 *
 * @param pfile_
 * @return true if successive parsing and semantic analysis of expressions performed.
 */
static bool Function_expression(pfile_t *pfile_) {
    pfile = pfile_;
    assert(false);
}

/**
 * Functions are in struct so we can use them in different files.
 */
const struct expr_interface_t Expr = {
        .function_expression = Function_expression,
        .global_expression = Global_expression,
        .assignment_expression = Assignment_expression,
        .return_expressions = Return_expressions,
};
