#include "parser.h"
#include "scanner.h"
#include "errors.h"

#include "expressions.h"

static void print_expected_err(const char *a, const char *b) {
    fprintf(stderr, "line %zu, character %zu\n", Scanner.get_line(), Scanner.get_charpos());
    fprintf(stderr, "\tERROR: Expected '%s', got '%s' instead\n", a, b);
}

#define expected_err(a) \
do { \
    Errors.set_error(ERROR_SYNTAX); \
    print_expected_err(Scanner.to_string(a), Scanner.to_string(Scanner.get_curr_token().type)); \
    return false; \
} while (0)

#define EXPECTED(p) \
do { \
    token_t tok__ = Scanner.get_curr_token(); \
    if (tok__.type == (p)) { \
        if (tok__.type == TOKEN_ID || tok__.type == TOKEN_STR) { \
            debug_msg("\t%s = { '%s' }\n", Scanner.to_string(tok__.type), Dynstring.c_str(tok__.attribute.id)); \
        } else { \
            debug_msg("\t%s\n", Scanner.to_string(tok__.type)); \
        } \
        Scanner.get_next_token(pfile); \
    } else { \
        expected_err((p)); \
    } \
} while(0)

// if there's a condition of type '<a> -> b | c', you have to add EXPECTED_OPT(b) in the function.
#define EXPECTED_OPT(toktype) \
do { \
    if (Scanner.get_curr_token().type == (toktype)) { \
        /* OR use Scanner.get_next_token, but let it be clear in case we want */ \
        /* to change Scanner.get_next_token or add debug messages. */ \
        EXPECTED((toktype)); \
        return true; \
    } \
} while(0)

static bool cond_stmt(pfile_t *);
static bool fun_body(pfile_t *);
static bool fun_stmt(pfile_t *);

/**
 * @brief List of expressont.
 *
 * !rule <other_expr> -> ) | , expr <other_expr>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_expr(pfile_t *pfile) {
    debug_msg_s("<other_expr> -> \n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    // ,
    EXPECTED(TOKEN_COMMA);

    // expr
    if (!Expr.parse(pfile)) {
        return false;
    }

    // <other_expr>
    return other_expr(pfile);
}

/**
 * @brief Expression list. TODO: probably this rule will be the part of Expr.parse().
 *
 * !rule <list_expr> -> ) | expr <other_expr>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool list_expr(pfile_t *pfile) {
    debug_msg_s("<list_expr> -> \n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    // expr
    if (!Expr.parse(pfile)) {
        return false;
    }

    // <other_expr>
    return other_expr(pfile);
}


/**
 * @brief Conditional expression body implemented with an extension. Contains statements.
 * !rule <cond_body> -> else <fun_stmt> <cond_body>
 * !rule <cond_body> -> elseif <cond_stmt>
 *
 * // todo: im not really sure about this rule.
 * !rule <cond_body> -> <fun_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool cond_body(pfile_t *pfile) {
    debug_msg_s("<cond_body> -> \n");

    // else
    if (Scanner.get_curr_token().type == KEYWORD_else) {
        return fun_body(pfile);
    }
    // elseif expression then
    if (Scanner.get_curr_token().type == KEYWORD_elseif) {
        return cond_stmt(pfile);
    }

    return fun_body(pfile);
}

/**
 * @brief Conditional(if or elseif statement). Contains an expression and body.
 *
 * !rule <cond_stmt> -> expr then <cond_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool cond_stmt(pfile_t *pfile) {
    debug_msg_s("<cond_stmt> -> \n");

    if (!Expr.parse(pfile)) {
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
            EXPECTED(TOKEN_DEAD);
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
 * !rule <assignment> -> e | = expression
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool assignment(pfile_t *pfile) {
    debug_msg_s("<assignment> -> \n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_ASSIGN) {
        return true;
    }
    EXPECTED(TOKEN_ASSIGN);

    // expression
    if (!Expr.parse(pfile)) {
        return false;
    }

    return true;
}

/**
 * @brief Other identifiers followed by a comma. Ends with =.
 *
 * !rule <other_identifiers> -> = | , id <other_identifiers>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_identifiers(pfile_t *pfile) {
    debug_msg_s("<other_identifiers> -> \n");

    // '=' |
    EXPECTED_OPT(TOKEN_ASSIGN);

    // ,
    EXPECTED(TOKEN_COMMA);

    // id
    EXPECTED(TOKEN_ID);

    // <other_identifiers>
    return other_identifiers(pfile);
}

/**
 * @brief Start the list with identifiers.
 *
 * !rule <list_identif> -> id <other_identifiers>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool list_identif(pfile_t *pfile) {
    debug_msg_s("<list_identif> -> \n");

    // id
    EXPECTED(TOKEN_ID);

    // <other_identifiers>
    return other_identifiers(pfile);
}

/**
 * @brief Statement inside the function.
 *
 *** The easiest statements:
 * !rule <fun_stmt> -> e
 * !rule <fun_stmt> -> return <list_expr>
 * !rule <fun_stmt> -> local id : <datatype> <assignment>
 *
 *
 *
 *** Cycles:
 ** A basic assignment.
 * !rule <fun_stmt> -> while <expr> do <fun_body>
 ** A premium part.
 * !rule <fun_stmt> -> repeat <repeat_body>
 * // for i = 1, i < 10 do ... end
 * !rule <fun_stmt> -> for id = expression, expression do <fun_body>
 *
 *
 *
 *** Statements:
 * // if cond_stmt which is <cond_stmt> -> else <fun_body> end | <elseif> <fun_body> end | <fun_body> end
 * !rule <fun_stmt> -> if <cond_stmt>
 *
 *
 *
 *** Expressions: function calling, assignments, conditions.
 * rule <fun_stmt> -> <expr_list> = <list_expr> todo: or something like that
 * just function calls, e.g. f + foo(baz(bar())) or soo(qua())
 *
 *
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool fun_stmt(pfile_t *pfile) {
    debug_msg_s("<fun_stmt> -> \n");

    switch (Scanner.get_curr_token().type) {
        // if <cond_stmt>
        case KEYWORD_if:
            EXPECTED(KEYWORD_if);
            // <cond_stmt>
            if (!cond_stmt(pfile)) {
                return false;
            }
            break;

            // local id : <datatype>
        case KEYWORD_local:
            EXPECTED(KEYWORD_local); // local
            EXPECTED(TOKEN_ID); // id
            EXPECTED(TOKEN_COLON); // :
            if (!datatype(pfile)) { // <datatype>
                return false;
            }

            // = expr, but can also be an empty statement.
            assignment(pfile); // assignment
            break;

            // return <list_expr>
        case KEYWORD_return:
            EXPECTED(KEYWORD_return);
            // return expr, expr, expr.
            if (!list_expr(pfile)) {
                return false;
            }
            break;

            // while <expr> do <fun_body> end
        case KEYWORD_while:
            EXPECTED(KEYWORD_while);

            // parse expressions
            if (Expr.parse(pfile)) {
                return false;
            }
            EXPECTED(KEYWORD_do);

            // get a body('end' at the end)
            if (!fun_body(pfile)) {
                return false;
            }
            break;

            // repeat <some_body> until expression
        case KEYWORD_repeat:
            EXPECTED(KEYWORD_repeat);

            // TODO:
            // I am not sure about it.
            // This also can be done in the function repeat_body().
            // probably it would be better to move this if statement to the function, because then after syntactic check
            // we will need to perfom semantic analysis and AST generation, so I am really not sure how to do this.
            // Why ? because in a function there is easy to understand what is added to the symtable, while
            // in the current implementation it probably can be more complicated to understand what will be added to the adt.
            if (!repeat_body(pfile)) {
                return false;
            }

            // expression represent a condition after an until keyword.
            if (!Expr.parse(pfile)) {
                return false;
            }
            break;

            // rule <fun_stmt> -> for id = expression, expression do <fun_body>
        case KEYWORD_for:
            EXPECTED(KEYWORD_for); // for

            // a, b, c.
            // TODO: so prodbably we can use rule from function statements, e.g.
            // a, b, c, d = function()
            // a, b, c, d = b, d, c, a
            if (!list_identif(pfile)) {
                return false;
            }

            // '='
            EXPECTED(TOKEN_ASSIGN);

            // <list_expr>
            if (!list_expr(pfile)) {
                return false;
            }

            // do
            EXPECTED(KEYWORD_do);

            // <fun_body>, which ends with 'end'
            if (!fun_body(pfile)) {
                return false;
            }
            break;


            // todo: add expressions.
        default:
            // at the end try to parse an expression, because actually recursive descent parser know nothing
            // about them so there "probably" can be an expression here.
            if (!Expr.parse(pfile)) {
                return false;
            }
    }

    return true;
}


/**
 * @brief Statements inside the function
 * !rule <fun_body> -> <fun_stmt> <fun_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool fun_body(pfile_t *pfile) {
    debug_msg("<fun_body> ->\n");

    // end |
    EXPECTED_OPT(KEYWORD_end);

    return fun_stmt(pfile) && fun_body(pfile);
}


/**
 * @brief
 * !rule <other_funparams> -> ) | , <datatype> id <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_funparams(pfile_t *pfile) {
    debug_msg("<other_funparam> ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    // ,
    EXPECTED(TOKEN_COMMA);

    // <datatype> here datatype is expected
    if (!datatype(pfile)) {
        return false;
    }

    EXPECTED(TOKEN_ID);


    return other_funparams(pfile);
}

/**
 * @brief List with function parameters in the function definition.
 *
 * !rule <funparam_def_list> -> ) | <datatype> id <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool funparam_def_list(pfile_t *pfile) {
    debug_msg("funparam_def_list ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }

    // id
    EXPECTED(TOKEN_ID);

    // <other_funparams>
    return other_funparams(pfile);
}

/**
 * @brief Other datatypes can be an e, or <datatype> <other datatypes> followed by a comma
 * !rule <other_datatypes> -> ) | , <datatype> <other_datatypes>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_datatypes(pfile_t *pfile) {
    debug_msg("<other_datatypes> ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    EXPECTED(TOKEN_COMMA);

    return datatype(pfile) && other_datatypes(pfile);
}

/**
 * @brief datatype_list: List of datatypes separated by a comma.
 * !rule <dataype_list> -> <datatype> <other_datatypes> | )
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool datatype_list(pfile_t *pfile) {
    debug_msg("<datatype_list> ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    //<datatype> && <other_datatypes>
    return datatype(pfile) && other_datatypes(pfile);
}

/**
 * @brief
 *
 * !rule <other_funcreturns> -> e | , <datatype> <other_funrets>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool other_funrets(pfile_t *pfile) {
    debug_msg("<other_funrets> -> \n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        return true;
    }

    // ,
    EXPECTED(TOKEN_COMMA);

    return datatype(pfile) && other_funrets(pfile);
}


/**
 * @brief
 * !rule <funretopt> -> e | : <datatype> <other_funrets>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool funretopt(pfile_t *pfile) {
    debug_msg("<funretopt> ->\n");

    // e |
    if (Scanner.get_curr_token().type != TOKEN_COLON) {
        return true;
    }

    // :
    EXPECTED(TOKEN_COLON);

    // <<datatype> <other_funrets>
    return datatype(pfile) && other_funrets(pfile);
}

/**
 *
 * @brief Statement(global statement) rule.
 *
 * function declaration: !rule <stmt> -> global id : function ( <datatype_list> <funcretopt>
 * function definition: !rule <stmt> -> function id ( <funparam_def_list> <funretopt> <fun_body>
 * function calling: !rule <stmt> -> id ( <list_expr>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool stmt(pfile_t *pfile) {
    debug_msg("<stmt> ->\n");

    switch (Scanner.get_curr_token().type) {
        // function declaration: global id : function ( <datatype_list> <funcretopt>
        case KEYWORD_global:
            // global
            EXPECTED(KEYWORD_global);

            // function name
            EXPECTED(TOKEN_ID);

            // :
            EXPECTED(TOKEN_COLON);

            // function
            EXPECTED(KEYWORD_function);

            // (
            EXPECTED(TOKEN_LPAREN);

            // <funparam_decl_list>
            if (!datatype_list(pfile)) {
                return false;
            }

            // <funretopt> can be empty
            if (!funretopt(pfile)) {
                return false;
            }
            break;

            // function definition: function id ( <funparam_def_list> <funretopt> <fun_body>
        case KEYWORD_function:
            // function
            EXPECTED(KEYWORD_function);

            // id
            EXPECTED(TOKEN_ID);

            // (
            EXPECTED(TOKEN_LPAREN);

            // <funparam_def_list>
            if (!funparam_def_list(pfile)) {
                return false;
            }

            // <funcretopt>
            if (!funretopt(pfile)) {
                return false;
            }

            // <fun_body>
            if (!fun_body(pfile)) {
                return false;
            }
            break;

            // function calling: id ( <list_expr> )
        case TOKEN_ID:
            EXPECTED(TOKEN_ID);
            // <list_expr>
            if (!list_expr(pfile)) {
                return false;
            }
            break;
        default:
            debug_todo(
                    "Add more <stmt> derivations, if there are so. Otherwise return an error_interface message\n");
            debug_msg("Got token: %s\n", Scanner.to_string(Scanner.get_curr_token().type));
            debug_msg("Line: %zu, position: %zu\n", Scanner.get_line(), Scanner.get_charpos());
            Errors.set_error(ERROR_SYNTAX);
            return false;
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
    dynstring_t prolog_str = Dynstring.ctor("ifj21");
    debug_msg("<program> ->\n");

    // require keyword
    EXPECTED(KEYWORD_require);

    // "ifj21" which is a prolog string after require keyword
    if (Scanner.get_curr_token().type != TOKEN_STR) {
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }
    if (0 != Dynstring.cmp(Scanner.get_curr_token().attribute.id, prolog_str)) {
        Errors.set_error(ERROR_SYNTAX);
        return false;
    }
    EXPECTED(TOKEN_STR);

    // <stmt_list>
    Dynstring.dtor(&prolog_str);
    return stmt_list(pfile);
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
    if (!pfile) {
        Errors.set_error(ERROR_INTERNAL);
        return NULL;
    }
    bool res = false;

    // get first token to get start
    Scanner.get_next_token(pfile);
    // todo add assert.
    debug_msg("Start parser.\n");

    // perfom a syntax analysis
    res = program(pfile);

    // dont forget to dtor

    Scanner.free();

    return res;
}

/**
 * parser interface.
 */
const struct parser_interface_t Parser = {
        .analyse = Analyse
};
