/**
 * @file parser.c
 *
 * @brief The file contains implementation of tre top-down parser.
 *        CFG rules are LL(1). Parser was implemented using recursive descent method.
 *        Also, code generation is integrated directly in to the parser.
 *
 *        ! rule  <rule> -> <derivations> is a part of documentation automatically
 *        generated by script i've written.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */

#include "parser.h"
#include "scanner.h"
#include "errors.h"

#include "symstack.h"
#include "expressions.h"
#include "code_generator.h"


static pfile_t *pfile;

/** Print an error ife terminal symbol is unexpected.
 *
 * @param a expected.
 * @param b given.
 */
static void print_error_unexpected_token(const char *a, const char *b) {
    fprintf(stderr, "line %zu, character %zu\n", Scanner.get_line(), Scanner.get_charpos());
    fprintf(stderr, "[error](syntax): Expected '%s', got '%s' instead\n", a, b);
}

#define GET_ID_SAFE(_idname)                                                                    \
    do {                                                                                       \
        if (Scanner.get_curr_token().type == TOKEN_ID) {                                       \
            _idname = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));  \
        }                                                                                      \
    } while (0)

#define CHECK_EXPR_SIGNATURES(accepted, received, errtype)                     \
    do {                                                                      \
        if (!Semantics.check_signatures_compatibility(accepted, received, errtype)) { \
            goto err;                                                         \
        }                                                                     \
    } while(0)

/** If there's a mismatch in number/type of parameters, then return false.
 */
#define SEMANTIC_CHECK_FUNCTION_SIGNATURES(sym)                          \
    do {                                                                \
        if (!Semantics.check_signatures(symbol->function_semantics)) {  \
            Errors.set_error(ERROR_FUNCTION_SEMANTICS);                 \
            return false;                                               \
        }                                                               \
    } while(0)

/** Push a new symtable on the stack,
 *  if _id_fun_name != NULL, put it on the stack.
 *  There is a need to store a function id to perform code generation magic.
 *  Set a local table to the newly puched table.
 */
#define SYMSTACK_PUSH(_scope_type, _id_fun_name)                  \
    do {                                                         \
        char *_str = NULL;                                       \
        local_table = Symtable.ctor();                           \
        if ((_id_fun_name) != NULL) {                            \
            _str = Dynstring.c_str(_id_fun_name);                \
        }                                                        \
        Symstack.push(symstack, local_table, _scope_type, _str); \
    } while (0)

/** Pop an item prom the stack. Change local table, too.
 */
#define SYMSTACK_POP()                          \
     do {                                      \
        Symstack.pop(symstack);                \
        local_table = Symstack.top(symstack);  \
     } while(0)

/** Return an error(syntax)
 */
#define error_unexpected_token(a)                               \
    do {                                                       \
        Errors.set_error(ERROR_SYNTAX);                        \
        print_error_unexpected_token(Scanner.to_string(a),     \
        Scanner.to_string(Scanner.get_curr_token().type));     \
        goto err;                                              \
    } while (0)

/** Set an error code and return an error if there's a declaration error.
 */
#define error_multiple_declaration(a)                                 \
    do {                                                             \
        Errors.set_error(ERROR_DEFINITION);                          \
        fprintf(stderr, "line %zu, character %zu\n",                 \
           Scanner.get_line(), Scanner.get_charpos());               \
        fprintf(stderr, "[error](semantic): variable with name '%s'" \
           " has already been declared!\n", Dynstring.c_str(a));     \
        return false;                                                \
    } while (0)

/** Macro expecting a non terminal from the scanner.
 */
#define EXPECTED(p)                                                            \
    do {                                                                      \
        token_t tok__ = Scanner.get_curr_token();                             \
        if (tok__.type == (p)) {                                              \
            if (tok__.type == TOKEN_ID || tok__.type == TOKEN_STR) {          \
                debug_msg("\t%s = { '%s' }\n", Scanner.to_string(tok__.type), \
                   Dynstring.c_str(tok__.attribute.id));                      \
            } else {                                                          \
                debug_msg("\t%s\n", Scanner.to_string(tok__.type));           \
            }                                                                 \
            if (TOKEN_DEAD == Scanner.get_next_token(pfile).type) {            \
                Errors.set_error(ERROR_LEXICAL);                              \
                goto err;                                                     \
            }                                                                 \
        } else {                                                              \
            error_unexpected_token((p));                                      \
        }                                                                     \
    } while(0)

/** if there's a condition of type '<a> -> b | c', you have to add
 * EXPECTED_OPT(b) in the function.
 */
#define EXPECTED_OPT(toktype)                                                         \
    do {                                                                             \
        if (Scanner.get_curr_token().type == (toktype)) {                            \
            /* OR use Scanner.get_next_token, but let it be clear in case we want */ \
            /* to change Scanner.get_next_token or add debug messages. */            \
            EXPECTED((toktype));                                                     \
            return true;                                                             \
        }                                                                            \
    } while(0)

/** Check semantics(for functions, variables, but not for expressions).
 */
#define SEMANTICS_SYMTABLE_CHECK_AND_PUT(name, type)                     \
    do {                                                                \
        dynstring_t *_name = name;                                      \
        symbol_t *_dummy_symbol = NULL;                                 \
        /* if name is already defined in the local scope */              \
        if (Symtable.get_symbol(local_table, _name, &_dummy_symbol)) {  \
            error_multiple_declaration(_name);                          \
            goto err;                                                   \
        }                                                               \
        /* if there exists a function with the same name */             \
        if (Symtable.get_symbol(global_table, _name, &_dummy_symbol)) { \
            error_multiple_declaration(_name);                          \
            goto err;                                                   \
        }                                                               \
        Symstack.put_symbol(symstack, _name, type);                     \
    } while (0)

#define PARSE_EXPR(expr_type, received_signature)                      \
    do {                                                              \
        if (!Expr.parse(pfile, expr_type, received_signature)) {       \
            debug_msg("\n");                                          \
            debug_msg_s("\t\t[error] Expression parsing failed.\n");  \
            goto err;                                                 \
        }                                                             \
    } while(0)

#define PARSE_EXPR_LIST(expr_type, received_signature)                      \
    do {                                                                   \
        if (!Expr.parse_expr_list(pfile, expr_type, received_signature)) {  \
            debug_msg("\n");                                               \
            debug_msg_s("\t\t[error] Expression parsing failed.\n");       \
            goto err;                                                      \
        }                                                                  \
    } while(0)


static void increase_nesting() {
#ifdef DEBUG
    FILE *fp = fopen("nesting.out", "a+");
    for (int i = 0; i < (int) nested_cycle_level; i++) {
        fprintf(fp, "\t");
    }
    fprintf(fp, "cycle\n");
    fclose(fp);
#endif
    nested_cycle_level++;
}

static void decrease_nesting() {
    nested_cycle_level--;
#ifdef DEBUG
    FILE *fp = fopen("nesting.out", "a+");
    for (int i = 0; i < nested_cycle_level; i++) {
        fprintf(fp, "\t");
    }
    fprintf(fp, "end\n");
    fclose(fp);
#endif
}

static bool cond_stmt();

static bool fun_body();

static bool fun_stmt();

static bool break_() {
    EXPECTED(KEYWORD_break);
    if (nested_cycle_level == 0) {
        Errors.set_error(ERROR_SYNTAX);
        goto err;
    }

    return true;
    err:
    return false;
}

/** Else statement.
 *
 * !rule <else_stmt> -> else <fun_body>
 * @return
 */
static bool else_stmt() {
    EXPECTED(KEYWORD_else);

    // pop an existent symtable, because we ara in the if statement(scope).
    SYMSTACK_POP();
    // also, we need to create a new symtable for 'else' scope.
    SYMSTACK_PUSH(SCOPE_TYPE_condition, NULL);
    // generate start of else block
    instructions.cond_cnt++;
    Generator.cond_else(instructions.outer_cond_id, instructions.cond_cnt);
    // <fun_body>
    if (!fun_body()) {
        goto err;
    }
    SYMSTACK_POP();

    return true;
    err:
    return false;
}

/** Elseif statement.
 * !rule <elseif_stmt> -> elseif <cond_body>
 */
static bool elseif_stmt() {
    EXPECTED(KEYWORD_elseif);
    // pop an existent symtable, because we ara in the if statement(scope).
    SYMSTACK_POP();
    // also, we need to create a new symtable for 'elseif' scope.
    SYMSTACK_PUSH(SCOPE_TYPE_condition, NULL);

    // generate start of elseif block
    Generator.cond_elseif(instructions.outer_cond_id, instructions.cond_cnt);

    return cond_stmt();
    err:
    return false;
}

/** Conditional expression body implemented with an extension. Contains statements.
 *
 * !rule <cond_body> -> end
 * !rule <cond_body> -> <else_stmt>
 * !rule <cond_body> -> <elseif_stmt>
 *
 *
 * here, we are free to take every statement from fun_stmt,
 * however, the next statement must be from <cond_body>,
 * because we remember about else or elseif
 * !rule <cond_body> -> <fun_stmt> <cond_body>
 *
 * @param pfile pfile.
 * @return bool.
 */
static bool cond_body() {
    debug_msg("<cond_body> -> \n");

    switch (Scanner.get_curr_token().type) {
        case KEYWORD_else:
            return else_stmt();

        case KEYWORD_elseif:
            return elseif_stmt();

        case KEYWORD_end:
            EXPECTED(KEYWORD_end);
            // end of statement reached, so push an existent table.
            SYMSTACK_POP();

            // generate start of end block
            Generator.cond_end(instructions.outer_cond_id, instructions.cond_cnt);
            Generator.pop_cond_info();
            return true;

        default:
            break;
    }

    return fun_stmt() && cond_body();
    err:
    return false;
}

/** Conditional(if or elseif statement). Contains an expression and body.
 * Symtable is created right before calling this function.
 *
 * !rule <cond_stmt> -> `expr` then <cond_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool cond_stmt() {
    debug_msg("<cond_stmt> -> \n");

    // expr
    PARSE_EXPR(EXPR_DEFAULT, NULL);
    // then
    EXPECTED(KEYWORD_then);
    // generate condition evaluation (JUMPIFNEQ ...)
    instructions.cond_cnt++;
    Generator.cond_if(instructions.outer_cond_id, instructions.cond_cnt);

    return cond_body();
    err:
    return false;
}

/** Datatype.
 *
 * !rule <datatype> -> string | integer | boolean | number | nil
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static inline bool datatype() {
    debug_msg("<datatype> ->\n");

    switch (Scanner.get_curr_token().type) {
        case KEYWORD_string:
            EXPECTED(KEYWORD_string);
            break;
        case KEYWORD_boolean:
            EXPECTED(KEYWORD_boolean);
            break;
        case KEYWORD_integer:
            EXPECTED(KEYWORD_integer);
            break;
        case KEYWORD_number:
            EXPECTED(KEYWORD_number);
            break;
        case KEYWORD_nil:
            EXPECTED(KEYWORD_nil);
            break;
        default:
            print_error_unexpected_token("datatype", Scanner.to_string(Scanner.get_curr_token().type));
            Errors.set_error(ERROR_SYNTAX);
            goto err;
    }

    return true;
    err:
    return false;
}

/** Repeat body - function represent body of a repeat-until cycle.
 * Function terminates when a keyword until is found on the input.
 *
 * !rule <repeat_body> -> until | <fun_stmt> <repeat_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool repeat_body() {
    debug_msg("<repeat_body> -> \n");

    // until |
    EXPECTED_OPT(KEYWORD_until);

    // a new solution which doesnt have to cause problems. But not tested yet, so i dont know.
    return fun_stmt() && repeat_body();
    err:
    return false;
}

/** Optional assignment after a local variable declaration.
 *
 * Here, an assign token is processed(if it is), and expression
 * parsing begins.
 * !rule <assignment> -> e | = `expr`
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool assignment(dynstring_t *var_name) {
    debug_msg("<assignment> -> \n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_ASSIGN) {
        // generate var declaration
        Generator.var_declaration(var_name);
        return true;
    }
    // =
    EXPECTED(TOKEN_ASSIGN);
    PARSE_EXPR(EXPR_DEFAULT, NULL);
    // expression result is in GF@%expr_result
    Generator.var_definition(var_name);

    return true;
    err:
    return false;
}

/** For assignment.
 *
 * !rule <for_assignment> -> do | , `expr` do
 *
 * @param pfile pfile
 * @return bool.
 */
static bool for_assignment() {
    debug_msg("<for_assignment> ->\n");
    dynstring_t *expected_signature = NULL;
    dynstring_t *received_signature = NULL;

    // do |
    EXPECTED_OPT(KEYWORD_do);
    // ,
    EXPECTED(TOKEN_COMMA);
    expected_signature = Dynstring.ctor("f");
    received_signature = Dynstring.ctor("");
    // expr
    PARSE_EXPR(EXPR_DEFAULT, received_signature);
    CHECK_EXPR_SIGNATURES(expected_signature, received_signature, ERROR_TYPE_MISSMATCH);
    // do
    EXPECTED(KEYWORD_do);

    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return false;
}

/** For cycle.
 * !rule <for_cycle> -> for id = `expr` , `expr` <for_assignment> <fun_body>
 *
 * @param pfile a program.
 * @return bool.
 */
static bool for_cycle() {
    debug_msg("for ->\n");

    dynstring_t *id_name = NULL;
    dynstring_t *expected_signature = Dynstring.ctor("f");
    dynstring_t *received_signature = Dynstring.ctor("");

    increase_nesting();
    // push a new symtable on the symstack
    SYMSTACK_PUSH(SCOPE_TYPE_for_cycle, NULL);

    // for
    EXPECTED(KEYWORD_for);
    // get id, or get an error.
    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);
    // check semantics
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, ID_TYPE_integer);
    // =
    EXPECTED(TOKEN_ASSIGN);
    // expr
    PARSE_EXPR(EXPR_DEFAULT, received_signature);
    // check signatures
    CHECK_EXPR_SIGNATURES(expected_signature, received_signature, ERROR_TYPE_MISSMATCH);
    // for reusing
    Dynstring.clear(received_signature);

    // ,
    EXPECTED(TOKEN_COMMA);

    // terminating `expr` in for cycle.
    PARSE_EXPR(EXPR_DEFAULT, received_signature);
    // check signatures for an assignment.
    CHECK_EXPR_SIGNATURES(expected_signature, received_signature, ERROR_TYPE_MISSMATCH);

    // do | , `expr` do
    if (!for_assignment()) {
        goto err;
    }
    // <fun_body>, which ends with 'end'
    if (!fun_body()) {
        goto err;
    }

    SYMSTACK_POP();
    decrease_nesting();

    Dynstring.dtor(id_name);
    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(id_name);
    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return false;
}

/** If conditional statement.
 *
 * !rule <if_stmt> -> if <cond_stmt>
 *
 * @param pfile
 * @return
 */
static bool if_statement() {
    debug_msg("<if_stmt> ->\n");

    // if
    EXPECTED(KEYWORD_if);
    SYMSTACK_PUSH(SCOPE_TYPE_condition, NULL);
    // generate code
    Generator.push_cond_info();
    instructions.outer_cond_id = Symstack.get_scope_info(symstack).unique_id;
    instructions.cond_cnt = 1;

    return cond_stmt();
    err:
    return false;
}

/** Local variable definition.
 *
 * !rule <var_definition> -> local id : <datatype> <assignment>
 *
 * @param pfile
 * @return
 */
static bool var_definition() {
    dynstring_t *id_name = NULL;
    int id_type;
    EXPECTED(KEYWORD_local);

    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);
    // :
    EXPECTED(TOKEN_COLON);
    // get id type
    id_type = Scanner.get_curr_token().type;
    // <datatype>
    if (!datatype()) {
        goto err;
    }
    // = `expr`
    if (!assignment(id_name)) {
        goto err;
    }
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** While cycle.
 *
 * !rule <while_cycle> -> while `expr` do <fun_body>
 *
 * @param pflile
 * @return
 */
static bool while_cycle() {
    debug_msg("<while_cycle> -> \n");
    increase_nesting();

    dynstring_t *expected_signature = Dynstring.ctor("b");
    dynstring_t *received_signature = Dynstring.ctor("");

    // while
    EXPECTED(KEYWORD_while);
    // create a new symtable for while cycle.
    SYMSTACK_PUSH(SCOPE_TYPE_while_cycle, NULL);
    // nested while
    if (!instructions.in_loop) {
        instructions.in_loop = true;
        instructions.outer_loop_id = Symstack.get_scope_info(symstack).unique_id;
        instructions.before_loop_start = instrList->tail;   // use when declaring vars in loop
    }
    Generator.while_header();
    // parse expressions
    PARSE_EXPR(EXPR_DEFAULT, received_signature);
    // check operation semantics
    CHECK_EXPR_SIGNATURES(expected_signature, received_signature, ERROR_TYPE_MISSMATCH);
    // expression result in LF@%result
    Generator.while_cond();
    // do
    EXPECTED(KEYWORD_do);
    if (!fun_body()) {
        return false;
    }
    // parent function pops a table from the stack.
    SYMSTACK_POP();
    decrease_nesting();

    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return false;
}

/** Return statement.
 *
 * !rule <return_stmt> -> return <return_expr_list>
 * @param pfile
 * @return
 */
static bool return_stmt() {
    debug_msg("<return_stmt> ->\n");
    dynstring_t *received_rets = Dynstring.ctor("");
    dynstring_t *expected_rets = NULL;

    EXPECTED(KEYWORD_return);
    // create expected returns vector from returns
    expected_rets = Dynstring.dup(Symstack.get_parent_func(symstack)->function_semantics->definition.returns);
    // return expr
    PARSE_EXPR_LIST(EXPR_DEFAULT, received_rets);
    // check signatures
    CHECK_EXPR_SIGNATURES(expected_rets, received_rets, ERROR_FUNCTION_SEMANTICS);

    Dynstring.dtor(expected_rets);
    Dynstring.dtor(received_rets);
    return true;
    err:
    Dynstring.dtor(expected_rets);
    Dynstring.dtor(received_rets);
    return false;
}

/** Repeat until cycle.
 *
 * !rule <repeat_until_cycle> -> repeat <repeat_body>
 *
 * @param pfile
 * @return
 */
static bool repeat_until_cycle() {
    debug_msg("<repeat_until> ->\n");

    dynstring_t *expected_signature = Dynstring.ctor("b");
    dynstring_t *received_signature = Dynstring.ctor("");

    increase_nesting();
    // create a new scope.
    SYMSTACK_PUSH(SCOPE_TYPE_do_cycle, NULL);

    // repeat
    EXPECTED(KEYWORD_repeat);
    // nested while
    if (!instructions.in_loop) {
        instructions.in_loop = true;
        instructions.outer_loop_id = Symstack.get_scope_info(symstack).unique_id;
        instructions.before_loop_start = instrList->tail;   // use when declaring vars in loop
    }
    Generator.repeat_until_header();
    // repeat
    if (!repeat_body()) {
        goto err;
    }
    // expr
    PARSE_EXPR(EXPR_DEFAULT, received_signature);
    // check type compatibility in 'until' condition.
    CHECK_EXPR_SIGNATURES(expected_signature, received_signature, ERROR_TYPE_MISSMATCH);
    // expression result in LF@%result
    Generator.repeat_until_cond();

    // pop a symstack
    SYMSTACK_POP();
    decrease_nesting();

    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return true;
    err:
    Dynstring.dtor(expected_signature);
    Dynstring.dtor(received_signature);
    return false;
}

/** Statement inside the function.
 *
 * !rule <fun_stmt> -> <return_stmt>
 * !rule <fun_stmt> -> <repeat_until_cycle>
 * !rule <fun_stmt> -> <while_cycle>
 * !rule <fun_stmt> -> <var_definition>
 * !rule <fun_stmt> -> <for_cycle>
 * !rule <fun_stmt> -> `expr`
 * !rule <fun_stmt> -> <break>
 *
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool fun_stmt() {
    debug_msg("<fun_stmt> ->\n");

    switch (Scanner.get_curr_token().type) {
        // for <for_def>, `expr` <for_assignment> <fun_body>
        case KEYWORD_for:
            return for_cycle();

            // if <cond_stmt>
        case KEYWORD_if:
            return if_statement();

            // local id : <datatype> <assignment>
        case KEYWORD_local:
            return var_definition();

            // return <return_expr_list>
        case KEYWORD_return:
            return return_stmt();

            // while `expr` do <fun_body> end
        case KEYWORD_while:
            return while_cycle();

            // repeat <some_body> until `expr`
        case KEYWORD_repeat:
            return repeat_until_cycle();

        case KEYWORD_break:
            return break_();

        case TOKEN_ID:
            PARSE_EXPR(EXPR_FUNC, NULL);
            break;

        case TOKEN_DEAD:
            Errors.set_error(ERROR_LEXICAL);
            goto err;

        default:
            Errors.set_error(ERROR_SYNTAX);
            goto err;
    }

    return true;
    err:
    return false;
}

/** Statements inside the function
 *
 * !rule <fun_body> -> <fun_stmt> <fun_body> | end
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool fun_body() {
    debug_msg("<fun_body> ->\n");

    // end |
    if (Scanner.get_curr_token().type == KEYWORD_end) {
        EXPECTED(KEYWORD_end);

        switch (Symstack.get_scope_info(symstack).scope_type) {
            case SCOPE_TYPE_while_cycle:
                // FIXME - can be also for loop
                Generator.while_end();
                if (instructions.outer_loop_id == Symstack.get_scope_info(symstack).unique_id) {
                    instructions.in_loop = false;       // FIXME - nested loops problem!!!
                    instructions.outer_loop_id = 0;
                    instructions.before_loop_start = NULL;
                }
                break;

            case SCOPE_TYPE_function:
                Generator.func_end(Symstack.get_parent_func_name(symstack));
                break;

            case SCOPE_TYPE_for_cycle:
                break;

            case SCOPE_TYPE_do_cycle:
                break;

            case SCOPE_TYPE_condition:
                Generator.cond_end(instructions.outer_cond_id, instructions.cond_cnt);
                Generator.pop_cond_info();
                break;

            default:
                debug_msg("Shouldn't be here.\n");
                assert(0);
                break;
        }
        return true;
    }

    return fun_stmt() && fun_body();
    err:
    return false;
}

/** List of parameter in function definition.
 *
 * !rule <other_funparams> -> ) | , id : <datatype> <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_funparams(pfile_t *pfile, func_info_t function_def_info) {
    debug_msg("<other_funparam> ->\n");

    dynstring_t *id_name = NULL;

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);
    // ,
    EXPECTED(TOKEN_COMMA);
    // get id
    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);
    // :
    EXPECTED(TOKEN_COLON);
    // get type
    token_type_t id_type = Scanner.get_curr_token().type;
    // <datatype>
    if (!datatype()) {
        goto err;
    }
    // semantic check.
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);
    // for function info in the symtable.
    Semantics.add_param(&function_def_info, id_type);
    // generate code
    Generator.func_start_param(id_name, 1);      // FIXME counter

    Dynstring.dtor(id_name);
    return other_funparams(pfile, function_def_info);
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** List with function parameters in the function definition.
 *
 * !rule <funparam_def_list> -> ) | id : <datatype> <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool funparam_def_list(pfile_t *pfile, func_info_t function_def_info) {
    debug_msg("<funparam_def_list> ->\n");

    dynstring_t *id_name = NULL;

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);
    // create a dynstring
    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);
    Generator.func_start_param(id_name, 0);    // need index of the param to the code generator, not the name
    // :
    EXPECTED(TOKEN_COLON);
    // get type
    token_type_t id_type = Scanner.get_curr_token().type;
    // <datatype>
    if (!datatype()) {
        goto err;
    }
    // add a datatype to function parameters
    Semantics.add_param(&function_def_info, id_type);
    // parameters in function definition cannot be declared twice, nor be function names.
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    Dynstring.dtor(id_name);

    // <other_funparams>
    return other_funparams(pfile, function_def_info);
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** Other datatypes.
 *
 * !rule <other_datatypes> -> ) | , <datatype> <other_datatypes>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_datatypes(pfile_t *pfile, func_info_t function_decl_info) {
    debug_msg("<other_datatypes> ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);
    // ,
    EXPECTED(TOKEN_COMMA);
    Semantics.add_param(&function_decl_info, Scanner.get_curr_token().type);

    return datatype() && other_datatypes(pfile, function_decl_info);
    err:
    return false;
}

/** List of datatypes separated by a comma.
 *
 * !rule <dataype_list> -> <datatype> <other_datatypes> | )
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool datatype_list(pfile_t *pfile, func_info_t function_decl_info) {
    debug_msg("<datatype_list> ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);
    Semantics.add_param(&function_decl_info, Scanner.get_curr_token().type);

    //<datatype> && <other_datatypes>
    return datatype() && other_datatypes(pfile, function_decl_info);
    err:
    return false;
}

/** list of return types of the function.
 *
 * !rule <other_funrets> -> e | , <datatype> <other_funrets>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_funrets(pfile_t *pfile, func_info_t function_info) {
    debug_msg("<other_funrets> -> \n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }
    // ,
    EXPECTED(TOKEN_COMMA);
    Semantics.add_return(&function_info, Scanner.get_curr_token().type);
    // generate return var
    Generator.func_return_value(1);         // FIXME counter

    // <datatype> <other_funrets>
    return datatype() && other_funrets(pfile, function_info);
    err:
    return false;
}

/** Optional returns of the function.
 *
 * !rule <funretopt> -> e | : <datatype> <other_funrets>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool funretopt(pfile_t *pfile, func_info_t function_info) {
    debug_msg("<funretopt> ->\n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_COLON) {
        return true;
    }

    // :
    EXPECTED(TOKEN_COLON);
    Semantics.add_return(&function_info, Scanner.get_curr_token().type);
    // generate return var
    Generator.func_return_value(0);

    // <datatype> <other_funrets>
    return (datatype() && other_funrets(pfile, function_info));
    err:
    return false;
}

/** Function declaration.
 *
 * !rule <function_declaration> -> global id : function ( <datatype_list> <funretopt>
 *
 * @param pfile input file.
 * @return bool
 */
static bool function_declaration() {
    debug_msg("<function_declaration> ->\n");

    dynstring_t *id_name = NULL;
    symbol_t *symbol = NULL;

    // global
    EXPECTED(KEYWORD_global);
    GET_ID_SAFE(id_name);
    // function name
    EXPECTED(TOKEN_ID);
    // Semantic control.
    // if we find a symbol on the stack, check it.
    if (Symstack.get_symbol(symstack, id_name, &symbol)) {
        // If function has been previously declared.
        if (Semantics.is_declared(symbol->function_semantics)) {
            Errors.set_error(ERROR_DEFINITION);
            goto err;
        }
    }
    // normally put id on the stack.
    symbol = Symstack.put_symbol(symstack, id_name, ID_TYPE_func_decl);
    // :
    EXPECTED(TOKEN_COLON);
    // function
    EXPECTED(KEYWORD_function);
    // (
    EXPECTED(TOKEN_LPAREN);
    // <funparam_decl_list>
    if (!datatype_list(pfile, symbol->function_semantics->declaration)) {
        goto err;
    }
    // <funretopt> can be empty
    if (!funretopt(pfile, symbol->function_semantics->declaration)) {
        goto err;
    }

    // if function has previously been defined, then check function signatures.
    if (Semantics.is_defined(symbol->function_semantics)) {
        SEMANTIC_CHECK_FUNCTION_SIGNATURES(symbol);
    }

    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** Function definition.
 *
 * !rule <function_definition> -> function id ( <funparam_def_list> <funretopt> <fun_body>
 *
 * @param pfile input file.
 * @return bool.
 */
static bool function_definition() {
    debug_msg("<function_declaration> ->\n");

    dynstring_t *id_name = NULL;
    symbol_t *symbol = NULL;

    // function
    EXPECTED(KEYWORD_function);
    debug_assert((local_table == global_table) && "tables must be equal now");
    // create dynstring to id_name
    GET_ID_SAFE(id_name);
    // id
    EXPECTED(TOKEN_ID);
    // if we find a symbol on the stack, check it.
    if (Symstack.get_symbol(symstack, id_name, &symbol)) {
        // function has been defined yee
        if (Semantics.is_defined(symbol->function_semantics)) {
            Errors.set_error(ERROR_DEFINITION);
            goto err;
        }
    }
    symbol = Symstack.put_symbol(symstack, id_name, ID_TYPE_func_def);
    // (
    EXPECTED(TOKEN_LPAREN);
    // push a symtable on to the stack.
    SYMSTACK_PUSH(SCOPE_TYPE_function, id_name);
    // generate code for new function start
    debug_msg_s("\t[define] function %s\n", Dynstring.c_str(id_name));
    Generator.func_start(id_name);
    // <funparam_def_list>
    if (!funparam_def_list(pfile, symbol->function_semantics->definition)) {
        goto err;
    }
    // <funretopt>
    if (!funretopt(pfile, symbol->function_semantics->definition)) {
        goto err;
    }
    // check signatures if declared
    if (Semantics.is_declared(symbol->function_semantics)) {
        SEMANTIC_CHECK_FUNCTION_SIGNATURES(symbol);
    }
    // <fun_body>
    if (!fun_body()) {
        goto err;
    }
    SYMSTACK_POP();

    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** Statement(global statement) rule.
 *
 * function declaration: !rule <stmt> -> <function_declaration>
 * function definition: !rule <stmt> -> <function_definintion>
 * function call: !rule <stmt> -> `expr`
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool stmt() {
    debug_msg("<stmt> ->\n");

    token_t token = Scanner.get_curr_token();
    dynstring_t *id_name = NULL;

    switch (token.type) {
        // function declaration: global id : function ( <datatype_list> <funretopt>
        case KEYWORD_global:
            return function_declaration();

            // function definition: function id ( <funparam_def_list> <funretopt> <fun_body>
        case KEYWORD_function:
            return function_definition();

            // function calling: id ( <list_expr> )
        case TOKEN_ID:;
            id_name = Dynstring.ctor(Dynstring.c_str(token.attribute.id));
            // create frame before passing parameters
            Generator.func_createframe();
            // in expressions we pass the parameters
            PARSE_EXPR(EXPR_GLOBAL, NULL);
            // function call
            Generator.func_call(id_name);
            break;

            // FIXME. I dont know how to solve this recursion.
        case TOKEN_EOFILE:
            return true;

        case TOKEN_DEAD:
            Errors.set_error(ERROR_LEXICAL);
            goto err;

        default:
            Errors.set_error(ERROR_SYNTAX);
            goto err;
    }

    Dynstring.dtor(id_name);
    return true;
    err:
    Dynstring.dtor(id_name);
    return false;
}

/** List of global statements: function calls, function declarations, function definitions.
 * !rule <stmt_list> -> <stmt> <stmt_list> | EOF
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool stmt_list() {
    debug_msg("<stmt_list> ->\n");

    // EOF |
    EXPECTED_OPT(TOKEN_EOFILE);

    // <stmt> <stmt_list>
    return stmt() && stmt_list();
    err:
    return false;
}

/** Predicate to check if every declared function is also defined.
 * @return bool
 */
static bool declared_implies_defined(symbol_t *symbol) {
    if (symbol->type == ID_TYPE_func_decl) {
        if (false == symbol->function_semantics->is_defined) {
            return false;
        }
    }
    return true;
}

/** Program(start) rule.
 * !rule <program> -> require "ifj21" <stmt_list>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool program() {
    debug_msg("=====================================\n\n\n");
    debug_msg("PARISNG STARTED\n");
    debug_msg("<program> ->\n");

    dynstring_t *prolog_str = Dynstring.ctor("ifj21");

    // require keyword
    EXPECTED(KEYWORD_require);
    // "ifj21" which is a prolog string after require keyword
    if (Scanner.get_curr_token().type != TOKEN_STR) {
        Errors.set_error(ERROR_SYNTAX);
        goto err;
    }
    // ifj21
    if (Dynstring.cmp(Scanner.get_curr_token().attribute.id, prolog_str) != 0) {
        Errors.set_error(ERROR_SYNTAX);
        goto err;
    }
    // get "ifj21"
    EXPECTED(TOKEN_STR);
    // generate code;
    Generator.prog_start();
    // <stmt_list>
    if (!stmt_list()) {
        goto err;
    }
    // every declared function must be defined.
    if (!Symstack.traverse(symstack, declared_implies_defined)) {
        Errors.set_error(ERROR_DEFINITION);
        goto err;
    }

    Dynstring.dtor(prolog_str);
    return true;
    err:
    Dynstring.dtor(prolog_str);
    return false;
}

/** Initialize symstack and global_frame structures
 * for semantic analysis.
 *
 * @return true/false.
 */
static void Init_parser() {
    Scanner.init();

    // create a stack with symtables.
    symstack = Symstack.init();
    soft_assert(symstack != NULL, ERROR_INTERNAL);

    // create a global table.
    global_table = Symtable.ctor();
    soft_assert(global_table != NULL, ERROR_INTERNAL);

    //at the beginning, local and global tables are equal.
    local_table = global_table;

    // push a global frame
    Symstack.push(symstack, global_table, SCOPE_TYPE_global, /*fun_name*/ NULL);

    // add builtin functions.
    Symtable.add_builtin_function(global_table, "write", "", "");

    Symtable.add_builtin_function(global_table, "readi", "", "i"); // string
    Symtable.add_builtin_function(global_table, "readn", "", "f"); // integer
    Symtable.add_builtin_function(global_table, "reads", "", "s"); // number

    Symtable.add_builtin_function(global_table, "tointeger", "f", "i"); // (f : number) : integer
    Symtable.add_builtin_function(global_table, "substr", "sff",
                                  "s"); // substr(s : string, i : number, j : number) : string
    Symtable.add_builtin_function(global_table, "ord", "si", "i"); // (s : string, i : integer) : integer
    Symtable.add_builtin_function(global_table, "chr", "i", "s"); // (i : integer) : string

}

/** Cleanup functions.
 *
 * @return void
 */
static void Free_parser() {
    Symstack.dtor(symstack);
    Scanner.free();
}


/** Analyse function initializes the scanner, and parsing a token sequence
 *  recursive descent method for everything except expressions and bottom-up
 *  precedence parsing method for expressions.
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool Analyse(pfile_t *pfile_) {
    soft_assert(pfile_ != NULL, ERROR_INTERNAL);

    bool res = false;
    pfile = pfile_;
    Errors.set_error(ERROR_NOERROR);

    // initialize structures(symstack, symtable)
    // add builtin functions.
    Init_parser();
    // get_symbol first token to get_symbol start
    if (TOKEN_DEAD == Scanner.get_next_token(pfile).type) {
        Errors.set_error(ERROR_LEXICAL);
    } else {
        res = program();
    }

    Generator.main_end();
    Free_parser();
    return res;
}

/**
 * parser interface.
 */
const struct parser_interface_t Parser = {
        .analyse = Analyse,
};
