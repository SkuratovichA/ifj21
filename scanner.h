/**
 * @file scanner.h
 *
 * @brief Header file for scanner.
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
#include "debug.h"


typedef union token_attribute {
    dynstring_t *id; ///< for storing string or identifier
    uint64_t num_i; ///< integer number representation.
    double num_f; ///< fp number representation
} attribute_t;

/** Token produced by a scanner.
 */
typedef struct token {
    int type;
    attribute_t attribute;
} token_t;


#define TOKEN(name) TOKEN_##name
#define STATE(name) STATE_##name
#define KEYWORD(name) KEYWORD_##name


#define STATES(X)              \
    X(INIT)                   \
    X(STR_INIT)               \
    X(STR_ESC)                \
    X(STR_HEX_1)              \
    X(STR_HEX_2)              \
    X(STR_DEC_1_0)            \
    X(STR_DEC_1_1)            \
    X(STR_DEC_1_2)            \
    X(STR_DEC_0_0)            \
    X(STR_DEC_0_1)            \
    X(STR_DEC_0_2)            \
    X(STR_FINAL)              \
    X(ID_INIT)                \
    X(ID_FINAL)               \
    X(COMMENT_INIT)           \
    X(COMMENT_SLINE)          \
    X(COMMENT_BLOCK_1)        \
    X(COMMENT_BLOCK_2)        \
    X(COMMENT_BLOCK_END)      \
    X(COMMENT_FINAL)          \
    X(NUM_1)                  \
    X(NUM_2)                  \
    X(NUM_3)                  \
    X(NUM_4)                  \
    X(NUM_5)                  \
    X(NUM_6)                  \
    X(NUM_7)                  \
    X(NUM_8)                  \
    X(NUM_9)                  \
    X(NUM_INIT)               \
    X(NUM_FINAL)

#define KEYWORDS(X) \
    X(do)       \
    X(string)   \
    X(boolean)  \
    X(number)   \
    X(integer)  \
    X(true)     \
    X(false)    \
    X(if)       \
    X(return)   \
    X(else)     \
    X(repeat)   \
    X(until)    \
    X(break)    \
    X(elseif)   \
    X(local)    \
    X(then)     \
    X(end)      \
    X(nil)      \
    X(while)    \
    X(function) \
    X(global)   \
    X(require)  \
    X(for)      \
    X(and)      \
    X(or)       \
    X(not)      \
    X(UNDEF)

#define SINGLE_CHAR_TOKENS(X) \
    X(EOFILE)                \
    X(LPAREN)                \
    X(RPAREN)                \
    X(ADD)                   \
    X(MUL)                   \
    X(HASH)                  \
    X(COLON)                 \
    X(COMMA)

/**
 * Tokens enum. What does each token represent.
 */
typedef enum token_type {
    // dead token == NULL
    TOKEN(DEAD) = 0,
    // normal tokens
    TOKEN(WS),      // ' '
    TOKEN(EOL),     // '\n'
    TOKEN(ID),      // variable name
    TOKEN(NUM_I),   // integer number
    TOKEN(NUM_F),   // floating pointer number
    TOKEN(STR),     //  c string
    TOKEN(NE),      // '~='
    TOKEN(EQ),      // '=='
    TOKEN(LT),      // '<'
    TOKEN(LE),      // '<='
    TOKEN(GT),      // '>'
    TOKEN(GE),      // '>='
    TOKEN(ASSIGN),  // '='
    TOKEN(DIV_I),   // '/'
    TOKEN(DIV_F),   // '//'
    TOKEN(SUB),     // '-'
    TOKEN(STRCAT),  // ..


    // to make single tokens compatible with their ascii values
    TOKEN(EOFILE) = EOF,
    TOKEN(LPAREN) = '(',
    TOKEN(PERCENT) = '%',
    TOKEN(CARET) = '^',
    TOKEN(RPAREN) = ')',
    TOKEN(ADD) = '+',
    TOKEN(MUL) = '*',
    TOKEN(HASH) = '#',
    TOKEN(COLON) = ':',
    TOKEN(COMMA) = ',',
} token_type_t;


typedef enum states {
#define X(name) STATE(name),
    STATES(X)
#undef X
} states_t;

typedef enum keywords {
    dummy_keyword = 666,
#define X(n) KEYWORD(n),
    KEYWORDS(X)
#undef X
} keyword_t;


extern const struct scanner_interface Scanner;

struct scanner_interface {
    void (*free)();

    struct token (*get_next_token)(pfile_t *);

    token_t (*get_curr_token)(void);

    char *(*to_string)(const int);

    size_t (*get_line)(void);

    size_t (*get_charpos)(void);

    void (*init)();
};

