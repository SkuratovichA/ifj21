//
// Created by xskura01 on 23.09.2021.
//
#pragma once


//============================================
// Includes
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
//


//============================================
// States of the scanner automaton
#define STATE(name) _cat_2_(STATE, name)

#define STATES(X) \
    X(INIT) \
    X(STR_INIT) \
    X(STR_ESC) \
    X(STR_DEC_1_0) \
    X(STR_DEC_1_1) \
    X(STR_DEC_1_2) \
    X(STR_DEC_0_0) \
    X(STR_DEC_0_1) \
    X(STR_DEC_0_2) \
    X(STR_FINAL) \
    X(ID_INIT) \
    X(ID_FINAL) \
    X(COMMENT_INIT) \
    X(COMMENT_SLINE) \
    X(COMMENT_BLOCK_1) \
    X(COMMENT_BLOCK_2) \
    X(COMMENT_BLOCK_END) \
    X(COMMENT_FINAL) \
    X(NUM_INIT) \
    X(NUM_ZERO) \
    X(NUM_ZERO_ZERO) \
    X(NUM_ZERO_TRANSITION) \
    X(NUM_DOT) \
    X(NUM_INT) \
    X(NUM_F_PART) \
    X(NUM_F_DOT) \
    X(NUM_EXP) \
    X(NUM_FINAL) \
    X(NUM_EXP_SIGN) \
    X(NUM_DOUBLE) \


enum states {
#define X(name) STATE(name),
    STATES(X)
#undef X
};
//


//============================================
#define TOKEN(name) _cat_2_(TOKEN, name)
typedef enum token_type {
    // errors
    TOKEN(DEAD) = -127,
    // tokens
    TOKEN(EOFILE) = EOF, // to be compatible with EOF
    // normal tokens
    TOKEN(WS), // ' '
    TOKEN(EOL), // '\n'
    TOKEN(ID), // variable name
    TOKEN(NUM_I), // integer number
    TOKEN(NUM_F), // floating pointer number
    TOKEN(STR), //  c string
    TOKEN(NE), // '~='
    TOKEN(EQ), // '=='
    TOKEN(LT), // '<'
    TOKEN(LE), // '<='
    TOKEN(GT), // '>'
    TOKEN(GE), // '>='
    TOKEN(ASSIGN), // '='
    TOKEN(MUL), // '*'
    TOKEN(DIV_I), // '/'
    TOKEN(DIV_F), // '//'
    TOKEN(ADD), // '+'
    TOKEN(SUB), // '-'
    TOKEN(LBRACE), // '{'
    TOKEN(RBRACE), // '}'
    TOKEN(LPAREN), // '('
    TOKEN(RPAREN),  // ')'
    TOKEN(COMMA), // ','
    TOKEN(SEMICOLON), // ';'
    TOKEN(STRCAT),  // ..
    TOKEN(COLON),  // ..
} token_type_t;
//


//===========================================
// Token and itt attribute
typedef union token_attribute {
    string id; // iff token == string || token == identifier
    uint64_t num_i; // iff token == number_int
    double num_f; // -//- number_float
} attribute_t;

typedef struct token {
    int type;
    attribute_t attribute;
} token_t;
//


//============================================
// An interface to access scanner functions
extern const struct scanner_op_struct Scanner;

struct scanner_op_struct {
    void (*free)(progfile_t *);
    progfile_t *(*initialize)();
    token_t (*get_next_token)(progfile_t *);
    token_t (*get_prev_token)();
    token_t (*get_curr_token)();
    char *(*to_string)(const int);
};
//




//============================================
// Keywordisation: should be moved into syntactic analysis
//
//// =====================================
//#define KEYWORD(name) _cat_2_(KEYWORD, name)
//
//#define KEYWORDS(X) \
//    X(do) \
//    X(if) \
//    X(return) \
//    X(else) \
//    X(local) \
//    X(then) \
//    X(end) \
//    X(nil) \
//    X(while) \
//    X(function) \
//    X(read) \
//    X(write) \
//    X(global) \
//    X(require) \
//    X(boolean) \
//    X(true) \
//    X(elseif) \
//    X(false) \
//    X(break) \
//    X(repeat) \
//    X(until)
//
//typedef enum keywords {
//    dummy_keyword = 666,
//#define X(n) KEYWORD(n),
//    KEYWORDS(X)
//#undef X
//} keyword_t;
//
//// keyword pair. name e.g "else" and kwd e.g KEYWORD_else
//typedef struct kypair {
//    const char *nam;
//    int kwd;
//} kpar_t;
//

//static int to_keyword(const char *identif) {
//    static const kpar_t kwds[] = {
//    #define X(n) { #n , KEYWORD(n) },
//            KEYWORDS(X)
//    #undef X
//    };
//    for( int i = 0; i < 9; i++ )
//        if( strcmp(kwds[i].nam, identif) == 0 )
//            return kwds[i].kwd;
//
//    return TOKEN_IDENTIFIER;
//}
////============================================
//