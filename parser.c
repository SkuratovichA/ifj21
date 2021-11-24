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


/** Print an error ife terminal symbol is unexpected.
 *
 * @param a expected.
 * @param b given.
 */
static void print_error_unexpected_token(const char *a, const char *b) {
    fprintf(stderr, "line %zu, character %zu\n", Scanner.get_line(), Scanner.get_charpos());
    fprintf(stderr, "[error](syntax): Expected '%s', got '%s' instead\n", a, b);
}

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
        return false;                                          \
    } while (0)

/** Set an error code and return an error if there's a declaration error.
 */
#define error_multiple_declaration(a)                               \
    do {                                                           \
        Errors.set_error(ERROR_DEFINITION);                        \
        fprintf(stderr, "line %zu, character %zu\n",               \
           Scanner.get_line(), Scanner.get_charpos());             \
        fprintf(stderr, "[error](semantic): variable with name '%s'" \
           " has already been declared!\n", Dynstring.c_str(a));   \
        return false;                                              \
    } while (0)

/** Macro expecting a non terminal from the scanner.
 */
#define EXPECTED(p)                                                           \
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
        symbol_t *_dummy_symbol;                                        \
        /* if name is already defined in the local scope */              \
        if (Symtable.get_symbol(local_table, _name, &_dummy_symbol)) {  \
            error_multiple_declaration(_name);                          \
            return false;                                               \
        }                                                               \
        /* if there exists a function with the same name */             \
        if (Symtable.get_symbol(global_table, _name, &_dummy_symbol)) { \
            error_multiple_declaration(_name);                          \
            return false;                                               \
        }                                                               \
        Symstack.put_symbol(symstack, _name, type);                     \
    } while (0)


static bool cond_stmt(pfile_t *);

static bool fun_body(pfile_t *);

static bool fun_stmt(pfile_t *);


/** Conditional expression body implemented with an extension. Contains statements.
 *
 * !rule <cond_body> -> end
 * !rule <cond_body> -> elseif <cond_stmt>
 * !rule <cond_body> -> else <fun_body>
 *
 * here, we are free to take every statement from fun_stmt,
 * however, the next statement must be from <cond_body>,
 * because we remember about else or elseif
 * !rule <cond_body> -> <fun_stmt> <cond_body>
 *
 * @param pfile pfile.
 * @return bool.
 */
static bool cond_body(pfile_t *pfile) {
    debug_msg("<cond_body> -> \n");
    switch (Scanner.get_curr_token().type) {
        case KEYWORD_else:
            EXPECTED(KEYWORD_else);
            // pop an existent symtable, because we ara in the if statement(scope).
            SYMSTACK_POP();
            // also, we need to create a new symtable for 'else' scope.
            SYMSTACK_PUSH(SCOPE_TYPE_condition, NULL);

            // generate start of else block
            instructions.cond_cnt++;
            Generator.cond_else(instructions.outer_cond_id, instructions.cond_cnt);

            if (!fun_body(pfile)) {
                return false;
            }
            SYMSTACK_POP();
            return true;

        case KEYWORD_elseif:
            EXPECTED(KEYWORD_elseif);
            // pop an existent symtable, because we ara in the if statement(scope).
            SYMSTACK_POP();
            // also, we need to create a new symtable for 'elseif' scope.
            SYMSTACK_PUSH(SCOPE_TYPE_condition, NULL);

            // generate start of elseif block
            Generator.cond_elseif(instructions.outer_cond_id, instructions.cond_cnt);

            return cond_stmt(pfile);

        case KEYWORD_end:
            EXPECTED(KEYWORD_end);
            // end of statement reached, so push an existent table.
            SYMSTACK_POP();

            // generate start of end block
            Generator.cond_end(instructions.outer_cond_id, instructions.cond_cnt);
            instructions.cond_cnt = 0;
            instructions.outer_cond_id = 0;

            return true;
        default:
            break;
    }

    return fun_stmt(pfile) && cond_body(pfile);
}

/** Conditional(if or elseif statement). Contains an expression and body.
 * Symtable is created right before calling this function.
 *
 * !rule <cond_stmt> -> `expr` then <cond_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool cond_stmt(pfile_t *pfile) {
    debug_msg("<cond_stmt> -> \n");

    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        instructions.cond_cnt = 0;
        return false;
    }
    EXPECTED(KEYWORD_then);

    // generate condition evaluation (JUMPIFNEQ ...)
    instructions.cond_cnt++;
    Generator.cond_if(instructions.outer_cond_id, instructions.cond_cnt);

    return cond_body(pfile);
}

/** Datatype.
 *
 * !rule <datatype> -> string | integer | boolean | number | nil
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static inline bool datatype(pfile_t *pfile) {
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
            return false;
    }
    return true;
}

/** Repeat body - function represent body of a repeat-until cycle.
 * Function terminates when a keyword until is found on the input.
 *
 * !rule <repeat_body> -> until | <fun_stmt> <repeat_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool repeat_body(pfile_t *pfile) {
    debug_msg("<repeat_body> -> \n");
    // until |
    EXPECTED_OPT(KEYWORD_until);
    // a new solution which doesnt have to cause problems. But not tested yet, so i dont know.
    return fun_stmt(pfile) && repeat_body(pfile);
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
static bool assignment(pfile_t *pfile, dynstring_t *var_name) {
    debug_msg("<assignment> -> \n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_ASSIGN) {
        // generate var declaration
        Generator.var_declaration(var_name);
        return true;
    }

    // =
    EXPECTED(TOKEN_ASSIGN);
    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        return false;
    }
    // expression result is in GF@%expr_result
    Generator.var_definition(var_name);

    return true;
}

/** For assignment.
 *
 * !rule <for_assignment> -> do | , `expr` do
 *
 * @param pfile pfile
 * @return bool.
 */
static bool for_assignment(pfile_t *pfile) {
    debug_msg("<for_assignment> ->\n");
    EXPECTED_OPT(KEYWORD_do);

    EXPECTED(TOKEN_COMMA);

    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        return false;
    }
    EXPECTED(KEYWORD_do);
    return true;
}

/** For cycle.
 * !rule <for_cycle> -> for id = `expr` , `expr` <for_assignment> <fun_body>
 *
 * @param pfile a program.
 * @return bool.
 */
static bool for_cycle(pfile_t *pfile) {
    EXPECTED(KEYWORD_for); // for
    SYMSTACK_PUSH(SCOPE_TYPE_cycle, NULL);
    dynstring_t *id_name;

    if (Scanner.get_curr_token().type == TOKEN_ID) {
        id_name = Scanner.get_curr_token().attribute.id;
    }

    // id
    EXPECTED(TOKEN_ID);

    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, ID_TYPE_integer);

    // =
    EXPECTED(TOKEN_ASSIGN);
    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        return false;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // terminating `expr` in for cycle.
    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        return false;
    }

    // do | , `expr` do
    if (!for_assignment(pfile)) {
        return false;
    }
    // <fun_body>, which ends with 'end'
    if (!fun_body(pfile)) {
        return false;
    }
    SYMSTACK_POP();
    return true;
}

/** If conditional statement.
 *
 * !rule <fun_stmt> -> if <cond_stmt>
 *
 * @param pfile
 * @return
 */
static bool if_statement(pfile_t *pfile) {
    EXPECTED(KEYWORD_if);
    SYMSTACK_PUSH(SCOPE_TYPE_condition, NULL);

    instructions.outer_cond_id = Symstack.get_scope_info(symstack).unique_id;

    return cond_stmt(pfile);
}

/** Local variable definition.
 *
 * !rule <var_definition> -> local id : <datatype> <assignment>
 *
 * @param pfile
 * @return
 */
static bool var_definition(pfile_t *pfile) {
    dynstring_t *id_name;
    int id_type;
    EXPECTED(KEYWORD_local);

    if (Scanner.get_curr_token().type == TOKEN_ID) {
        id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    }
    EXPECTED(TOKEN_ID); // id

    EXPECTED(TOKEN_COLON); // :

    id_type = Scanner.get_curr_token().type;

    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);
    // = `expr`
    if (!assignment(pfile, id_name)) {
        Dynstring.dtor(id_name);
        return false;
    }
    Dynstring.dtor(id_name);
    return true;
}

/** While cycle.
 *
 * !rule <while_cycle> -> while `expr` do <fun_body>
 *
 * @param pflile
 * @return
 */
static bool while_cycle(pfile_t *pfile) {
    EXPECTED(KEYWORD_while);

    // create a new symtable for while cycle.
    SYMSTACK_PUSH(SCOPE_TYPE_cycle, NULL);

    // nested while
    if (!instructions.in_loop) {
        instructions.in_loop = true;
        instructions.outer_loop_id = Symstack.get_scope_info(symstack).unique_id;
        instructions.before_loop_start = instrList->tail;   // use when declaring vars in loop
    }
    Generator.while_header();

    // parse expressions
    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        return false;
    }

    // expression result in LF@%result
    Generator.while_cond();

    EXPECTED(KEYWORD_do);
    if (!fun_body(pfile)) {
        return false;
    }
    // parent function pops a table from the stack.
    SYMSTACK_POP();
    return true;
}

/** Return statement.
 *
 * !rule <return_stmt> -> return <return_expr_list>
 * @param pfile
 * @return
 */
static bool return_stmt(pfile_t *pfile) {
    EXPECTED(KEYWORD_return);
    dynstring_t *sign_returns;
    dynstring_t *return_types = Dynstring.ctor("");

    // pointer is not NULL, because we are inside a function.
    sign_returns = Symstack.get_parent_func(symstack)->function_semantics->definition.returns;

    // return expr
    if (!Expr.parse_expr_list(pfile, return_types)) {
        return false;
    }

    if (!Semantics.check_return_semantics(sign_returns, return_types)) {
        Dynstring.dtor(return_types);
        return false;
    }

    Dynstring.dtor(return_types);
    return true;
}

/** Repeat until cycle.
 *
 * !rule <repeat_until_cycle> -> repeat <repeat_body>
 *
 * @param pfile
 * @return
 */
static bool repeat_until_cycle(pfile_t *pfile) {
    EXPECTED(KEYWORD_repeat);
    SYMSTACK_PUSH(SCOPE_TYPE_do_cycle, NULL);

    // nested while
    if (!instructions.in_loop) {
        instructions.in_loop = true;
        instructions.outer_loop_id = Symstack.get_scope_info(symstack).unique_id;
        instructions.before_loop_start = instrList->tail;   // use when declaring vars in loop
    }
    Generator.repeat_until_header();

    if (!repeat_body(pfile)) {
        return false;
    }

    // expression represent a condition after an until keyword.
    if (!Expr.parse(pfile, true, NULL)) {
        debug_msg("\n\t\t[error] Expression parsing failed.\n");
        return false;
    }

    // expression result in LF@%result
    Generator.repeat_until_cond();

    SYMSTACK_POP();
    return true;
}

/** Statement inside the function.
 *
 * !rule <fun_stmt> -> <return_stmt>
 * !rule <fun_stmt> -> <repeat_until_cycle>
 * !rule <fun_stmt> -> <while_cycle>
 * !rule <fun_stmt> -> <var_definition>
 * !rule <fun_stmt> -> <for_cycle>
 * !rule <fun_stmt> -> `expr`
 *
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool fun_stmt(pfile_t *pfile) {
    debug_msg("<fun_stmt> ->\n");

    switch (Scanner.get_curr_token().type) {
        // for <for_def>, `expr` <for_assignment> <fun_body>
        case KEYWORD_for:
            return for_cycle(pfile);

            // if <cond_stmt>
        case KEYWORD_if:
            return if_statement(pfile);

            // local id : <datatype> <assignment>
        case KEYWORD_local:
            return var_definition(pfile);

            // return <return_expr_list>
        case KEYWORD_return:
            return return_stmt(pfile);

            // while `expr` do <fun_body> end
        case KEYWORD_while:
            return while_cycle(pfile);

            // repeat <some_body> until `expr`
        case KEYWORD_repeat:
            return repeat_until_cycle(pfile);

        case TOKEN_ID:
            if (!Expr.parse(pfile, false, NULL)) {
                debug_msg("\n\t\t[error] Expression parsing failed.\n");
                return false;
            }
            break;

        case TOKEN_DEAD:
            Errors.set_error(ERROR_LEXICAL);
            return false;

            // FIXME: shit, I dont know what to do here...
            // probably, I am not allowed to write code such that. But I really dont know.
        case KEYWORD_end:
            return true;

        default:
            Errors.set_error(ERROR_SYNTAX);
            return false;
    }

    return true;
}

/** Statements inside the function
 *
 * !rule <fun_body> -> <fun_stmt> <fun_body> | end
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool fun_body(pfile_t *pfile) {
    debug_msg("<fun_body> ->\n");
    // end |
    if (Scanner.get_curr_token().type == KEYWORD_end) {
        EXPECTED(KEYWORD_end);

        switch (Symstack.get_scope_info(symstack).scope_type) {
            case SCOPE_TYPE_cycle:
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
            case SCOPE_TYPE_do_cycle:
                break;

                // else <....> end
            case SCOPE_TYPE_condition:
                break;
            default:
                debug_msg("Shouldn't be here.\n");
                assert(0);
                break;
        }
        return true;
    }

    return fun_stmt(pfile) && fun_body(pfile);
}

/** List of parameter in function definition.
 *
 * !rule <other_funparams> -> ) | , id : <datatype> <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_funparams(pfile_t *pfile, func_info_t function_def_info) {
    dynstring_t *id_name;
    debug_msg("<other_funparam> ->\n");
    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);
    // ,
    EXPECTED(TOKEN_COMMA);

    if (Scanner.get_curr_token().type == TOKEN_ID) {
        id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    }

    // id
    EXPECTED(TOKEN_ID);
    // :
    EXPECTED(TOKEN_COLON);

    token_type_t id_type = Scanner.get_curr_token().type;

    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    // for an id in the symtable.
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    // for function info in the symtable.
    Semantics.add_param(&function_def_info, id_type);

    Generator.func_start_param(id_name, 1);      // FIXME counter

    Dynstring.dtor(id_name);
    return other_funparams(pfile, function_def_info);
}

/** List with function parameters in the function definition.
 *
 * !rule <funparam_def_list> -> ) | id : <datatype> <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool funparam_def_list(pfile_t *pfile, func_info_t function_def_info) {
    dynstring_t *id_name;
    debug_msg("<funparam_def_list> ->\n");
    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    if (Scanner.get_curr_token().type == TOKEN_ID) {
        id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    }

    // id
    EXPECTED(TOKEN_ID);
    Generator.func_start_param(id_name, 0);    // need index of the param to the code generator, not the name
    // :
    EXPECTED(TOKEN_COLON);

    token_type_t id_type = Scanner.get_curr_token().type;
    // add a datatype to function parameters
    Semantics.add_param(&function_def_info, id_type);
    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    // parameters in function definition cannot be declared twice, nor be function names.
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    Dynstring.dtor(id_name);
    // <other_funparams>
    return other_funparams(pfile, function_def_info);
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
    return datatype(pfile) && other_datatypes(pfile, function_decl_info);
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
    return datatype(pfile) && other_datatypes(pfile, function_decl_info);
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
    return datatype(pfile) && other_funrets(pfile, function_info);
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
    return (datatype(pfile) && other_funrets(pfile, function_info));
}

/** Function declaration.
 *
 * !rule <function_declaration> -> global id : function ( <datatype_list> <funretopt>
 *
 * @param pfile input file.
 * @return bool
 */
static bool function_declaration(pfile_t *pfile) {
    dynstring_t *id_name;
    symbol_t *symbol;

    // global
    EXPECTED(KEYWORD_global);

    if (Scanner.get_curr_token().type == TOKEN_ID) {
        id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    }

    // function name
    EXPECTED(TOKEN_ID);

    // Semantic control.
    // if we find a symbol on the stack, check it.
    if (Symstack.get_symbol(symstack, id_name, &symbol, NULL)) {
        // If function has been previously declared.
        if (Semantics.is_declared(symbol->function_semantics)) {
            Errors.set_error(ERROR_DEFINITION);
            Dynstring.dtor(id_name);
            return false;
        }
    }
    // normally put id on the stack.
    symbol = Symstack.put_symbol(symstack, id_name, ID_TYPE_func_decl);
    Dynstring.dtor(id_name);

    // :
    EXPECTED(TOKEN_COLON);
    // function
    EXPECTED(KEYWORD_function);
    // (
    EXPECTED(TOKEN_LPAREN);

    // <funparam_decl_list>
    if (!datatype_list(pfile, symbol->function_semantics->declaration)) {
        return false;
    }

    // <funretopt> can be empty
    if (!funretopt(pfile, symbol->function_semantics->declaration)) {
        return false;
    }

    // if function has previously been defined, then check function signatures.
    if (Semantics.is_defined(symbol->function_semantics)) {
        SEMANTIC_CHECK_FUNCTION_SIGNATURES(symbol);
    }
    return true;
}

/** Function definition.
 *
 * !rule <function_definition> -> function id ( <funparam_def_list> <funretopt> <fun_body>
 *
 * @param pfile input file.
 * @return bool.
 */
static bool function_definition(pfile_t *pfile) {
    // function
    EXPECTED(KEYWORD_function);

    dynstring_t *id_name;
    // We need to have a pointer to the symbol in the symbol table.
    symbol_t *symbol = NULL;

    debug_assert((local_table == global_table) && "tables must be equal now");

    // it can be another token
    if (Scanner.get_curr_token().type == TOKEN_ID) {
        id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    }

    // id
    EXPECTED(TOKEN_ID);

    // Semantic control.
    // if we find a symbol on the stack, check it.
    if (Symstack.get_symbol(symstack, id_name, &symbol, NULL)) {
        // we don't have to control if the symbol is a function,
        // because in the grammar, there's only one options and this option is
        // to be a function.
        // If function has been defined, return false and set an error code.
        if (Semantics.is_defined(symbol->function_semantics)) {
            Errors.set_error(ERROR_DEFINITION);
            Dynstring.dtor(id_name); // free the name.
            return false;
        }
    }
    symbol = Symstack.put_symbol(symstack, id_name, ID_TYPE_func_def);

    // (
    EXPECTED(TOKEN_LPAREN);

    // push a symtable on to the stack.
    SYMSTACK_PUSH(SCOPE_TYPE_function, id_name);

    // generate code for new function start
    debug_msg("[define] function %s\n", Dynstring.c_str(id_name));
    Generator.func_start(id_name);
    Dynstring.dtor(id_name);

    // <funparam_def_list>
    if (!funparam_def_list(pfile, symbol->function_semantics->definition)) {
        return false;
    }

    // <funretopt>
    if (!funretopt(pfile, symbol->function_semantics->definition)) {
        return false;
    }

    // check signatures
    if (Semantics.is_declared(symbol->function_semantics)) {
        SEMANTIC_CHECK_FUNCTION_SIGNATURES(symbol);
    }

    // <fun_body>
    if (!fun_body(pfile)) {
        return false;
    }

    SYMSTACK_POP();
    return true;
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
static bool stmt(pfile_t *pfile) {
    debug_msg("<stmt> ->\n");
    token_t token = Scanner.get_curr_token();

    switch (token.type) {
        // function declaration: global id : function ( <datatype_list> <funretopt>
        case KEYWORD_global:
            return function_declaration(pfile);

            // function definition: function id ( <funparam_def_list> <funretopt> <fun_body>
        case KEYWORD_function:
            return function_definition(pfile);

            // function calling: id ( <list_expr> )
        case TOKEN_ID:;
            dynstring_t *id_name = Dynstring.ctor(Dynstring.c_str(token.attribute.id));
            // create frame before passing parameters
            Generator.func_createframe();

            // in expressions we pass the parameters
            // TODO add enum list with INSIDE_STMT, INSIDE_FUNC, GLOBAL_SCOPE
            if (!Expr.parse(pfile, false, NULL)) {
                debug_msg("\n\t\t[error] Expression parsing failed.\n");
                return false;
            }
            // function call
            Generator.func_call(id_name);
            Dynstring.dtor(id_name);
            break;

            // FIXME. I dont know how to solve this recursion.
        case TOKEN_EOFILE:
            return true;

        case TOKEN_DEAD:
            Errors.set_error(ERROR_LEXICAL);
            return false;

        default:
            Errors.set_error(ERROR_SYNTAX);
            return false;
    }
    return true;
}

/** List of global statements: function calls, function declarations, function definitions.
 * !rule <stmt_list> -> <stmt> <stmt_list> | EOF
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool stmt_list(pfile_t *pfile) {
    debug_msg("<stmt_list> ->\n");

    // EOF |
    EXPECTED_OPT(TOKEN_EOFILE);

    // <stmt> <stmt_list>
    return stmt(pfile) && stmt_list(pfile);
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
static bool program(pfile_t *pfile) {
    dynstring_t *prolog_str = Dynstring.ctor("ifj21");
    debug_msg("=====================================\n\n\n");
    debug_msg("PARISNG STARTED\n");
    debug_msg("<program> ->\n");

    // require keyword
    EXPECTED(KEYWORD_require);

    // "ifj21" which is a prolog string after require keyword
    if (Scanner.get_curr_token().type != TOKEN_STR) {
        Dynstring.dtor(prolog_str);
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }
    if (Dynstring.cmp(Scanner.get_curr_token().attribute.id, prolog_str) != 0) {
        Dynstring.dtor(prolog_str);
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }
    EXPECTED(TOKEN_STR);

    Dynstring.dtor(prolog_str);

    Generator.prog_start();

    // <stmt_list>
    if (!stmt_list(pfile)) {
        return false;
    }

    //TODO: every declared function must be defined also. -- it is though.
    if (!Symstack.traverse(symstack, declared_implies_defined)) {
        Errors.set_error(ERROR_DEFINITION);
        return false;
    }
    return true;
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
static bool Analyse(pfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    Errors.set_error(ERROR_NOERROR);
    bool res = false;

    // initialize structures(symstack, symtable)
    // add builtin functions.
    Init_parser();

    // get_symbol first token to get_symbol start
    if (TOKEN_DEAD == Scanner.get_next_token(pfile).type) {
        Errors.set_error(ERROR_LEXICAL);
    } else {
        res = program(pfile);
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
