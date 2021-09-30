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


/**
 * @brief
 *
 * @param
 * @return
 */
static bool stmt_list(progfile_t *pfile) {
    return true;
}

/**
 * @brief <prolog> -> require "ifj21"
 *
 * @param
 * @return
 */
static bool prolog(progfile_t *pfile) {
    EXPECTED(KEYWORD_require); // must be require

    if (Scanner.get_curr_token().type != TOKEN_STR) {
        return false;
    }

    if (!Dynstring.cmp(Scanner.get_curr_token().attribute.id, "ifj21")) { // can be an error with memory here
        return false;
    }

    EXPECTED(TOKEN_STR); // "ifj21"

}

/**
 * @brief <prog> -> <prolog> <stmt_list> EOF
 *
 * @param
 * @return
 */
static bool prog(progfile_t *pfile) {
    if (!prolog(pfile)) {
        return false;
    }
    if (!stmt_list(pfile)) {
        return false;
    }

    EXPECTED(TOKEN_EOFILE);
    return true;
}

/**
 * @brief
 *
 * @param
 * @return
 */
static bool Analyse(progfile_t *pfile) {
    if (Scanner.initialize()) {
        return Errors.return_error(ERROR_INTERNAL);
    }

    Scanner.get_next_token(pfile); // get first token to get start
    bool res = !prog(pfile); // here we go
    Scanner.free(pfile);

    // todo: i guess it wants more clearly solution because there will
    //  be semantics controls in the parser so every function probably has to set the error code global variable up
    return !res ? Errors.return_error(ERROR_SYNTAX) : true;
}

/**
 * parser interface.
 */
const struct parser_op_struct Parser = {
        .analyse = Analyse
};
