/**
 * @file scanner.c
 *
 * @brief Lexical analyser. Implemented as DFA matching regular expressions.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#include "scanner.h"


/** Previous token is used for error handling, if I will not give a d&ck.
 */
static token_t prev;

/** Current token of the program.
 */
static token_t curr;

/** Current line of the program.
 */
static size_t lines = 1;

/** Current state of the dfa.
 */
static int state = STATE_INIT;

/** Current character position in the line
 */
static size_t charpos;


#define is_hexnumber(ch) (((ch) >= '0' && (ch) <= '9') || ((ch) >= 'A' && (ch) <= 'F') || ((ch) >= 'a' && (ch) <= 'f'))
#define hex2dec(ch) ((uint8_t)((ch) -(((ch) > '9') ? (-10 + (((ch) > 'Z' ) ? 'a': 'A')): '0')))


/** Covert state to string.
 *
 * @param s state
 * @return String that represent state.
 */
static char *state_tostring(const int s) {
    switch (s) {
        #define X(nam) case STATE(nam): return #nam;
        STATES(X)
        #undef X
        default:
            soft_assert(false, ERROR_INTERNAL);
    }
}

/** Structure for converting to keywords.
 * keyword pair. name e.g "else" and kwd e.g KEYWORD_else
 */
typedef struct kypair {
    const char *nam;
    int kwd;
} kpar_t;

/** Convert an identifier to keyword.
 *
 * @param identif to be coverted.
 * @return keyword.
 */
static int to_keyword(const char *identif) {
    static const kpar_t kwds[] = {
            #define X(n) { #n , KEYWORD(n) },
            KEYWORDS(X)
            #undef X
            {NULL, 0} // the end
    };

    for (int i = 0; kwds[i].nam != NULL; i++)
        if (strcmp(kwds[i].nam, identif) == 0) {
            return kwds[i].kwd;
        }

    return TOKEN_ID;
}

/** Pretty print. Token to string.
 *
 * @param t token
 * @return String that represent token.
 */
static char *To_string(const int t) {
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
        case TOKEN_LPAREN:
            return "(";
        case TOKEN_RPAREN:
            return ")";
        case TOKEN_COMMA:
            return ",";
        case TOKEN_STRCAT:
            return "..";
        case TOKEN_COLON:
            return ":";
        case TOKEN_HASH:
            return "#";
        case TOKEN_DEAD:
            return "DEAD TOKEN";

            #define X(name) case KEYWORD(name): return #name;
        KEYWORDS(X)
            #undef X
        default:
            return "unrecognized token";
    }
}

/**
 * @brief finite automaton to lex a c string
 *
 * @param pfile
 * @return token. If STATE!= STATE_STR_FINAL returns TOKEN_DEAD.
 */
static token_t lex_string(pfile_t *pfile) {
    state = STATE_STR_INIT;
    int ch;
    uint8_t escaped_char;
    bool accepted = false;
    token_t token = {.type = TOKEN_STR, .attribute.id = Dynstring.ctor("")};

    while (!accepted && (ch = Pfile.pgetc(pfile)) != EOF) {
        charpos++;
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
                        Dynstring.append(token.attribute.id, (char) ch);
                        break;
                }
                break;

            case STATE_STR_ESC:
                escaped_char = 0;
                switch (ch) {
                    case '0':
                        escaped_char += (ch - '0') * 100;
                        state = STATE_STR_DEC_0_0;
                        break;
                    case '1':
                        escaped_char += (ch - '0') * 100;
                        state = STATE_STR_DEC_0_1;
                        break;
                    case '2':
                        escaped_char += (ch - '0') * 100;
                        state = STATE_STR_DEC_0_2;
                        break;
                    case 't':
                        Dynstring.append(token.attribute.id, '\n');
                        state = STATE_STR_INIT;
                        break;
                    case 'n':
                        Dynstring.append(token.attribute.id, '\t');
                        state = STATE_STR_INIT;
                        break;
                    case 'x':
                        state = STATE_STR_HEX_1;
                        break;
                    case_2('\\', '\"'):
                        Dynstring.append(token.attribute.id, (char) ch);
                        state = STATE_STR_INIT;
                        break;
                    default:
                        accepted = true;
                        break;
                }
                break;

            case STATE_STR_HEX_1:
                if (!is_hexnumber(ch)) {
                    accepted = true;
                } else if (ch == '0') {
                    state = STATE_STR_HEX_3;
                } else {
                    state = STATE_STR_HEX_2;
                }
                escaped_char = hex2dec(ch) << 4;
                break;

                // \x0
            case STATE_STR_HEX_3:
                if (!is_hexnumber(ch) || ch == '0') {
                    accepted = true;
                }
                escaped_char |= hex2dec(ch);
                state = STATE_STR_INIT;
                break;

                // \x^[0]
            case STATE_STR_HEX_2:
                if (!is_hexnumber(ch)) {
                    accepted = true;
                }
                escaped_char |= hex2dec(ch);
                Dynstring.append(token.attribute.id, (char) escaped_char);
                state = STATE_STR_INIT;
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
                Dynstring.append(token.attribute.id, (char) escaped_char);
                break;

            case STATE_STR_DEC_1_1:
                if (isdigit(ch)) {
                    state = STATE_STR_INIT;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0');
                Dynstring.append(token.attribute.id, (char) escaped_char);
                break;

            case STATE_STR_DEC_1_2:
                if (ch >= '0' && ch <= '5') {
                    state = STATE_STR_INIT;
                } else {
                    accepted = true;
                }
                escaped_char += (ch - '0');
                Dynstring.append(token.attribute.id, (char) escaped_char);
                break;

            default:
                accepted = true;
                break;
        }
    }
    if (state != STATE_STR_FINAL) {
        token.type = TOKEN_DEAD;
        Dynstring.dtor(token.attribute.id);
    }

    return token;
}

/** Finite automaton to lex an identifier.
 *
 * @param pfile
 * @return token. If state != STATE_ID_FINAL return TOKEN_DEAD
 */
static token_t lex_identif(pfile_t *pfile) {

    state = STATE_ID_INIT;
    int ch;
    bool accepted = false;
    charpos--;

    token_t token = {.type = TOKEN_ID, .attribute.id = Dynstring.ctor("")};

    while (!accepted && (ch = Pfile.pgetc(pfile)) != EOF) {
        charpos++;
        switch (state) { // an e transition between INIT and STATE_IT_INIT, because it have to be in the separate function
            case STATE_ID_INIT:
                if (isalpha(ch) || ch == '_') {
                    Dynstring.append(token.attribute.id, (char) ch);
                    state = STATE_ID_FINAL;
                } else {
                    accepted = true;
                }
                break;

            case STATE_ID_FINAL: // so actually there has to be only one identifier state in the dfa
                if (isalnum(ch) || ch == '_') {
                    Dynstring.append(token.attribute.id, (char) ch);
                } else {
                    accepted = true;
                    Pfile.ungetc(pfile);
                }
                break;

            default:
                break;
        }
    }

    if (state != STATE_ID_FINAL) {
        Dynstring.dtor(token.attribute.id);
        token.type = TOKEN_DEAD;
    }

    // this 2 lines of code make parsing much more easier
    if ((token.type = to_keyword(Dynstring.c_str(token.attribute.id))) != TOKEN_ID) {
        Dynstring.dtor(token.attribute.id);
    }

    return token;
}

/** Process a comment.
 *
 * @param pfile
 * @return false if comment is wrong
 */
static bool process_comment(pfile_t *pfile) {
    state = STATE_COMMENT_INIT;
    bool accepted = false;
    int ch;
    charpos--;

    // != EOF is not a true "DFA" way of thinking, but it is more clearly
    while (!accepted && (ch = Pfile.pgetc(pfile)) != EOF) {
        charpos++;
        if (ch == '\n') {
            lines++;
            charpos = 0;
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
                if (ch == '\n') {
                    state = STATE_COMMENT_FINAL;
                } // comment end
                break;

            case STATE_COMMENT_BLOCK_1:
                if (ch == '[') {
                    state = STATE_COMMENT_BLOCK_2;
                } else {
                    state = STATE_COMMENT_SLINE;
                } // an order comment
                break;

            case STATE_COMMENT_BLOCK_2:
                if (ch == ']') {
                    state = STATE_COMMENT_BLOCK_END;
                } // end block comment 1.
                break;

            case STATE_COMMENT_BLOCK_END:
                if (ch == ']')
                    state = STATE_COMMENT_FINAL; // comment done
                break;

            case STATE_COMMENT_FINAL: // more clear way of creating an automaton
                Pfile.ungetc(pfile);
                accepted = true;
                break;

            default:
                accepted = true;
                break;
        }
    }

    return state == STATE_COMMENT_FINAL;
}

/** DFA accepted relational operators
 *
 *  >= <= == ~= < >
 *
 * @param pfile
 * @return (token_t) (.type = actual_state)
 */
static token_t lex_relate_op(pfile_t *pfile) {
    bool accepted = false;
    int ch;

    while (!accepted) {
        ch = Pfile.pgetc(pfile);
        charpos++;
        switch (ch) {
            case '>':
                charpos++;
                return Pfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_GE} : (Pfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_GT});
            case '<':
                charpos++;
                return Pfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_LE} : (Pfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_LT});
            case '=':
                charpos++;
                return Pfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_EQ} : (Pfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_ASSIGN});
            case '~':
                charpos++;
                return Pfile.pgetc(pfile) == '=' ? (token_t) {.type = TOKEN_NE} : (Pfile.ungetc(
                        pfile), (token_t) {.type = TOKEN_DEAD});
            default:
                break;
        }
    }
    return (token_t) {.type = TOKEN_DEAD};
}

/** DFA accepts numbers.
 *
 * @param pfile a program.
 * @return token
 */
static token_t lex_number(pfile_t *pfile) {
    state = STATE_NUM_INIT;
    dynstring_t *ascii_num = Dynstring.ctor(""); // ctor an empty string

    bool is_fp = true; // suppose it is true.
    int ch;
    bool accepted = false;
    charpos--;

    while (!accepted && (ch = Pfile.pgetc(pfile)) != EOF) {
        charpos++;
        switch (state) {
            case STATE_NUM_INIT:
                if (ch == '0') {
                    state = STATE_NUM_1;
                } else if (isdigit(ch)) {
                    state = STATE_NUM_2;
                } else {
                    accepted = true;
                    break;
                }
                Dynstring.append(ascii_num, (char) ch);
                break;

                // we got here from number init state and there was 0.
                // final, got 0.
            case STATE_NUM_1:
                // append char is after the statement, so an empty statement is here.
                if (isdigit(ch)) {
                    state = STATE_NUM_3;
                    Dynstring.append(ascii_num, (char) ch);
                } else if (ch == '.') {
                    state = STATE_NUM_7;
                    Dynstring.append(ascii_num, (char) ch);
                } else {
                    // null has been accepted, so it is a legit integer.
                    is_fp = false;
                    accepted = true;
                    state = STATE_NUM_FINAL;
                }
                break;

                // 1-9 has been accepted, so is is an integer now.
            case STATE_NUM_2:
                if (isdigit(ch)) {
                    // append character after if statement.
                    ;;
                } else if (ch == '.') {
                    state = STATE_NUM_7;
                } else if (ch == 'e' || ch == 'E') {
                    state = STATE_NUM_5;
                } else {
                    is_fp = false;
                    accepted = true;
                    state = STATE_NUM_FINAL;
                    break;
                }
                Dynstring.append(ascii_num, (char) ch);
                break;

                // tro zeros have been accepted.
            case STATE_NUM_3:
                if (isdigit(ch)) {
                    state = STATE_NUM_4;
                } else if (ch == '.') {
                    state = STATE_NUM_7;
                } else {
                    is_fp = false;
                    state = STATE_NUM_FINAL;
                    accepted = true;
                    break;
                }
                Dynstring.append(ascii_num, (char) ch);
                break;

            case STATE_NUM_4:
                if (isdigit(ch)) { ;;
                } else if (ch == '.') {
                    state = STATE_NUM_7;
                } else if (ch == 'e' || ch == 'E') {
                    // got an exponent
                    state = STATE_NUM_5;
                } else {
                    accepted = true;
                    break;
                }
                Dynstring.append(ascii_num, (char) ch);
                break;

            case STATE_NUM_5:
                if (isdigit(ch)) {
                    state = STATE_NUM_9;
                    Dynstring.append(ascii_num, (char) ch);
                } else if (ch == '+' || ch == '-') {
                    state = STATE_NUM_6;
                    Dynstring.append(ascii_num, (char) ch);
                } else {
                    accepted = true;
                }
                break;

            case STATE_NUM_6:
                if (isdigit(ch)) {
                    state = STATE_NUM_9;
                    Dynstring.append(ascii_num, (char) ch);
                } else {
                    accepted = true;
                }
                break;

            case STATE_NUM_7:
                if (isdigit(ch)) {
                    state = STATE_NUM_8;
                    Dynstring.append(ascii_num, (char) ch);
                } else {
                    accepted = true;
                }
                break;

            case STATE_NUM_8:
                if (isdigit(ch)) {
                    Dynstring.append(ascii_num, (char) ch);
                } else if (ch == 'e' || ch == 'E') {
                    Dynstring.append(ascii_num, (char) ch);
                    state = STATE_NUM_5;
                } else {
                    accepted = true;
                    state = STATE_NUM_FINAL;
                }
                break;

            case STATE_NUM_9:
                if (isdigit(ch)) {
                    Dynstring.append(ascii_num, (char) ch);
                } else {
                    accepted = true;
                    state = STATE_NUM_FINAL;
                }
                break;

            default:
                break;
        }
    }
    if (state != STATE_NUM_FINAL) {
        Dynstring.dtor(ascii_num);
        return (token_t) {.type = TOKEN_DEAD};
    }

    Pfile.ungetc(pfile);
    token_t token;

    // TODO: actually we can get rid of it and left number as string but with market that its an integer or a float.
    if (is_fp) {
        token.type = TOKEN_NUM_F;
        token.attribute.num_f = strtod(Dynstring.c_str(ascii_num), NULL);
    } else {
        token.type = TOKEN_NUM_I;
        token.attribute.num_i = strtoull(Dynstring.c_str(ascii_num), NULL, 10);
    }
    Dynstring.dtor(ascii_num);

    return token;
}

/** Returns one token, in case of a lexical error_interface returned token is TOKEN_DEAD
 *
 * @param pfile program file.
 * @return token token of the program.
 */
static token_t scanner(pfile_t *pfile) {
    int ch;
    token_t token = {0,};

    next_lexeme:
    charpos++;
    state = STATE_INIT;
    ch = Pfile.pgetc(pfile);

    switch (ch) {
        #define X(a) case TOKEN(a): token.type = TOKEN(a); break;
        SINGLE_CHAR_TOKENS(X)
        #undef X

        case_2('\n', 13):
            lines++;
            charpos = 0;
            goto next_lexeme;
            break;

        case_2('\t', ' '):
            charpos += (ch == '\t') ? 3 : 0;
            goto next_lexeme;
            break;

        case_alpha:
        case '_': // alpha and underscore
            Pfile.ungetc(pfile);
            token = lex_identif(pfile);
            break;

        case_digit:
            Pfile.ungetc(pfile);
            token = lex_number(pfile);
            break;

        case '-':
            if (Pfile.peek_at(pfile, 0) == '-') {
                Pfile.pgetc(pfile);
                if (!process_comment(pfile)) {
                    token = (token_t) {.type = TOKEN_DEAD};
                    break;
                }
                goto next_lexeme;
            } else if (isalnum(Pfile.peek_at(pfile, 0))) {
                token = lex_number(pfile); // negative number
            } else {
                token.type = TOKEN_SUB;
            }
            break;

        case '/':
            token.type = TOKEN_DIV_F;
            if (Pfile.peek_at(pfile, 0) == '/') {
                Pfile.pgetc(pfile);
                charpos++;
                token.type = TOKEN_DIV_I;
            }
            break;

        case_4('>', '<', '=', '~'):
            Pfile.ungetc(pfile); // move tape back to call a function to process an operator.
            token = lex_relate_op(pfile);
            break;

        case '\"':
            token = lex_string(pfile);
            break;

        case '.':
            // there's no symbol such as '.' in the language
            token.type = TOKEN_DEAD;
            if (Pfile.pgetc(pfile) == '.') {
                charpos++;
                token.type = TOKEN_STRCAT;
                break;
            }
            break;

        default:
            token.type = TOKEN_DEAD;
            debug_msg("unrecognized ch with ascii '%d'\n", ch);
            break;
    }
    if (token.type == TOKEN_DEAD) {
        Errors.set_error(ERROR_LEXICAL);
    }

    return token;
}

/** Free token.
 *
 * @param token token to be freed.
 * @return void
 */
static void Free_token(token_t *token) {
    if (token->type == TOKEN_ID || token->type == TOKEN_STR) {
        Dynstring.dtor(token->attribute.id);
    }
}

/** Gets next token. and move to next one.
 *
 * @param pfile
 * @return token
 */
static token_t Get_next_token(pfile_t *pfile) {
    Free_token(&prev); // need to dtor string
    prev = curr;
    curr = scanner(pfile);
    return curr;
}

/** Get previous token.
 *
 * @return previous token
 */
static token_t Get_prev_token() {
    return prev;
}

/** Get current token.
 *
 * @return current token.
 */
static token_t Get_curr() {
    return curr;
}

/** Get current line of the file.
 *
 * @return current line
 */
static size_t Get_line() {
    return lines;
}

/** Get current line of the file.
 *
 * @return current line
 */
static size_t Get_charpos() {
    return charpos;
}

/** Free memory allocated by scanner
 *
 * @param pfile prorgram file.
 * @return void
 */
static void Free_scanner() {
    if (prev.type == TOKEN_ID || prev.type == TOKEN_STR) {
        Dynstring.dtor(prev.attribute.id);
    }
    if (curr.type == TOKEN_ID || curr.type == TOKEN_STR) {
        Dynstring.dtor(curr.attribute.id);
    }
}

/** Scanner initialization.
 */
static void Init_scanner() {
    memset(&prev, 0x0, sizeof(prev));
    memset(&curr, 0x0, sizeof(prev));
}


const struct scanner_interface Scanner = {
        .free = Free_scanner,
        .get_next_token = Get_next_token,
        .get_curr_token = Get_curr,
        .to_string = To_string,
        .get_line = Get_line,
        .get_charpos = Get_charpos,
        .init = Init_scanner,
};


#ifdef SELFTEST_scanner
#include "tests/tests.h"

int main() {
    token_t token;

    //********************************************************************//
    //****************************** NUMBERS *****************************//

    // floats ok
    printf("Test 1 - float numbers, good\n");

    pfile_t *pfile = Pfile.getfile("../tests/number_ok.tl", "r");
    if (!pfile) {
        goto nexttest;
    }
    for (int nu = 0; (token = Scanner.get_next_token(pfile)).type != TOKEN_EOFILE; nu++) {
        if (token.type != TOKEN_NUM_F) {
            Tests.failed("expected: got: with attribute:\n");
        } else {
            Tests.passed("%d\n", nu);
        }
    }
    if (0) {
        nexttest:
        Tests.failed("Cannot open the file!\n");
    }
    Pfile.dtor(pfile);

    // integer ok
    printf("Test 3 - integer numbers, good\n");
    pfile = Pfile.getfile("../tests/integer_ok.tl", "r");
    if (!pfile) {
        goto nexttest3;
    }
    for (int nu = 0; (token = Scanner.get_next_token(pfile)).type != TOKEN_EOFILE; nu++) {
        if (token.type != TOKEN_NUM_I) {
            Tests.failed("expected: got: with attribute:\n");
        } else {
            Tests.passed("%d\n", nu);
        }// green [PASSED]
    }
    if (0) {
        nexttest3:
        Tests.failed("Cannot open the file!\n");
    }
    Pfile.dtor(pfile);


    // identifier ok
    printf("Test 5 - identifiers, good\n");
    pfile = Pfile.getfile("../tests/identif_ok.tl", "r");
    if (!pfile) {
        goto nexttest5;
    }
    for (int nu = 0; (token = Scanner.get_next_token(pfile)).type != TOKEN_EOFILE; nu++) {
        if (token.type != TOKEN_ID) {
            Tests.failed("expected: got: with attribute:\n");
        } else {
            Tests.passed("%d\n", nu);
        }// green [PASSED]
    }
    if (0) {
        nexttest5:
        Tests.failed("Cannot open the file!\n");
    }
    Pfile.dtor(pfile);

    // strings good
    printf("Test 7 - strings, good\n");
    pfile = Pfile.getfile("../tests/string_ok.tl", "r");
    if (!pfile) {
        goto nexttest7;
    }
    for (int nu = 0; (token = Scanner.get_next_token(pfile)).type != TOKEN_EOFILE; nu++) {
        if (token.type != TOKEN_STR) {
            Tests.failed("expected: got: with attribute:\n");
        } else {
            Tests.passed("%d\n", nu);
        }// green [PASSED]
    }
    if (0) {
        nexttest7:
        Tests.failed("Cannot open the file!\n");
    }
    Pfile.dtor(pfile);

    return 0;
}

#endif
