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


#define EXPECTED_OPT(toktype) \
do { \
    if (Scanner.get_curr_token().type == (toktype)) { \
        /* OR use Scanner.get_next_token, but let it be clear in case we want */ \
        /* to change Scanner.get_next_token or add debug messages. */ \
        EXPECTED((toktype)); \
        return true; \
    } \
} while(0)
// ***************************************************************************** //
// ***************************************************************************** //
// ***************************************************************************** //
// ***************************************************************************** //


static bool unmatched_part(pfile_t *);

static bool cond_stmt(pfile_t *);

static bool fun_body(pfile_t *pfile);


/**
 * @brief List of expressont.
 *
 * !rule <other_expr> -> ) | , expr <other_expr>
 *
 * @param pfile program file to pass in to scanner.
 * @return true or false. If false, set an appropriate error code.
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
 * !rule <expr_list> -> ) | expr <other_expr>
 *
 * @param pfile structure representing the program.
 * @return true iff rule derives its production successfully else  false with with an error message otherwise.
 */
static bool expr_list(pfile_t *pfile) {
    debug_msg_s("<expr_list> -> \n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    // expr
    if (!Expr.parse(pfile)) {
        return false;
    }

    // <other_expr>
    return other_expr(pfile);
}


/** // FIXME viz TODO(q) for more info
 * @brief Unmatched part of a conditional statement. Can be empty(premium assignment),
 * else <fun_body>  or elseif <cond_stmt>.
 *
 * !rule <unmatched_part> -> else <fun_body> | elseif <cond_stmt> | e
 *
 * @param pfile structure representing the program filethe input program file
 * @return true iff rule derives its production successfully else wishfalse wisothfalse with an error_interface message otherwise
 */
static bool unmatched_part(pfile_t *pfile) {
    debug_msg_s("<unmatched_part> -> \n");

    switch (Scanner.get_curr_token().type) {
        // else <fun_body>
        case KEYWORD_else:
            EXPECTED(KEYWORD_else);
            if (!fun_body(pfile)) {
                return false;
            }
            break;
            // elseif <cond_stmt>
        case KEYWORD_elseif:
            EXPECTED(KEYWORD_elseif);
            if (!cond_stmt(pfile)) {
                return false;
            }
            break;

            // e
        default:
            break;
    }
    return true;
}

/**
 * @brief // FIXME reimplement me without e rules
 *
 * !rule <cond_stmt> -> expr then <fun_body> <unmatched_part>
 *
 * @param pfile structure representing the program filethe input program file
 * @return true iff rule derives its production successfully else wishfalse wisothfalse with an error_interface message otherwise
 */
static bool cond_stmt(pfile_t *pfile) {
    debug_msg_s("<cond_stmt> -> \n");

    if (!Expr.parse(pfile)) {
        return false;
    }

    EXPECTED(KEYWORD_then);
//    // TODO(q): i dont know yet how to deal with statment inside if body...
//    // why dont i know? because I have no idea where to find a terminal for breaking the recursion.
//    // it's however possible to use obvious solution - e transition. But e transitions will only complicate parsing...
//    // also <fun_body> has transition: <fun_body> -> end and it makes no sense in this case.
//    //
//    if (!fun_body(pfile)) {
//        return false;
//    }

    // unmathed part can either be e(perhaps?) or else/else if ... end
    if (!unmatched_part(pfile)) {
        return false;
    }

    return true;
}

/**
 * @brief Datatype.
 *
 * !rule <datatype> -> string | integer | boolean | number
 *
 * @param pfile structure representing the input program file
 * @return true if token is datatype.
 */
static inline bool datatype(pfile_t *pfile) {
    debug_msg("<datatype> ->\n");

    switch (Scanner.get_curr_token().type) {
        case KEYWORD_string:
            EXPECTED(KEYWORD_number);
            break;
        case KEYWORD_boolean:
            EXPECTED(KEYWORD_number);
            break;
        case KEYWORD_integer:
            EXPECTED(KEYWORD_number);
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
 * @brief
 *
 * !rule <repeat_body> -> until | <fun_body>
 *
 * @param pfile for scanner.
 * @return true or false with Errors.set_error()
 */
static bool repeat_body(pfile_t *pfile) {
    debug_msg_s("<repeat_body> -> \n");

    EXPECTED_OPT(KEYWORD_until);

    // TODO: see TODO(q) for more information.
    //fun_body(pfile);
}


/**
 * @brief Statement inside the function
 * rule <fun_body> -> return expression //todo
 * rule <fun_body> -> local id : <datatype> //todo
 *
 * todo:  deal with the same terminals on the lhs of the rhs of the rule
 * rule <fun_body> -> expr // todo e.g x = fun(a + b, c + d, fun()) or a = b or fun(a, b, c)
 * rule <fun_body> -> expr <more_expressions> = <expression> // todo x = fun(a, b)
 *
 * // cycles
 * rule <fun_body> -> repeat <repeat_body> // todo
 * rule <body> -> while <expr> do <cycle_body>
 *
 * // statements
 * !rule <fun_body> -> if <cond_stmt> end
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
 */
static bool fun_body(pfile_t *pfile) {
    debug_msg("<fun_body> ->\n");

    switch (Scanner.get_curr_token().type) {
        // if <cond_stmt>
        case KEYWORD_if:
            EXPECTED(KEYWORD_if);
            // <cond_stmt>
            if (!cond_stmt(pfile)) {
                return false;
            }
            break;

            // while <expr> do <body>
        case KEYWORD_while:
            EXPECTED(KEYWORD_while);
            if (Expr.parse(pfile)) {
                return false;
            }
            EXPECTED(KEYWORD_do);
            //  there's no problem with while statements, because they end with 'end' keyword,
            // so we can just use <fun_body> for while body, because we get <end> as a recursion break.
            if (!fun_body(pfile)) {
                return false;
            }
            break;

        case KEYWORD_repeat:
            EXPECTED(KEYWORD_repeat);
            if (!repeat_body(pfile)) {
                return false;
            }

            // TODO:
            // I am not sure about it.
            // This also can be done in the function repeat_body().
            // probably it would be better to move this if statement to the function, because then after syntactic check
            // we will need to perfom semantic analysis and AST generation, so I am really not sure how to do this.
            // Why ? because in a function there is easy to understand what is added to the symtable, while
            // in the current implementation it probably can be more complicated to understand what will be added to the adt.
            if (!Expr.parse(pfile)) {
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
            break;

            // return expr_list
        case KEYWORD_return:
            EXPECTED(KEYWORD_return);
            if (!expr_list(pfile)) {
                return false;
            }
            break;

            // end of the function
        case KEYWORD_end:
            EXPECTED(KEYWORD_end);
            return true;

        default:
            // at the end try to parse an expression, because actually recursive descent parser know nothing
            // about them so there "probably" can be an expression here.
            if (!Expr.parse(pfile)) {
                return false;
            }
    }

    return fun_body(pfile);
}


/**
 * @brief
 * !rule <other_funparams> -> ) | , <datatype> id <other_funparams>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
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
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
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
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
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
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
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
 * @param pfile structure representing the program file the input program file
 * @return true if rule derives its production successfully else false  with an error_interface message otherwise
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
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
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
 * function definition: !rule <stmt> -> function id ( <funparam_def_list> <funretopt>
 * function calling: !rule <stmt> -> id ( <expr_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based unsuccessfully based on the production rule(described above)
 */
static bool stmt(pfile_t *pfile) {
    debug_msg("<stmt> ->\n");
    token_t tok; // for debug

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

            if (!fun_body(pfile)) {
                return false;
            }

            break;

        default:
            // function call:
            // <stmt> -> id ( expr_list
            if (Scanner.get_curr_token().type == TOKEN_ID) {
                EXPECTED(TOKEN_ID);
                return expr_list(pfile);
            } else {
                debug_todo(
                        "Add more <stmt> derivations, if there are so. Otherwise return an error_interface message\n");
                debug_msg("Got token: %s\n", Scanner.to_string(Scanner.get_curr_token().type));
                debug_msg("Line: %zu, position: %zu\n", Scanner.get_line(), Scanner.get_charpos());
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
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully
 */
static bool stmt_list(pfile_t *pfile) {
    debug_msg("<stmt_list> ->\n");

    // EOF |
    EXPECTED_OPT(TOKEN_EOFILE);

    // <stmt_list>
    return stmt(pfile) && stmt_list(pfile);
}

/**
 * @brief Program(start) rule.
 * !rule <program> -> require "ifj21" <stmt_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based on the production rule(described above)
 */
static bool program(pfile_t *pfile) {
    dynstring_t prolog_str = Dynstring.create("ifj21");
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
    Dynstring.free(&prolog_str);
    return stmt_list(pfile);
}

/**
 * @brief Analyze function initializes the scanner, gets 1st token and starts parsing using top-down
 * recursive descent method for everything except expressions and bottom-up precedence parsing method for expressions.
 * Syntax analysis is based on LL(1) grammar.
 *
 * @param pfile structure representing program file
 * @return appropriate return code, viz error_interface.c, errror.h
 */
static bool Analyse(pfile_t *pfile) {
    if (!pfile) {
        Errors.set_error(ERROR_INTERNAL);
        return NULL;
    }
    bool res = false;

    // get first token to get start
    Scanner.get_next_token(pfile);
    debug_msg("Start parser.\n");

    // perfom a syntax analysis
    res = program(pfile);

    // dont forget to free
    Scanner.free();

    return res;
}

/**
 * parser interface.
 */
const struct parser_interface_i Parser = {
        .analyse = Analyse
};
