/**
 * @file parser.h
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#pragma once


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#include "progfile.h"
#include "macros.h"
#include "dynstring.h"
#include "symtable.h"
#include "symstack.h"
#include "debug.h"

void print_error_unexpected_token(const char *, const char *);

/** Return an error(syntax)
 */
#define error_unexpected_token(a)                               \
    do {                                                       \
        Errors.set_error(ERROR_SYNTAX);                        \
        print_error_unexpected_token(Scanner.to_string(a),     \
        Scanner.to_string(Scanner.get_curr_token().type));     \
        goto err;                                              \
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

#define GET_ID_SAFE(_idname)                                                 \
    do {                                                                    \
        if (Scanner.get_curr_token().type == TOKEN_ID) {                    \
            _idname = Dynstring.dup(Scanner.get_curr_token().attribute.id); \
        }                                                                   \
    } while (0)

#define EXPECTED_OPT(_tok)                                      \
    do {                                                        \
        if (Scanner.get_curr_token().type == (_tok)) {          \
            EXPECTED(_tok);                                     \
            goto noerr;                                         \
        }                                                       \
    } while (0)


/** A symbol stack with symbol tables for the program.
 */
symstack_t *symstack;

/** Global scope(the first one) with function declarations and definitions.
 */
symtable_t *global_table;

/** A current table.
 */
symtable_t *local_table;

int nested_cycle_level;

extern const struct parser_interface_t Parser;

struct parser_interface_t {
    bool (*analyse)(pfile_t *);
};
