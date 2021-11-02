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
do {                \
    token_t tok__ = Scanner.get_curr_token(); \
    if (tok__.type == (p)) {                  \
        if (tok__.type == TOKEN_ID || tok__.type == TOKEN_STR) { \
            debug_msg("\t%s = { '%s' }\n", Scanner.to_string(tok__.type), Dynstring.c_str(tok__.attribute.id)); \
        } else {    \
            if (tok__.type == KEYWORD_end) {      \
                active_scope = active_scope->parent; /* change scope to the prev.scope */         \
            }           \
            debug_msg("\t%s\n", Scanner.to_string(tok__.type));  \
        }           \
        if (TOKEN_DEAD == Scanner.get_next_token(pfile).type) {  \
            Errors.set_error(ERROR_LEXICAL);  \
        }           \
    } else {        \
        expected_err((p));                    \
    }               \
} while(0)

// if there's a condition of type '<a> -> b | c', you have to add EXPECTED_OPT(b) in the function.
#define EXPECTED_OPT(toktype) \
do {                          \
    if (Scanner.get_curr_token().type == (toktype)) { \
        /* OR use Scanner.get_next_token, but let it be clear in case we want */ \
        /* to change Scanner.get_next_token or add debug messages. */            \
        EXPECTED((toktype));  \
        return true;          \
    }                         \
} while(0)

static bool cond_stmt(pfile_t *);
static bool fun_body(pfile_t *);
static bool fun_stmt(pfile_t *);


// TODO: rename - i dont like it. I want it to be SCOPE_GLOBAL, SCOPE_LOCAL
// think if there's need to have 2 'SCOPES' for functions
typedef enum scope_frame_opt {
    SCOPE_FUN_DECL, SCOPE_FUN_DEF, SCOPE_LOCAL_FRAME
} scope_frame_opt_t;

/**
 * @brief Create a new scope from the given token.
 *
 * @param name token that is head of scope.
 * // fixme: do whe really need it?
 * // fixme: what does the token do here? Can we create a scope without it?
 * @param opt SCOPE_FUN_DECL
 *            SCOPE_FUN_DEF
 */
static void create_scope(token_t name, scope_frame_opt_t opt ){
    switch (opt) {
        case SCOPE_FUN_DECL:
            // If global scope is not create yet. Don't need to set as active. Because main is always on index 0.
            if(symbol_tab->size == 0){
                // Main scope doesn't have any parent so third parameter is null.
                Symt.Ctor(symbol_tab, name.attribute.id, name.type, NULL);
            } else{
                // store to main scope
                Symt.store_id(symbol_tab->scopes[0], name.attribute.id, name.type);
            }
            break;
        case SCOPE_FUN_DEF:
            active_scope = Symt.Ctor(symbol_tab, name.attribute.id, name.type, symbol_tab->scopes[0]);
            break;
        case SCOPE_LOCAL_FRAME:
            // while loop, for loop, if statement.
            active_scope = Symt.Ctor(symbol_tab, name.attribute.id, name.type, active_scope);
            break;
    }
}

/**
 * @brief Conditional expression body implemented with an extension. Contains statements.
 * !rule <cond_body> -> else <fun_body>
 * !rule <cond_body> -> elseif <cond_stmt>
 *
 * // todo: im not quite sure about this rule.
 * here, we are free to take every statement from fun_stmt,
 * however, the next statement must be from <cond_body>,
 * because we remember about else or elseif
 * !rule <cond_body> -> <fun_stmt> <cond_body>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool cond_body(pfile_t *pfile) {
    debug_msg_s("<cond_body> -> \n");

    // TODO: create new scopes here, probably
    // also, we can delete a previous scope a nd create a new one,
    // because anyway, we dont use variables prom the past scope.
    switch (Scanner.get_curr_token().type) {
        case KEYWORD_else:
            EXPECTED(KEYWORD_else);
            // TODO: somehow change scope, because here,
            // scopes must lay lineary to each other.
            return fun_body(pfile);
        case KEYWORD_elseif:
            EXPECTED(KEYWORD_elseif);
            // TODO: somehow change scope, because here,
            // scopes must lay lineary to each other.
            return cond_stmt(pfile);
        case KEYWORD_end: // terminating condition i guess.
            EXPECTED(KEYWORD_end);
            active_scope = active_scope->parent;
            return true;
        default:
            break;
    }

    return fun_stmt(pfile) && cond_body(pfile);
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

    if (!Expr.parse(pfile, true)) {
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
            print_expected_err("datatype", Scanner.to_string(Scanner.get_curr_token().type));
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
 * !rule <assignment> -> e | = expr
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

    // TODO: probably we dont need an Expr.parse() to be boolean (at least here).
    // expression
    if (!Expr.parse(pfile, true)) {
        return false;
    }

    return true;
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
 * // FIXME chose one
 * xrule <fun_stmt> -> for <forcycleexp> do <fun_body> // todo: Probably we have to change this rule.
 * xrule <fun_stmt> -> for id = expression, expression do <fun_body> // todo: Probably we have to change this rule.
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
 * rule <fun_stmt> -> expression
 * just function calls, e.g. f + foo(baz(bar())) or soo(qua())
 *
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
        // if <cond_stmt>
        case KEYWORD_if:
            EXPECTED(KEYWORD_if);

            // Creating scope for if statement
            create_scope(active_scope, SCOPE_LOCAL_FRAME);

            // <cond_stmt>
            if (!cond_stmt(pfile)) {
                return false;
            }
            break;

            // local id : <datatype>
        case KEYWORD_local:
            EXPECTED(KEYWORD_local); // local

            // get the variable name(for a symtable)
            dynstring_t *name = Scanner.get_curr_token().attribute.id;
            EXPECTED(TOKEN_ID); // id
            EXPECTED(TOKEN_COLON); // :

            // get type(for a tymtable)
            token_type_t type = Scanner.get_curr_token().type;
            if (!datatype(pfile)) { // <datatype>
                return false;
            }

            // TODO: perform semantic actions later.
            if (Symt.find_id(active_scope, name)) {
                Errors.set_error(ERROR_SEMANTICS_OTHER);
                return false;
            }

            // = expr, but can also be an empty statement.
            // todo should it be booolean?
            assignment(pfile); // assignment
            break;

            // return expr
        case KEYWORD_return:
            EXPECTED(KEYWORD_return);
            // return expr
            if (!Expr.parse(pfile, /*inside=*/true)) {
                debug_msg("Expression analysis failed.\n");
                return false;
            }
            break;

            // while <expr> do <fun_body> end
        case KEYWORD_while:
            EXPECTED(KEYWORD_while);

            create_scope(active_scope, SCOPE_LOCAL_FRAME);

            // parse expressions
            if (!Expr.parse(pfile, true)) {
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
            if (!Expr.parse(pfile, true)) {
                debug_msg("Expression function returned false\n");
                return false;
            }
            break;

            // rule <fun_stmt> -> for id = expression, expression do <fun_body>
        case KEYWORD_for:
            EXPECTED(KEYWORD_for); // for

            create_scope(active_scope, SCOPE_LOCAL_FRAME);

            // TODO: call function directly for for parsing.
            if (!Expr.parse(pfile, true)) {
                debug_msg("Expression function returned false\n");
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
            if (!Expr.parse(pfile, false)) {
                debug_msg("Expression function returned false\n");
                return false;
            }
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
    EXPECTED_OPT(KEYWORD_end);

    return fun_stmt(pfile) && fun_body(pfile);
}

/**
 * @brief
 * !rule <other_funparams> -> ) | , id : <datatype> <other_funparams>
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

    dynstring_t *name = Scanner.get_curr_token().attribute.id;

    // id
    EXPECTED(TOKEN_ID);

    // :
    EXPECTED(TOKEN_COLON);

    token_type_t type = Scanner.get_curr_token().type;

    // <datatype> here datatype is expected
    if (!datatype(pfile)) {
        return false;
    }

    // TODO: emantics
    if (Symt.find_id(active_scope, name)) {
        debug_msg("Variable has the same name as one of the previously declared functions\n!");
        Errors.set_error(ERROR_SEMANTICS_OTHER);
        return false;
    }

    Symt.store_id(active_scope, name, type);

    return other_funparams(pfile);
}

/**
 * @brief List with function parameters in the function definition.
 *
 * !rule <funparam_def_list> -> ) | id : <datatype> <other_funparams>
 *
 * @param pfile input file for Scanner.get_next_token().
 * @return bool.
 */
static bool funparam_def_list(pfile_t *pfile) {
    debug_msg("funparam_def_list ->\n");

    // ) |
    EXPECTED_OPT(TOKEN_RPAREN);

    dynstring_t *name = Scanner.get_curr_token().attribute.id;

    // id
    EXPECTED(TOKEN_ID);

    // :
    EXPECTED(TOKEN_COLON);

    token_type_t type = Scanner.get_curr_token().type;
    // <datatype>
    if (!datatype(pfile)) {
        return false;
    }
    // TODO: semantics
    // here, a function argument(local scope variable)
    // cannot have the same name as one of previously declared/defined functions.
    if (Symt.find_id(active_scope, name)) {
        debug_msg("Variable has the same name as one of the previously declared functions\n!");
        Errors.set_error(ERROR_SEMANTICS_OTHER);
        return false;
    }
    Symt.store_id(active_scope, name, type);

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

    token_t token = Scanner.get_curr_token();
    token_t name;

    switch (token.type) {

        // function declaration: global id : function ( <datatype_list> <funcretopt>
        case KEYWORD_global:
            // global
            EXPECTED(KEYWORD_global);

            name = Scanner.get_curr_token();

            // function name
            EXPECTED(TOKEN_ID);

            // FIXME: not sure if there's a need to add a scope here.
            // it looks more adequate if there will be only adding an
            // identifier name + function type (parameters, return values and so).
            create_scope(name, SCOPE_FUN_DECL);

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

            name = Scanner.get_curr_token();

            // id
            EXPECTED(TOKEN_ID);

            // Create scope for function and set as active set main scope as parent.
            create_scope(name, SCOPE_FUN_DEF);

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

            name = Scanner.get_curr_token();

            EXPECTED(TOKEN_ID);

            // TODO: semantics.
            // however, first, there's need to implement
            // storing to the symtable, then finding
          //  if((Symt.find_id_in_scope(symbol_tab->scopes[0], token.attribute.id)) == false){
          //      // ERROR: Function is called but not declared.
          //      Errors.set_error(ERROR_SEMANTICS_OTHER);
          //      return false;
          //  }

            // <list_expr>
            if (!list_expr(pfile)) {
                return false;
            }
            break;

        case TOKEN_DEAD:
            Errors.set_error(ERROR_LEXICAL);
            return false;

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
    dynstring_t *prolog_str = Dynstring.ctor("ifj21");
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

    symbol_tab = NULL;
    active_scope = NULL;
    symbol_tab = Symt.st_ctor();

    // <stmt_list>
    Dynstring.dtor(prolog_str);
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
    Errors.set_error(ERROR_NOERROR);
    if (!pfile) {
        Errors.set_error(ERROR_INTERNAL);
        return NULL;
    }
    bool res = false; // Suppose we have an error.

    // get first token to get start
    if (TOKEN_DEAD == Scanner.get_next_token(pfile).type) {
        Errors.set_error(ERROR_LEXICAL);
    } else {
        res = program(pfile);
    }
    // Free scanner
    Scanner.free();

    return res;
}

/**
 * parser interface.
 */
const struct parser_interface_t Parser = {
        .analyse = Analyse
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
            GLOBAL "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"
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
#define RETURN " return "
    //4
    pfile_t *pf4 = Pfile.ctor(
            PROLOG
            GLOBAL " foo : " FUN "(string) : string\n"
            FUN "bar(param : string) : string\n"
            RETURN "foo (param)\n"
            END
            FUN "foo(param:string):string \n"
            RETURN "bar(param)\n"
            END
    );


    //5
    pfile_t *pf5 = Pfile.ctor(
            "-- Program 3: Prace s ěretzci a vestavenymi funkcemi \n"
            PROLOG
            FUN "main()"
            LOCAL "s1" ": string = " SOME_STRING
            LOCAL "s2" ": string = s1 .." SOME_STRING
            "print(s1,"SOME_STRING", s2)"
            LOCAL "s1len : integer=#s1"
            "s1len = s1len - 4 "
            "s1" "=" SUBSTR"(s2, s1len, s1len + 4)"
            "s1len = s1len + 1 "
            WRITE "("SOME_STRING")"
            WRITE "("SOME_STRING")"
            WRITE "("SOME_STRING")"
            "s1 = reads()"
            IF "s1 ~= nil" THEN
            WHILE "s1" "~=" SOME_STRING DO
            WRITE "("SOME_STRING")"
            "s1" "=" READS"()"
            END
            ELSE
            END
            END
            "main()"
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
            LOCAL "suka" ":" NUMBER " = 69"

            IF "suka > 10" THEN
            WRITE"("SOME_STRING")"
            LOCAL "suka" ":" STRING "=" SOME_STRING
            IF "suka > 10" THEN
            "fuck()"
            ELSE
            "die()"
            END
            ELSIF "suka < 10" THEN
            WRITE"("SOME_STRING")"
            ELSE
            "die()"
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
            FUN "yours()"
            REPEAT
            REPEAT
            REPEAT
            REPEAT
            REPEAT
            REPEAT
            " live_is_beautiful() "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
            UNTIL " true "
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


    "if s1 ~= nil then                                                     \n "
    "while s1 ~= \"abcdefgh\" do                                           \n "
    "    write(\"\\n\", \"Spatne zadana posloupnost, zkuste znovu:\")      \n "
    "s1 = reads()                                                          \n "
    "end else                                                              \n "
    "end end                                                               \n "
    "main()                                                                \n ";

    // tests.
#if 0
    Tests.warning("1: prolog only.");
    TEST_EXPECT(Parser.analyse(pf1), true, "First test.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("2: prolog with an error..");
    TEST_EXPECT(Parser.analyse(pf2), false, "Second test. Lixecal error handled.");
    TEST_EXPECT(Errors.get_error() == ERROR_LEXICAL, true, "This error must be a lexical one.");

    Tests.warning("3: function declarations.");
    TEST_EXPECT(Parser.analyse(pf3), true, "Function declarations OK.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("4: Mutually recursive functions.");
    TEST_EXPECT(Parser.analyse(pf4), true, "Mutually recursive functions. Return statement.");
    TEST_EXPECT((Errors.get_error() == ERROR_NOERROR), true, "There's no error.");
    if (Errors.get_error() != ERROR_NOERROR) {
        Tests.warning("Error(error must not be here) %s\n", Errors.get_errmsg());
    }
#endif

    Tests.warning("8: if statements");
    TEST_EXPECT(Parser.analyse(pf8), true, "If statements");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("7: while statements");
    TEST_EXPECT(Parser.analyse(pf7), true, "while statements.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");


    Tests.warning("9: repeat until statements");
    TEST_EXPECT(Parser.analyse(pf9), true, "Repeat until statement");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("10: repeat until statements");
    TEST_EXPECT(Parser.analyse(pf10), true, "Repeat until statements");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("11: for statements");
    TEST_EXPECT(Parser.analyse(pf11), true, "For statements");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");


    Tests.warning("5: Curve's test");
    TEST_EXPECT(Parser.analyse(pf5), true, "curve's program(bigger).");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");

    Tests.warning("6: Curve's test simplified");
    TEST_EXPECT(Parser.analyse(pf6), true, "curve's program.");
    TEST_EXPECT(Errors.get_error() == ERROR_NOERROR, true, "There's no error.");



    // destructors
    Pfile.dtor(pf1);
    Pfile.dtor(pf2);
    Pfile.dtor(pf3);
    Pfile.dtor(pf4);
    Pfile.dtor(pf5);
    Pfile.dtor(pf6);
    return 0;
}
#endif
