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
    fprintf(stderr, "line %zu, character %zu\n", Scanner.get_line(), Scanner.get_charpos());
    fprintf(stderr, "\tERROR: Expected %s, got %s instead\n", a, b);
}

#define expected_err(a) \
    return print_expected_err( Scanner.to_string(a), Scanner.to_string(Scanner.get_curr_token().type) ), false

#define EXPECTED(p) \
    if( Scanner.get_curr_token().type == (p)) { \
      Scanner.get_next_token(pfile); \
    } else \
        expected_err((p))

//#define EXPECTED_2(a, ...) \
//    if (Scanner.get_curr_token().type == (a)) { \
//        Scanner.get_next_token(pfile); \
//    } else \
//    EXPECTED(__VA_ARGS__)
//
//#define EXPECTED_3(a, ...) \
//    if (Scanner.get_curr_token().type == (a)) { \
//        Scanner.get_next_token(pfile); \
//    } else \
//    EXPECTED_2(__VA_ARGS__)
//
//#define EXPECTED_4(a, ...) \
//    if (Scanner.get_curr_token().type == (a)) { \
//        Scanner.get_next_token(pfile); \
//    } else \
//    EXPECTED_3(__VA_ARGS__)
//
//#define EXPECTED_5(a, ...) \
//    if (Scanner.get_curr_token().type == (a)) { \
//        Scanner.get_next_token(); \
//    } else \
//    EXPECTED_4(__VA_ARGS__)
// ***************************************************************************** //




/**
 * @brief Datatype rule. To have a more "pure" solution this function has to have a "brother",
 * which is a datatype or an empty string, because some rules expects e production either,
 * when other don't. However, to write less code I've decided not to implement two similar function,
 * but to move an error message "above"(to the rule that calls this function).
 *
 * !rule <datatype> -> string | integer | number | boolean | e
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool datatype(progfile_t *pfile) {
    debug_msg("<datatype> ->\n");
    switch (Scanner.get_curr_token().type) {
        case_4(KEYWORD_string, KEYWORD_boolean, KEYWORD_integer, KEYWORD_number):
            debug_msg("\t%s\n", Scanner.to_string(Scanner.get_curr_token().type));
            Scanner.get_next_token(pfile);
            break;
        default:
            // todo:
            //      Not sure about an error message here,
            //      because <datatype_list> calls <datatype>
            //      and <datatype_list> can be empty,
            //      so it's possible to have another token here.
            //      Probably it'll be good to add an e derivation here
            // todo:
            //      Not even sure if <datatype> has to be a rule.
            //      Anyway it is more convenient if it is a rule
            //      to not "ovverule" the grammar.
            //print_expected_err("Datatype", Scanner.to_string(Scanner.get_curr_token().type));
            return false;
    }
    return true;
}

/**
 * @brief <other_funparams> -> e | , <datatype> id <other_funparams>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool other_funparams(progfile_t *pfile) {
    debug_msg("<funparam_def_list> ->\n");
    // e | , ...
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        debug_msg("\te\n");
        return true;
    }

    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }
    debug_msg("\t<datatype>\n");

    EXPECTED(TOKEN_ID);
    debug_msg("\tid\n");

    return other_funparams(pfile);
}


/**
 * @brief <funparam_def_list> -> e | <datatype> id <other_funparams>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool funparam_def_list(progfile_t *pfile) {
    debug_msg("funparam_def_list ->\n");
    // <datatype> | e
    if (!datatype(pfile)) {
        // e
        debug_msg("\te\n");
        return true;
    }

    // id
    EXPECTED(TOKEN_ID);
    debug_msg("\tid\n");

    // <other_funparams>
    if (!other_funparams(pfile)) {
        return false;
    }
    debug_msg("\t<other_funparams>\n");

    return true;
}

/**
 * @brief Other datatypes can be an e, or <datatype> <other datatypes> followed by a comma
 * !rule <other_datatypes> -> e | , <datatype> <other_datatypes>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool other_datatypes(progfile_t *pfile) {
    debug_msg("<other_datatypes> ->\n");
    // e
    if (Scanner.get_curr_token().type != TOKEN_COMMA) {
        debug_msg("\te\n");
        return true;
    }

    EXPECTED(TOKEN_COMMA);
    // <datatype>
    if (!datatype(pfile)) {
        print_expected_err("Datatype", Scanner.to_string(Scanner.get_curr_token().type));
        return false;
    }
    debug_msg("\t<datatype>\n");

    // <other_datatypes> it is better to tail recurse this function
//    if (!other_datatypes(pfile)) {
//        return false;
//    }
//    debug_msg("\t<other_datatypes>\n");

    return other_datatypes(pfile);
}

/**
 * @brief datatype_list: List of datatypes separated by a comma.
 * !rule <dataype_list> -> <datatype> <other_datatypes>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool datatype_list(progfile_t *pfile) {
    debug_msg("<datatype_list> ->\n");
    // <datatype>
    if (!datatype(pfile)) {
        debug_msg("\te\n");
        return true;
    }

    // <other_datatypes>
    if (!other_datatypes(pfile)) {
        return false;
    }
    debug_msg("\t<other_datatypes>\n");

    return true;
}

/** //fixme decide fhat to do with this e production, because datatype list can have an error(i guess)
 * @brief
 * !rule <funretopt> -> e | : <datatype> <datatype_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool funretopt(progfile_t *pfile) {
    debug_msg("<funretopt> ->\n");

    // e
    if (Scanner.get_curr_token().type != TOKEN_COLON) {
        debug_msg("\t\te\n");
        return true;
    }

    // :
    EXPECTED(TOKEN_COLON);
    debug_msg("\t:\n");

    // <datatype>. There must be at least one datatype.
    if (!datatype(pfile)) {
        return false;
    }
    debug_msg("\t<datatype>\n");

    // <datatype_list>
    if (!datatype_list(pfile)) {
        return false;
    }
    debug_msg("\t<datatype_list>\n");

    return true;
}

/**
 *
 * @brief Statement(global statement) rule.
 *
 * function declaration: !rule <stmt> -> global id : function ( <datatype_list> ) <funcretopt>
 * function definition: !rule <stmt> -> function id ( <funparam_def_list> ) <funretopt>
 *
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production succesfully based onsuccessfully based on the production rule(described above)
 */
static bool stmt(progfile_t *pfile) {
    debug_msg("<stmt> ->\n");
    string id;
    switch (Scanner.get_curr_token().type) {
        // function declaration: global id : function ( <datatype_list> ) <funcretopt>
        case KEYWORD_global:
            // global
            EXPECTED(KEYWORD_global);
            debug_msg("\tglobal\n");

            // function name
            EXPECTED(TOKEN_ID);
            id = Scanner.get_curr_token().attribute.id;
            debug_msg("\tid { token.attribute.id = %s }\n", Dynstring.c_str(&id));

            // :
            EXPECTED(TOKEN_COLON);
            debug_msg("\t:\n");

            // function
            EXPECTED(KEYWORD_function);
            debug_msg("\tfunction\n");

            // (
            EXPECTED(TOKEN_LPAREN);
            debug_msg("\t(\n");

            // <funparam_decl_list>
            if (!datatype_list(pfile)) {
                return false;
            }
            debug_msg("\t<func_decl_list>\n");

            // )
            EXPECTED(TOKEN_RPAREN);
            debug_msg("\t)\n");

            // <funretopt>
            if (!funretopt(pfile)) {
                return false;
            }
            debug_msg("\t<funretopt>\n");
            break;

            // function definition: function id ( <funparam_def_list> ) <funretopt>
        case KEYWORD_function:
            // function
            EXPECTED(KEYWORD_function);
            debug_msg("\tfunction\n");

            // id
            EXPECTED(TOKEN_ID);
            id = Scanner.get_curr_token().attribute.id;
            debug_msg("\tid { token.attribute.id = %s }\n", Dynstring.c_str(&id));

            // (
            EXPECTED(TOKEN_LPAREN);
            debug_msg("\t(\n");

            // <funparam_def_list>
            if (!funparam_def_list(pfile)) {
                return false;
            }
            debug_msg("\t<funparam_def_list>\n");

            EXPECTED(TOKEN_RPAREN);
            debug_msg("\t)\n");

            if (!funretopt(pfile)) {
                return false;
            }
            debug_msg("\t<funretopt\n");

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
 * !rule <stmt_list> -> <stmt> <stmt_list>
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully
 */
static bool stmt_list(progfile_t *pfile) {
    debug_msg("<stmt_list> ->\n");
    if (Scanner.get_curr_token().type == TOKEN_EOFILE) {
        debug_msg("\te\n");
        return true;
    }

    // <stmt>
    if (!stmt(pfile)) {
        return false;
    }
    debug_msg("\t<stmt>\n");

    // <stmt_list>
    if (!stmt_list(pfile)) {
        return false;
    }
    debug_msg("\t<stmt_list>\n");

    return true;
}

/**
 * @brief Program(start) rule.
 * !rule <program> -> require "ifj21" <stmt_list> EOF
 *
 * @param pfile structure representing the input program file
 * @return true if rule derives its production successfully based on the production rule(described above)
 */
static bool program(progfile_t *pfile) {
    const static char *prolog_str = "ifj21";
    debug_msg("<program> ->\n");

    // require keyword
    EXPECTED(KEYWORD_require);
    debug_msg("\trequire\n");

    // "ifj21" which is a prolog string after require keyword
    if (Scanner.get_curr_token().type != TOKEN_STR) {
        return false;
    }
    if (0 != Dynstring.cmp(Scanner.get_curr_token().attribute.id, prolog_str)) {
        return false;
    }

    EXPECTED(TOKEN_STR);
    debug_msg("\t\"ifj21\"\n");

    // <stmt_list>
    if (!stmt_list(pfile)) {
        return false;
    }
    debug_msg("\t<stmt_list>\n");

    // EOF
    EXPECTED(TOKEN_EOFILE);
    debug_msg("\tEOF\n");

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
static bool Analyse() {
    progfile_t *pfile;
    if (!(pfile = Scanner.initialize())) {
        return Errors.return_error(ERROR_INTERNAL);
    }

    // get first token to get start
    Scanner.get_next_token(pfile);
    debug_msg("Start parser.\n");

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
