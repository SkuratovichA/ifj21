#include "parser.h"
#include "scanner.h"
#include "errors.h"

#include "expressions.h"
#include "code_generator.h"

static void print_error_unexpected_token(const char *a, const char *b) {
    fprintf(stderr, "line %zu, character %zu\n", Scanner.get_line(), Scanner.get_charpos());
    fprintf(stderr, "ERROR(syntax): Expected '%s', got '%s' instead\n", a, b);
}

// push a new symtable on symstack
#define SYMSTACK_PUSH(_scope_type, _id_name)                \
    do {                                                   \
        local_table = Symtable.ctor();                     \
        Symstack.push(symstack, local_table, _scope_type, Dynstring.c_str(_id_name)); \
    } while (0)

#define SYMSTACK_POP()                          \
     do {                                      \
        Symstack.pop(symstack);                \
        local_table = Symstack.top(symstack);  \
     } while(0)

#define error_unexpected_token(a)                               \
    do {                                                       \
        Errors.set_error(ERROR_SYNTAX);                        \
        print_error_unexpected_token(Scanner.to_string(a),     \
            Scanner.to_string(Scanner.get_curr_token().type)); \
        return false;                                          \
    } while (0)

// a is a dynstring with name of the variable.
#define error_multiple_declaration(a)                               \
    do {                                                           \
        Errors.set_error(ERROR_DEFINITION);                        \
        fprintf(stderr, "line %zu, character %zu\n",               \
           Scanner.get_line(), Scanner.get_charpos());             \
        fprintf(stderr, "ERROR(semantic): variable with name '%s'" \
           " has already been declared!\n", Dynstring.c_str(a));   \
        return false;                                              \
    } while (0)


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
            }                                                                 \
        } else {                                                              \
            error_unexpected_token((p));                                      \
        }                                                                     \
    } while(0)

// if there's a condition of type '<a> -> b | c', you have to add EXPECTED_OPT(b) in the function.
#define EXPECTED_OPT(toktype)                                                         \
    do {                                                                             \
        if (Scanner.get_curr_token().type == (toktype)) {                            \
            /* OR use Scanner.get_next_token, but let it be clear in case we want */ \
            /* to change Scanner.get_next_token or add debug messages. */            \
            EXPECTED((toktype));                                                     \
            return true;                                                             \
        }                                                                            \
    } while(0)

// semantic control whether variable have been declared previously.
#define SEMANTICS_SYMTABLE_CHECK_AND_PUT(name, type)                     \
    do {                                                                \
        dynstring_t *_name = name;                                      \
        symbol_t *_dummy_symbol;                                        \
        /* if name is already defined in the local scope or there*/      \
        /* exists a function with the same name                 */      \
        if (Symtable.get_symbol(local_table, _name, &_dummy_symbol)) {  \
            error_multiple_declaration(_name);                          \
            return false;                                               \
        }                                                               \
        if (Symtable.get_symbol(global_table, _name, &_dummy_symbol)) { \
            error_multiple_declaration(_name);                          \
            return false;                                               \
        }                                                               \
        Symstack.put_symbol(symstack, _name, type);                     \
    } while (0)


static bool cond_stmt(pfile_t *);

static bool fun_body(pfile_t *);

static bool fun_stmt(pfile_t *);


/**
 * @brief Conditional expression body implemented with an extension. Contains statements.
 * !rule <cond_body> -> end
 * !rule <cond_body> -> elseif <cond_stmt>
 * !rule <cond_body> -> else <fun_body>
 *
 *
 * here, we are free to take every statement from fun_stmt,
 * however, the next statement must be from <cond_body>,
 * because we remember about else or elseif
 * !rule <cond_body> -> <fun_stmt> <cond_body>
 *
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
            instructions.cond_num++;
            Generator.cond_else(instructions.outer_cond_id, instructions.cond_num);

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
            Generator.cond_elseif(instructions.outer_cond_id, instructions.cond_num);

            return cond_stmt(pfile);

        case KEYWORD_end:
            EXPECTED(KEYWORD_end);
            // end of statement reached, so push an existent table.
            SYMSTACK_POP();

            // generate start of end block
            Generator.cond_end(instructions.outer_cond_id, instructions.cond_num);
            instructions.cond_num = 0;
            instructions.outer_cond_id = 0;

            return true;
        default:
            break;
    }

    return fun_stmt(pfile) && cond_body(pfile);
}

/**
 * @brief Conditional(if or elseif statement). Contains an expression and body.
 * Symtable is created right before calling this function.
 *
 * !rule <cond_stmt> -> `expr` then <cond_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool cond_stmt(pfile_t *pfile) {
    debug_msg_s("<cond_stmt> -> \n");

    // generate condition evaluation (JUMPIFNEQ ...)
    instructions.cond_num++;
    Generator.cond_if(instructions.outer_cond_id, instructions.cond_num);

    if (!Expr.parse(pfile, true)) {
        instructions.cond_num = 0;
        return false;
    }
    EXPECTED(KEYWORD_then);

    return cond_body(pfile);
}

/**
 * @brief Datatype.
 *
 * !rule <datatype> -> string | integer | boolean | number
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
        default:
            print_error_unexpected_token("datatype", Scanner.to_string(Scanner.get_curr_token().type));
            Errors.set_error(ERROR_SYNTAX);
            return false;
    }
    return true;
}

/**
 * @brief Repeat body - function represent body of a repeat-until cycle.
 * Function terminates when a keyword until is found on the input.
 *
 * !rule <repeat_body> -> until | <fun_stmt> <repeat_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool repeat_body(pfile_t *pfile) {
    debug_msg_s("<repeat_body> -> \n");
    // until |
    EXPECTED_OPT(KEYWORD_until);
    // a new solution which doesnt have to cause problems. But not tested yet, so i dont know.
    return fun_stmt(pfile) && repeat_body(pfile);
}

/**
 * @brief Optional assignment after a local variable declaration.
 *
 * Here, an assign token is processed(if it is), and expression
 * parsing begins.
 * !rule <assignment> -> e | = `expr`
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool assignment(pfile_t *pfile) {
    debug_msg_s("<assignment> -> \n");

    token_t prev_token_id = Scanner.get_prev_token();
    // e |
    if (Scanner.get_curr_token().type != TOKEN_ASSIGN) {
        // generate var declaration
        Generator.var_declaration(prev_token_id);
        return true;
    }

    // =
    EXPECTED(TOKEN_ASSIGN);
    if (!Expr.parse(pfile, true)) {
        return false;
    }
    // where is the expression result?
    // token_t token_value = Scanner.get_curr_token(); // not here - remove
    // generate var definition - in expressions I think
    // Generator.var_definition(prev_token_id, token_value);

    return true;
}

/**
 * @brief optional expressions followed by a comma.
 *
 * !rule <other_return_expr> -> , `expr` <other_return_expr> | e
 *
 * @param pfile pfile
 * @return bool.
 */
static bool other_return_expr(pfile_t *pfile) {
    debug_msg_s("<other_return_expr> -> \n");
    // , |
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }

    EXPECTED(TOKEN_COMMA);
    if (!Expr.parse(pfile, true)) {
        return false;
    }
    return other_return_expr(pfile);
}

/**
 * @brief Expression list after return statement in the function.
 *
 * !rule <return_expr_list> -> `expr` <other_return_expr>
 *
 * @param pfile pfile
 * @return bool.
 */
static bool return_expr_list(pfile_t *pfile) {
    debug_msg_s("<return_expr_list> -> \n");
    if (!Expr.parse(pfile, true)) {
        debug_msg("Expression analysis failed.\n");
        return false;
    }

    return other_return_expr(pfile);
}

/**
 * @brief For assignment.
 * !rule <for_assignment> -> do | , `expr` do
 * @param pfile pfile
 * @return bool.
 */
static bool for_assignment(pfile_t *pfile) {
    debug_msg("<for_assignment> ->\n");
    EXPECTED_OPT(KEYWORD_do);

    EXPECTED(TOKEN_COMMA);

    if (!Expr.parse(pfile, true)) {
        debug_msg("[error] Expression parsing failed.\n");
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

    dynstring_t *id_name = Scanner.get_curr_token().attribute.id;
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, ID_TYPE_integer);

    // id
    EXPECTED(TOKEN_ID);

    //Generator.for_header(Scanner.get_curr_token(), token);


    // =
    EXPECTED(TOKEN_ASSIGN);
    if (!Expr.parse(pfile, true)) {
        debug_msg("Expression function returned false\n");
        return false;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    // terminating `expr` in for cycle.
    if (!Expr.parse(pfile, true)) {
        debug_msg("Expression function returned false\n");
        return false;
    }

    // expr result in LF@%result
    //Generator.for_cond();

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

    id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    EXPECTED(TOKEN_ID); // id
    EXPECTED(TOKEN_COLON); // :

    id_type = Scanner.get_curr_token().type;
    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    // = `expr`
    if (!assignment(pfile)) {
        return false;
    }
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
    if (!Expr.parse(pfile, true)) {
        debug_msg("Expression analysis failed.\n");
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
    // return expr
    if (!return_expr_list(pfile)) {
        return false;
    }

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
    if (!Expr.parse(pfile, true)) {
        debug_msg("Expression function returned false\n");
        return false;
    }

    // expression result in LF@%result
    Generator.repeat_until_cond();

    SYMSTACK_POP();
    return true;
}

/**
 * @brief Statement inside the function.
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
    debug_msg("<fun_stmt> ->");
    debug_msg_s(" (token = %s %s ) -> \n",
                Scanner.to_string(Scanner.get_curr_token().type),
                Scanner.get_curr_token().type == TOKEN_ID || Scanner.get_curr_token().type == TOKEN_STR ?
                Dynstring.c_str(Scanner.get_curr_token().attribute.id) : ""
    );

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
            return Expr.parse(pfile, false);

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

/**
 * @brief Statements inside the function
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
            default:
                debug_msg("Shouldn't be here.\n");
                break;
        }
        return true;
    }

    return fun_stmt(pfile) && fun_body(pfile);
}

/**
 * @brief List of parameter in function definition.
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

    token_t token_func = Scanner.get_curr_token();
    id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    // id
    EXPECTED(TOKEN_ID);

    // :
    EXPECTED(TOKEN_COLON);

    token_type_t id_type = Scanner.get_curr_token().type;
    // add a parameter to the list.
    Semantics.add_param(function_def_info, id_type);
    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    Generator.func_start_param(Dynstring.get_str(id_name), 1);      // FIXME counter

    Dynstring.dtor(id_name);
    return other_funparams(pfile, function_def_info);
}

/**
 * @brief List with function parameters in the function definition.
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

    id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));

    Generator.func_start_param(Dynstring.get_str(id_name), 0);    // need index of the param to the code generator, not the name

    // id
    EXPECTED(TOKEN_ID);
    // :
    EXPECTED(TOKEN_COLON);


    token_type_t id_type = Scanner.get_curr_token().type;
    // add a datatype to function parameters
    Semantics.add_param(function_def_info, id_type);
    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    SEMANTICS_SYMTABLE_CHECK_AND_PUT(id_name, id_type);

    Dynstring.dtor(id_name);
    // <other_funparams>
    return other_funparams(pfile, function_def_info);
}

/**
 * @brief Other datatypes.
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

    Semantics.add_param(function_decl_info, Scanner.get_curr_token().type);
    return datatype(pfile) && other_datatypes(pfile, function_decl_info);
}

/**
 * @brief List of datatypes separated by a comma.
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

    Semantics.add_param(function_decl_info, Scanner.get_curr_token().type);
    //<datatype> && <other_datatypes>
    return datatype(pfile) && other_datatypes(pfile, function_decl_info);
}

/**
 * @brief list of return types of the function.
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

    Semantics.add_return(function_info, Scanner.get_curr_token().type);

    // generate return var
    Generator.func_return_value(1);         // FIXME counter

    // <datatype> <other_funrets>
    return datatype(pfile) && other_funrets(pfile, function_info);
}

/**
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

    Semantics.add_return(function_info, Scanner.get_curr_token().type);

    // generate return var
    Generator.func_return_value(0);

    // <datatype> <other_funrets>
    return datatype(pfile) && other_funrets(pfile, function_info);
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

    // assertion is only for test.
    soft_assert(local_table == global_table && "tables must be equal now", ERROR_SYNTAX);
    id_name = Scanner.get_curr_token().attribute.id;

    // Semantic control.
    // if we find a symbol on the stack, check it.
    if (Symstack.get_symbol(symstack, id_name, &symbol, NULL)) {
        // If function has been defined or declared.
        if (Semantics.is_defined(symbol->function_semantics) || Semantics.is_declared(symbol->function_semantics)) {
            Errors.set_error(ERROR_DEFINITION);
            return false;
        }
    }
    // normally put id on the stack.
    symbol = Symstack.put_symbol(symstack, id_name, ID_TYPE_func_decl);

    // function name
    EXPECTED(TOKEN_ID);
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
    return funretopt(pfile, symbol->function_semantics->declaration);

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
    dynstring_t *id_name2;      // FIXME this merge
    // We need to have a pointer to the symbol in the symbol table.
    symbol_t *symbol = NULL;


    soft_assert((local_table == global_table) && "tables must be equal now", ERROR_SYNTAX);
    id_name = Dynstring.ctor(Dynstring.c_str(Scanner.get_curr_token().attribute.id));
    id_name2 = Scanner.get_curr_token().attribute.id;
    debug_msg("[define] function %s\n", Dynstring.get_str(id_name));
    // Semantic control.
    // if we find a symbol on the stack, check it.
    if (Symstack.get_symbol(symstack, id_name, &symbol, NULL)) {
        // we don't have to control if the symbol is a function,
        // because in the grammar, there's only one options and this option is
        // to be a function.
        // If function has been defined, return false and set an error code.
        if (Semantics.is_defined(symbol->function_semantics)) {
            Errors.set_error(ERROR_FUNCTION_SEMANTICS);
            return false;
        }
    }
    symbol = Symstack.put_symbol(symstack, id_name, ID_TYPE_func_def);

    // generate code for new function start
    Generator.func_start(Dynstring.get_str(id_name2));
    // id
    EXPECTED(TOKEN_ID);
    // (
    EXPECTED(TOKEN_LPAREN);
    // symtable for a function.
    SYMSTACK_PUSH(SCOPE_TYPE_function, id_name);

    // <funparam_def_list>
    if (!funparam_def_list(pfile, symbol->function_semantics->definition)) {
        return false;
    }

    // pass params

    // <funretopt>
    if (!funretopt(pfile, symbol->function_semantics->definition)) {
        return false;
    }

    // check signatures
    if (Semantics.is_declared(symbol->function_semantics)) {
        if (false == Semantics.check_signatures(symbol->function_semantics)) {
            Errors.set_error(ERROR_FUNCTION_SEMANTICS);
        }
    }

    // <fun_body>
    if (!fun_body(pfile)) {
        return false;
    }
    SYMSTACK_POP();
    Dynstring.dtor(id_name);
    return true;
}

/**
 * @brief Statement(global statement) rule.
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
    dynstring_t *id_name = Dynstring.ctor("");

    switch (token.type) {
        // function declaration: global id : function ( <datatype_list> <funretopt>
        case KEYWORD_global:
            return function_declaration(pfile);

            // function definition: function id ( <funparam_def_list> <funretopt> <fun_body>
        case KEYWORD_function:
            return function_definition(pfile);

            // function calling: id ( <list_expr> )
        case TOKEN_ID:
            id_name = Dynstring.cat(id_name, token.attribute.id);
            if (!Expr.parse(pfile, true)) {
                return false;
            }
            // in expressions we pass the parameters
            // function call
            Generator.func_call(Dynstring.get_str(id_name));
            Dynstring.dtor(id_name);
            break;

            // FIXME. I dont know how to solve this recursion.
        case TOKEN_EOFILE:
            return true;

        case TOKEN_DEAD:
            Errors.set_error(ERROR_LEXICAL);
            return false;

        default:
            if (!Expr.parse) {
                Errors.set_error(ERROR_SYNTAX);
                return false;
            }
    }
    return true;
}

/**
 *
 * @brief List of global statements: function calls, function declarations, function definitions.
 * !rule <stmt_list> -> <stmt> <stmt_list> | EOF
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool stmt_list(pfile_t *pfile) {
    debug_msg("<stmt_list> ->\n");
    debug_msg("token in gloobal scope '%s'\n", Scanner.to_string(Scanner.get_curr_token().type));
    // EOF |
    EXPECTED_OPT(TOKEN_EOFILE);

    // <stmt> <stmt_list>
    return stmt(pfile) && stmt_list(pfile);
}

/**
 * @brief Program(start) rule.
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
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }
    if (Dynstring.cmp(Scanner.get_curr_token().attribute.id, prolog_str) != 0) {
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }
    EXPECTED(TOKEN_STR);

    Dynstring.dtor(prolog_str);

    Generator.prog_start();

    // <stmt_list>
    return stmt_list(pfile);
}

/**
 * @brief Initialize symstack and global_frame structures
 * for semantic analysis.
 *
 * @return true/false.
 */
static bool Init_parser() {
    // create a stack with symtables.
    symstack = Symstack.init();
    // create a global table.
    global_table = Symtable.ctor();
    //at the beginning, local and global tables are equal.
    local_table = global_table;

    if (((bool) symstack && (bool) global_table) == 0) {
        return false;
    }

    // push a global frame
    Symstack.push(symstack, global_table, SCOPE_TYPE_global, /*fun_name*/ NULL);


    // TODO: should we add parameters?
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

    return true;
}

/**
 * @breaf Cleanup functions.
 *
 * @return void
 */
static void Free_parser() {
    Scanner.free();
    Symstack.dtor(symstack);
}

/**
 * @brief Analyze function initializes the scanner, gets 1st token and starts parsing using top-down
 * recursive descent method for everything except expressions and bottom-up precedence parsing method for expressions.
 * Syntax analysis is based on LL(1) grammar.
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
    if (Init_parser() == false) {
        Errors.set_error(ERROR_INTERNAL);
        return res;
    }

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

#ifdef SELFTEST_parser

#include "tests/tests.h"
#define PROLOG "require \"ifj21\" \n"
#define END " end "
#define FUN " function "
#define LOCAL " local "
#define STRING " string "
#define IF " if "
#define ELSIF " elseif "
#define ELSE " else "
#define THEN " then "
#define NUMBER " number "
#define WHILE " while "
#define DO " do "
#define REPEAT " repeat "
#define UNTIL " until "
#define FOR " for "
#define CONCAT " .. "
//#define SOME_STRING " \"arst \\\\ \\n 123 \\192 string \" "
#define SOME_STRING "\"test_string\""
#define WRITE " write "
#define READ " read "
#define READS " reads "
#define SUBSTR " substr "
#define GLOBAL " global "
#define RETURN " return "
int main() {
    //1
    pfile_t *pf1 = Pfile.ctor(PROLOG);
    //2
    pfile_t *pf2 = Pfile.ctor("1234.er" PROLOG);
    //3
    pfile_t *pf3 = Pfile.ctor(
            PROLOG
            GLOBAL "foo : function()"
            GLOBAL "baz : function(string)"
            GLOBAL "bar : function(string, integer)"
            GLOBAL "arst : function(string, integer, number, number, integer, string)"
            GLOBAL "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"

            GLOBAL "foo:function()"
            GLOBAL "baz:function(string)"
            GLOBAL "bar:function(string, integer)"
            GLOBAL "arst:function(string, integer, number, number, integer, string)"
            GLOBAL "foo : function() : string\n"
            GLOBAL "baz : function(string) : integer\n"
            GLOBAL "bar : function(string, integer) : number\n"
            GLOBAL "arst : function(string, integer, number) : number\n"
            GLOBAL "foo : function() : string\n"
            GLOBAL "baz : function(number) : integer, integer, integer, integer\n"
            GLOBAL "bar : function(string, integer, number) : number\n"
            GLOBAL "foo : function():string\n"
            GLOBAL "baz : function(number):integer, integer, integer, integer\n"
            GLOBAL "bar : function(string, integer, number):number\n"
            GLOBAL "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"
            GLOBAL "bar : function(string,integer,number):number, integer\n"
            GLOBAL "bar : function(string,integer,number):number, number\n"
            GLOBAL "bar : function(string,integer,number):number, string\n"
            GLOBAL "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"
            GLOBAL "foo : function()\n"
            GLOBAL "foo : function()\n"
            GLOBAL "foo : function()\n"
    );
     pfile_t *pf_semantics_good = Pfile.ctor(
            PROLOG
            GLOBAL "foo : function()"
            GLOBAL "baz : function(string)"
            GLOBAL "bar : function(string, integer)"
            GLOBAL "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"

            FUN "foo()"
            END

            FUN "baz(str : string)"
            END

            FUN "bar(str : string, int : integer)"
            END

            GLOBAL "arst : function(string,         integer,             number,       number,     integer, string)"
            FUN               "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)"
            END

            FUN "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string\n"
            END
    );
    //4
    pfile_t *pf4 = Pfile.ctor(
            PROLOG
            GLOBAL " foo : " FUN "(string) : string\n"
            FUN "bar(param : string) : string\n"
                RETURN "foo\n"
            END
            FUN "foo(param:string):string \n"
                RETURN "bar\n"
            END
    );

    //5
    pfile_t *pf6 = Pfile.ctor(
            "-- Program 3: Prace s ěretzci a vestavenymi funkcemi \n"
            PROLOG
            FUN "main()"
            LOCAL "s1 : string =" SOME_STRING
            LOCAL "s2 : string = s1" CONCAT SOME_STRING
            "print("SOME_STRING")"
            LOCAL "s1len : integer = #s1"
            "s1len = s1len - 4"
            "s1 = "SUBSTR"(s2, s1len, s1len + 4)"
            WRITE"("SOME_STRING")"
            WRITE"("SOME_STRING")"
            "s1 = "READS"()"
            END
    );

    pfile_t *pf7 = Pfile.ctor(
            PROLOG
            FUN "mein()"
            LOCAL "myself" " : " STRING " = " "\"me\""
            WHILE "opposite(love, hate) == false and opposite(love, indifference)" DO
                WHILE "opposite(art, ugliness) == false and opposite(art, indifference)" DO
                    WHILE "opposite(faith, heresy) == false and opposite(faith, indifference)" DO
                        WHILE "opposite(life, death) == false and opposite(life, indifference)" DO
                                "is_beautiful(life)"
                        END
                    END
                END
            END
            END // fun
    );

    pfile_t *pf8 = Pfile.ctor(
            PROLOG
            FUN "main()"
                LOCAL "suka" ":" NUMBER

                IF "suka > 10" THEN
                    WRITE"("SOME_STRING")"
                    LOCAL "suka" ":" STRING "=" SOME_STRING
                    IF "suka > 10" THEN
                        "fuck()"
                    ELSIF "suka < 10" THEN
                        WRITE"("SOME_STRING")"
                    ELSE
                        "die()"
                    END
                END
            END // fun
    );

    pfile_t *pf9 = Pfile.ctor(
            PROLOG
            FUN "yours()"
            REPEAT
            "to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()"
            UNTIL " true "
            END
    );

    pfile_t *pf10 = Pfile.ctor(
            PROLOG
            FUN "yours() : string "
            REPEAT
            REPEAT
            REPEAT
            REPEAT
            REPEAT
            REPEAT
            " live_is_beautiful "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            RETURN "you"
            END
    );

    pfile_t *pf11 = Pfile.ctor(
            PROLOG
            FUN "healthy()"
            FOR "i=0" "," "i<3" DO
            FOR "j=0" "," "j<12" DO
            "push_up()"
            END
            END
            END
    );
    pfile_t *pf12 = Pfile.ctor(
            PROLOG
            FUN "funnnn()"
            END
            );
    pfile_t *pf13 = Pfile.ctor(
            PROLOG
            FUN "funnnn()"
                "AAAAAA()"
            END
            );
    pfile_t *pf14 = Pfile.ctor(
            PROLOG
            FUN "funnnn()"
                "AAAAA_ERRORRR() +++ lol"
            END
            );
    pfile_t *pf_nested_whiles = Pfile.ctor(
            PROLOG
            FUN "funnnn()"
                WHILE "1" DO
                    WHILE "1" DO
                    END
                END
            END
            "main()"
            );


    //5
    pfile_t *pf5 = Pfile.ctor(
            "-- Program 3: Prace s ěretzci a vestavenymi funkcemi \n"
            PROLOG
            FUN "main()"
                LOCAL "s1 : string = " SOME_STRING
                LOCAL "s2 : string = s1"
                "print" //(s1,"SOME_STRING", s2)"
                LOCAL "s1len : integer = 10"
                //"s1 =" SUBSTR"(s2, s1len, s1len + 4)"
            "main()"
    );

    // tests.

#if 0
    Tests.warning("1: prolog only.");
    TEST_EXPECT(Parser.analyse(pf1), true, "First test.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("2: prolog with an error..");
    TEST_EXPECT(Parser.analyse(pf2), false, "Second test. Lixecal error handled.");
    TEST_EXPECT(Errors.get_error() == ERROR_LEXICAL, true, "This error must be a lexical one.");

    Tests.warning("4: Mutually recursive functions.");
    TEST_EXPECT(Parser.analyse(pf4), true, "Mutually recursive functions. Return statement.");
    TEST_EXPECT((Errors.get_error() == ERROR_NOERROR), true, "There's no error.");
    if (Errors.get_error() != ERROR_NOERROR) {
        Tests.warning("Error(error must not be here) %s\n", Errors.get_errmsg());
    }

    Tests.warning("7: while statements");
    TEST_EXPECT(Parser.analyse(pf7), true, "while statements.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");


    Tests.warning("9: repeat until statements");
    TEST_EXPECT(Parser.analyse(pf9), true, "Repeat until statement");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

   Tests.warning("11: for statements");
    TEST_EXPECT(Parser.analyse(pf11), true, "For statements");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("6: Curve's test simplified");
    TEST_EXPECT(Parser.analyse(pf6), true, "curve's program.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("12: Function with no body");
    TEST_EXPECT(Parser.analyse(pf12), true, "function with no body");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("13: Function with an expression as a body.");
    TEST_EXPECT(Parser.analyse(pf13), true, "function which body is only one expression");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("8: if statements");
    TEST_EXPECT(Parser.analyse(pf8), true, "If statements");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    TEST_EXPECT(Parser.analyse(pf_nested_whiles), true, "nested whiles");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("14: Function with a wrong expression as a body.");
    TEST_EXPECT(Parser.analyse(pf14), false, "function which body is only one wrong expression.");
    TEST_EXPECT(Errors.get_error() == ERROR_SYNTAX, true, "There's a syntax error..");

    Tests.warning("10: repeat until statements");
    TEST_EXPECT(Parser.analyse(pf10), true, "Repeat until statements");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("5: ERROR");
    TEST_EXPECT(Parser.analyse(pf5), true, "curve's program(bigger).");
    TEST_EXPECT(Errors.get_error() == ERROR_SYNTAX, true, "There's an error.");

#endif
    Tests.warning("42: good function semantics");
    TEST_EXPECT(Parser.analyse(pf_semantics_good), true, " decl & def semantics good");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There are no errors.");

    Tests.warning("3: function declarations SEMANTIC ERROR.");
    TEST_EXPECT(Parser.analyse(pf3), false, "Function declarations SEMANTIC ERROR.");
    TEST_EXPECT(Errors.get_error() == ERROR_DEFINITION, true, "Definition error must be handled.");

    // destructors
    Pfile.dtor(pf1);
    Pfile.dtor(pf_semantics_good);
    Pfile.dtor(pf2);
    Pfile.dtor(pf3);
    Pfile.dtor(pf4);
    Pfile.dtor(pf5);
    Pfile.dtor(pf6);

    Pfile.dtor(pf12);
    Pfile.dtor(pf13);
    Pfile.dtor(pf14);

    Pfile.dtor(pf_nested_whiles);

    return 0;
}
#endif
