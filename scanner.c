/********************************************
 * Project name: IFJ - projekt
 * File: scanner.c
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
 *  @package scanner
 *  @file scanner.c
 *  @brief Scanner creates valid tokens for Parser. And check if all the inputs characters are language alphabet.
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */


#include "scanner.h"

// todo: create documentation
// todo: write tests, test || someone on dc will write tests

/**
 * @brief Covert state to string.
 *
 * @param s
 * @return String that represent state.
 */
static char *state_tostring(const int s) {
    switch (s) {
        #define X(nam) case STATE(nam): return #nam;
        STATES(X)
        #undef X
        default:
            assert(!"No such state...");
    }
}

// keyword pair. name e.g "else" and kwd e.g KEYWORD_else
typedef struct kypair {
    const char *nam;
    int kwd;
} kpar_t;


static int to_keyword(const char *identif) {
    static const kpar_t kwds[] = {
            #define X(n) { #n , KEYWORD(n) },
            KEYWORDS(X)
            #undef X
    };
    for (int i = 0; i < 9; i++)
        if (strcmp(kwds[i].nam, identif) == 0) {
            return kwds[i].kwd;
        }

    return TOKEN_ID;
}

/**
 * @brief Convert token to string.
 *
 * @param t token
 * @return String that represent token.
 */
static char *_to_string(const int t) {
    switch (t) {
        case TOKEN_EOFILE:
            return "END OF FILE";
        case TOKEN_WS:
            return " whitespace ";
        case TOKEN_EOL:
            return "END OF LINE";
        case TOKEN_ID:
            return "id";
        case TOKEN_NUM_I:
            return "integer number";
        case TOKEN_NUM_F:
            return "floating pointer number";
        case TOKEN_STR:
            return "string";
        case TOKEN_NE:
            return "~=";
        case TOKEN_EQ:
            return "==";
        case TOKEN_LT:
            return "<";
        case TOKEN_LE:
            return "<=";
        case TOKEN_GT:
            return ">";
        case TOKEN_GE:
            return ">=";
        case TOKEN_ASSIGN:
            return "=";
        case TOKEN_MUL:
            return "*";
        case TOKEN_DIV_F:
            return "/";
        case TOKEN_DIV_I:
            return "//";
        case TOKEN_ADD:
            return "+";
        case TOKEN_SUB:
            return "-";
        case TOKEN_LBRACE:
            return "}";
        case TOKEN_RBRACE:
            return "{";
        case TOKEN_LPAREN:
            return "(";
        case TOKEN_RPAREN:
            return ")";
        case TOKEN_COMMA:
            return ",";
//        case TOKEN_SEMICOLON:
//            return ";";
        case TOKEN_STRCAT:
            return "..";
        case TOKEN_COLON:
            return ":";
        case TOKEN_DEAD:
            return "DEAD TOKEN";

            #define X(name) case KEYWORD(name): return #name;
        KEYWORDS(X)
            #undef X
        default:
            return "unrecognized token";
    }
}


static size_t lines;
static int state;

/**
 * @brief finite automaton to lex a c string
 *
 * @param pfile
 * @return token. If STATE!= STATE_STR_FINAL returns TOKEN_DEAD.
 */
static token_t lex_string(progfile_t *pfile) {
    debug_msg(DEBUG_SEP);
    state = STATE_STR_INIT;
    int ch;
    uint8_t escaped_char;
    bool accepted = false;
    token_t token = { .type = TOKEN_STR };

    Dynstring.create_empty(&token.attribute.id);

    while (!accepted && ch != EOF) {
        ch = Progfile.pgetc(pfile);
        debug_msg("GOT: %c in state %s\n", ch, state_tostring(state));
        switch (state) {
            case STATE_STR_INIT:
                switch (ch) {
                    case '\\':
                        state = STATE_STR_ESC;
                        break;
                    case '"':
                        state = STATE_STR_FINAL;
                        accepted = true;
                        break;
                    case '\n':
                        accepted = true;
                    default:
                        Dynstring.append_char(&token.attribute.id, (char) ch);
                        break;
                }
                break;

            case STATE_STR_ESC:
                escaped_char = 0;
                switch (ch) {
                    case '0':
                        state = STATE_STR_DEC_0_0;
                        break;
                    case '1':
                        state = STATE_STR_DEC_0_1;
                        break;
                    case '2':
                        state = STATE_STR_DEC_0_2;
                        break;
                    case 't':
                        Dynstring.append_char(&token.attribute.id, '\n');
                        state = STATE_STR_INIT;
                        break;
                    case 'n':
                        Dynstring.append_char(&token.attribute.id, '\t');
                        state = STATE_STR_INIT;
                        break;
                    case_2('\\', '\"'):
                        Dynstring.append_char(&token.attribute.id, (char) ch);
                        state = STATE_STR_INIT;
                        break;
                    default:
                        accepted = true;
                        break;
                }
                escaped_char += (ch - '0') * 100;
                break;

            case STATE_STR_DEC_0_0:
                if (ch != 0 && isdigit(ch)) {
                    state = STATE_STR_DEC_1_1;
                } else if (ch == '0') {
                    state = STATE_STR_DEC_1_0;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0') * 10;
                break;

            case STATE_STR_DEC_0_1:
                if (isdigit(ch)) {
                    state = STATE_STR_DEC_1_1;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0') * 10;
                break;

            case STATE_STR_DEC_0_2:
                if (ch >= '0' && ch <= '4') {
                    state = STATE_STR_DEC_1_1;
                } else if (ch == '5') {
                    state = STATE_STR_DEC_1_2;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0') * 10;
                break;

            case STATE_STR_DEC_1_0:
                if (isdigit(ch) && ch != '0') {
                    state = STATE_STR_INIT;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0');
                break;

            case STATE_STR_DEC_1_1:
                if (isdigit(ch)) {
                    state = STATE_STR_INIT;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0');
                break;

            case STATE_STR_DEC_1_2:
                if (ch >= '0' && ch <= '5') {
                    state = STATE_STR_INIT;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0');
                break;

            default:
                assert(!"This statement mustn't occur!");
                break;
        }
    }
    debug_msg("GOT STRING: %s\n", Dynstring.c_str(&token.attribute.id));
    if (state != STATE_STR_FINAL) {
        token.type = TOKEN_DEAD;
        Dynstring.free(&token.attribute.id);
        debug_msg_stderr("ERROR while lexing a string\n");
    }

    return token;
}

/**
 * @brief Finite automaton to lex an identifier.
 * @param pfile
 * @return token. If state != STATE_ID_FINAL return TOKEN_DEAD
 */
static token_t lex_identif(progfile_t *pfile) {
    debug_msg(DEBUG_SEP);

    state = STATE_ID_INIT;
    token_t token = { .type = TOKEN_ID };
    Dynstring.create_empty(&token.attribute.id);
    int ch;
    bool accepted = false;

    while (!accepted && ch != EOF) {
        ch = Progfile.pgetc(pfile);
        switch (state) {
            case STATE_ID_INIT:
                if (isalpha(ch) || ch == '_') {
                    Dynstring.append_char(&token.attribute.id, (char) ch);
                    state = STATE_ID_FINAL;
                } else {
                    accepted = true;
                }
                break;

            case STATE_ID_FINAL:
                if (isalnum(ch) || ch == '_') {
                    Dynstring.append_char(&token.attribute.id, (char) ch);
                } else {
                    accepted = true;
                    Progfile.ungetc(pfile);
                }
                break;

            default:
                break;
        }
    }

    if (state != STATE_ID_FINAL) {
        Dynstring.free(&token.attribute.id);
        token.type = TOKEN_DEAD;
    }

    debug_msg("identif: %s", Dynstring.c_str(&token.attribute.id));
    // this 2 lines of code make parsing much more easier
    if ((token.type = to_keyword(Dynstring.c_str(&token.attribute.id))) != TOKEN_ID) {
        Dynstring.free(&token.attribute.id);
        debug_msg_s("- keyword!\n");
    } else
        debug_msg_s("\n");

    return token;
}

/**
 * @brief Process a comment.
 *  [-][-].* for a single line comment.
 *  [-][-]\[\[.*\]\] for a block comment.
 * @param pfile
 * @return false if comment is wrong
 */
static bool process_comment(progfile_t *pfile) {
    debug_msg(DEBUG_SEP);
    state = STATE_COMMENT_INIT;
    bool accepted = false;
    int ch;

    // != EOF is not a true "DFA" way of thinking, but it is more clearly
    while (!accepted && ch != EOF) {
        ch = Progfile.pgetc(pfile);
        if (ch == '\n') {
            lines++;
        }
        switch (state) {
            case STATE_COMMENT_INIT:
                if (ch == '[') {
                    state = STATE_COMMENT_BLOCK_1; // block comment
                } else if (ch == '\n') {
                    state = STATE_COMMENT_FINAL; // an empty comment
                } else {
                    state = STATE_COMMENT_SLINE;
                } // an order comment
                break;
            case STATE_COMMENT_SLINE:
                if (ch == '\n')
                    state = STATE_COMMENT_FINAL; // comment end
                break;
            case STATE_COMMENT_BLOCK_1:
                if (ch == '[')
                    state = STATE_COMMENT_BLOCK_2;
                else
                    state = STATE_COMMENT_SLINE; // an order comment
                break;
            case STATE_COMMENT_BLOCK_2:
                if (ch == ']')
                    state = STATE_COMMENT_BLOCK_END; // end block comment 1.
                break;
            case STATE_COMMENT_BLOCK_END:
                if (ch == ']')
                    state = STATE_COMMENT_FINAL; // comment done
                break;
            case STATE_COMMENT_FINAL: // more clear way of creating an automaton
                Progfile.ungetc(pfile);
                accepted = true;
                break;
            default:
                assert(!"This statement must not occur!");
                break;
        }
    }

    return state == STATE_COMMENT_FINAL;
}

// >= <= == ~= < >
/**
 * @brief DFA accepted relational operators
 *
 * @param pfile
 * @return (token_t) (.type = actual_state)
 */
static token_t lex_relate_op(progfile_t *pfile) {
    debug_msg(DEBUG_SEP);
    token_t token;
    bool accepted = false;
    int ch;

    while (!accepted) {
        ch = Progfile.pgetc(pfile);
        switch (ch) {
            case '>':
                return Progfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_GE} : (Progfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_GT});
            case '<':
                return Progfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_LE} : (Progfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_LT});
            case '=':
                return Progfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_EQ} : (Progfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_ASSIGN});
            case '~':
                return Progfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_EQ} : (Progfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_DEAD});
            default:
                assert(!"Statement must not occur!");
        }
    }
    return (token_t) {.type = TOKEN_DEAD};
}

/**
 * @brief DFA accepts the number
 *
 * @param pfile
 * @return token
 */
static token_t lex_number(progfile_t *pfile) {
    debug_msg(DEBUG_SEP);
    state = STATE_NUM_INIT;
    string ascii_num;
    Dynstring.create_empty(&ascii_num);

    bool is_fp = false;
    int ch;
    bool accepted = false;

    while (!accepted && ch != EOF) {
        ch = Progfile.pgetc(pfile);
        switch (state) {
            case STATE_NUM_INIT:
                if (ch == '0') {
                    state = STATE_NUM_ZERO;
                } else if (isdigit(ch)) {
                    state = STATE_NUM_INT;
                } else {
                    accepted = true;
                    break;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            case STATE_NUM_INT: // final
                if (isdigit(ch)) { ;
                } else if (ch == '.') {
                    state = STATE_NUM_F_DOT;
                } else if (ch == 'e' || ch == 'E') {
                    state = STATE_NUM_EXP;
                } else { // integer number has been taken
                    state = STATE_NUM_FINAL;
                    accepted = true;
                    break;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            case STATE_NUM_ZERO:
                if (ch == '0') {
                    state = STATE_NUM_ZERO_TRANSITION;
                } else if (ch == '.') {
                    state = STATE_NUM_F_DOT;
                } else { // can get zero here , e.g '0'
                    state = STATE_NUM_FINAL;
                    accepted = true;
                    break;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            case STATE_NUM_ZERO_TRANSITION: // final - number can be 00, and it is an integer. Otherwise 8ing for fp part
                if (isdigit(ch)) {
                    state = STATE_NUM_ZERO_ZERO;
                } else if (ch == '.') {
                    state = STATE_NUM_F_DOT;
                } else {
                    state = STATE_NUM_FINAL;
                    accepted = true;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            case STATE_NUM_F_DOT:
                if (isdigit(ch)) {
                    state = STATE_NUM_F_PART;
                    Dynstring.append_char(&ascii_num, (char) ch);
                } else {
                    accepted = true;
                }
                break;

            case STATE_NUM_F_PART: // final
                is_fp = true;
                if (isdigit(ch)) {}
                else if (ch == 'e' || ch == 'E') {
                    state = STATE_NUM_EXP;
                } else { // now we got a character which signifies that there will be no more 'numberness'
                    state = STATE_NUM_FINAL;
                    accepted = true;
                    break;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            case STATE_NUM_EXP:
                is_fp = true;
                if (isdigit(ch)) {
                    state = STATE_NUM_DOUBLE;
                } else if (ch == '-' || ch == '+') {
                    state = STATE_NUM_EXP_SIGN;
                } else {
                    accepted = true;
                    break;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;
            case STATE_NUM_EXP_SIGN:
                is_fp = true;
                if (isdigit(ch)) {
                    state = STATE_NUM_EXP_SIGN;
                    Dynstring.append_char(&ascii_num, (char) ch);
                } else {
                    accepted = true;
                }
                break;
            case STATE_NUM_DOUBLE: // final
                is_fp = true;
                if (isdigit(ch)) {}
                else {
                    state = STATE_NUM_FINAL;
                    accepted = true;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            case STATE_NUM_ZERO_ZERO:
                is_fp = true;
                if (isdigit(ch)) {}
                else if (ch == '.') {
                    state = STATE_NUM_F_PART;
                } else {
                    accepted = true;
                    break;
                }
                Dynstring.append_char(&ascii_num, (char) ch);
                break;

            default:
                debug_assert(!"Error in dead state while lexing number!!!!");
                break;
        }
    }
    if (state != STATE_NUM_FINAL) {
        Dynstring.free(&ascii_num);
        return (token_t) {.type = TOKEN_DEAD};
    }

    token_t token;

    if (is_fp) {
        token.type = TOKEN_NUM_F;
        token.attribute.num_f = strtod(Dynstring.c_str(&ascii_num), NULL);
    } else {
        token.type = TOKEN_NUM_I;
        token.attribute.num_i = strtoull(Dynstring.c_str(&ascii_num), NULL, 10);
    }
    Dynstring.free(&ascii_num);

    return token;
}

/**
 * @brief returns one token, in case of a lexical error returned token is TOKEN_DEAD
 *
 * @param pfile
 * @return token
 */
static token_t scanner(progfile_t *pfile) {
    debug_msg(DEBUG_SEP);
    int ch;
    token_t token = {0, };

    next_lexeme:
    state = STATE_INIT;
    ch = Progfile.pgetc(pfile);

    switch (ch) {
        case '\n':
            lines++;
            goto next_lexeme;
            break;

        case_2('\t', ' '):
            token.type = TOKEN_WS;
            break;

        case_alpha:
        case '_': // alpha and underscore
            Progfile.ungetc(pfile);
            token = lex_identif(pfile);
            break;

        case_digit:
            Progfile.ungetc(pfile);
            token = lex_number(pfile);
            break;

        case '-':
            if (Progfile.peek_at(pfile, 0) == '-') {
                Progfile.pgetc(pfile);
                if (!process_comment(pfile)) {
                    return (token_t) {.type = TOKEN_DEAD};
                }
                goto next_lexeme;
            }
            token.type = TOKEN_SUB;
            break;

        case '/':
            token.type = TOKEN_DIV_F;
            if (Progfile.peek_at(pfile, 0) == '/') {
                Progfile.pgetc(pfile);
                token.type = TOKEN_DIV_I;
            }
            break;

        case_4('>', '<', '=', '~'):
            Progfile.ungetc(pfile); // move tape back to call a function to process an operator.
            token = lex_relate_op(pfile);
            break;

        case '#':
            token.type = TOKEN_STRCAT;
            break;
        case '\"':
            token = lex_string(pfile);
            break;
        case '*':
            token.type = TOKEN_MUL;
            break;
        case '+':
            token.type = TOKEN_ADD;
            break;
        case '{':
            token.type = TOKEN_LBRACE;
            break;
        case '}':
            token.type = TOKEN_RBRACE;
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            break;
        case ',':
            token.type = TOKEN_COMMA;
            break;
//        case ';': //not supported
//            token.type = TOKEN_SEMICOLON;
//            break;
        case '.':
            token.type = TOKEN_DEAD;
            if (Progfile.pgetc(pfile) == '.') {
                token.type = TOKEN_STRCAT;
            }
            break;
        case ':':
            token.type = TOKEN_COLON;
            break;
        case EOF:
            token.type = TOKEN_EOFILE;
            break;
        default:
            debug_msg("UNKNOWN CHARACTER: %c\n", ch);
            token.type = TOKEN_DEAD;
            break;
    }

    if (token.type == TOKEN_DEAD) {
        assert(!"Handle errors"); // todo: handle errors
    }

    return token;
}


// ==============================================
/**
 * @brief Initialize scanner
 *
 * @param
 * @return File
 */
static progfile_t *_initialize_scanner() {
    return Progfile.getfile_stdin();
}

/**
 * @brief Free token.
 *
 * @param token
 * @return void
 */
static void _free_token(token_t *token) {
    if (token->type == TOKEN_ID || token->type == TOKEN_STR) {
        Dynstring.free(&token->attribute.id);
    }
}


// ==============================================
static token_t prev, curr;

/**
 * @brief Gets next token. and move to next one.
 *
 * @param pfile
 * @return token
 */
static token_t _get_next_token(progfile_t *pfile) {
    _free_token(&prev); // need to free string
    prev = curr;
    curr = scanner(pfile);
    return curr;
}

/**
 * @brief Get previous token.
 *
 * @return previous token
 */
static token_t _get_prev_token() {
    return prev;
}

/**
 * @brief Get current token.
 *
 * @param
 * @return current token.
 */
static token_t _get_curr() {
    return curr;
}

/**
 * @brief Free heap memory allocated by scanner
 *
 * @param pfile
 * @return void
 */
// ==============================================
static void _free_scanner(progfile_t *pfile) {
    Dynstring.free(&prev.attribute.id);
    Dynstring.free(&curr.attribute.id);
    Progfile.free(pfile);
}

/**
 *
 * Scanner interface.
 */
const struct scanner_op_struct Scanner = {
        .free = _free_scanner,
        .get_next_token = _get_next_token,
        .get_prev_token = _get_prev_token,
        .initialize = _initialize_scanner,
        .get_curr_token = _get_curr,
        .to_string = _to_string
};
