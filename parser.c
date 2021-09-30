/********************************************
 * Project name: IFJ - projekt
 * File: parser
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 *
 *
 *  @package parser
 *  @file parser.c
 *  @brief // TODO info about file
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */


#include "parser.h"
#include "scanner.h"
#include "errors.h"

static void print_expected_err(const char *a, const char *b) {
    fprintf(stderr, "file: <filename>, line <line number>, character <character position>:\n:");
    fprintf(stderr, "ERROR: Expected %s, got %s instead\n", a, b);
}

#define expected_err(a) \
    return print_expected_err( Scanner.to_string(a), Scanner.to_string(Scanner.get_curr_token().type) ), false

#define EXPECTED(p) \
    if( Scanner.get_curr_token().type != (p)) \
        expected_err((p)); \
    Scanner.get_next_token(pfile)
// ***************************************************************************** //


/** //fixme decide fhat to do with this e production, because datatype list can have an error(i guess)
 * @brief
 * !rule <funparam_decl_list> -> e | <datatype_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool funparam_decl_list(progfile_t *pfile) {
    return true;
}

/** //fixme decide fhat to do with this e production, because datatype list can have an error(i guess)
 * @brief
 * !rule <funret_list> -> e | : <datatype_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool funret_list(progfile_t *pfile) {
    return true;
}

/**
 *
 *
 * @brief Statement(global statement) rule.
 *
 * function declaration: !rule <stmt> -> global id : function ( <funparam_decl_list> ) <funcret_list>
 * function definition: !rule <stmt> -> function id ( <funparam_def_list> ) <funcret_list>
 *
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool stmt(progfile_t *pfile) {
    switch (Scanner.get_curr_token().type) {
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
            if (!funparam_decl_list(pfile)) {
                return false;
            }
            // )
            EXPECTED(TOKEN_RPAREN);
            // <funret_list>
            if (!funret_list(pfile)) {
                return false;
            }

            break;

        case KEYWORD_function:
        EXPECTED(KEYWORD_function);

            break;

        default:
            debug_todo("Add more <stmt> derivations, if there are so. Otherwise return an error message");
            return Errors.return_error(ERROR_SYNTAX);
    }

    return true;
}

/**
 *
 * @brief List of global statements: function calls, function declarations, function definitions.
 * !rule <stmt_list> -> EOF | <stmt> <stmt_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully
 */
static bool stmt_list(progfile_t *pfile) {
    if (Scanner.get_curr_token().type == TOKEN_EOFILE) {
        return true;
    }

    return stmt(pfile) && stmt_list(pfile);
}

/**
 * @brief Program(start) rule.
 * !rule <program> -> require "ifj21" <stmt_list> EOF
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based on the production rule(described above)
 */
static bool program(progfile_t *pfile) {
    bool ret = false;
    const static char *prolog_str = "ifj21";

    // require keyword
    EXPECTED(KEYWORD_require);

    // "ifj21" which is a prolog string after require keyword
    if (Scanner.get_curr_token().type == TOKEN_STR
        || 0 != Dynstring.cmp(Scanner.get_curr_token().attribute.id, prolog_str)) {
        return false;
    }

    // <stmt_list>
    if (!stmt_list(pfile)) {
        return false;
    }

    // EOF
    EXPECTED(TOKEN_EOFILE);
    return true;
}

/**
 * @brief Analyze function initializes the scanner, gets 1st token and starts parsing using top-down
 * recursive descent method for everything except expressions and bottom-up precedence parsing method for expressions.
 * Syntax analysis is based on LL(1) grammar.
 *
 * @param pfile structure representing program file
 * @return appropriate return code, viz error.c, errror.h
 */
static bool Analyse(progfile_t *pfile) {
    if (!Scanner.initialize()) {
        return Errors.return_error(ERROR_INTERNAL);
    }

    // get first token to get start
    Scanner.get_next_token(pfile);

    // perfom a syntax analysis
    bool res = program(pfile);

    // dont forget to free
    Scanner.free(pfile);

    // todo: i guess it wants more clearly solution because there will
    //  be semantics controls in the parser so every function probably has to set the error code global variable up
    return res ? true : Errors.return_error(ERROR_SYNTAX);
}

/**
 * parser interface.
 */
const struct parser_op_struct Parser = {
        .analyse = Analyse
};
