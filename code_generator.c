#include "code_generator.h"

/*
 * @brief   Generates built-in function reads().
 *          function reads() : string
 *          result: %res
 */
static void generate_func_reads() {
    ADD_INSTR(  "LABEL $READS \n"                 \
                "PUSHFRAME \n"                    \
                "DEFVAR LF@%res \n"               \
                "READ LF@%res string \n"          \
                "POPFRAME \n"                     \
                "RETURN \n");
}

/*
 * @brief   Generates built-in function readi().
 *          function readi() : integer
 *          result: %res
 */
static void generate_func_readi() {
    ADD_INSTR(  "LABEL $READI \n"                 \
                "PUSHFRAME \n"                    \
                "DEFVAR LF@%res \n"               \
                "READ LF@%res int \n"             \
                "POPFRAME \n"                     \
                "RETURN \n");
}

/*
 * @brief   Generates built-in function readn().
 *          function readn() : integer
 *          result: %res
 */
static void generate_func_readn() {
    ADD_INSTR(  "LABEL $READN \n"                 \
                "PUSHFRAME \n"                    \
                "DEFVAR LF@%res \n"               \
                "READ LF@%res float \n"           \
                "POPFRAME \n"                     \
                "RETURN \n");
}

/*
 * FIXME
 *      function write (term1, term2, ..., termn)
 *      right now it prints only one term
 */
/*
 * @brief   Generates built-in function write().
 *          function write (term1, term2, ..., termn)
 *          params: FIXME
 */
static void generate_func_write() {
    ADD_INSTR(  "LABEL $WRITE \n"                 \
                "PUSHFRAME \n"                    \
                "DEFVAR LF@p0 \n"                 \
                "MOVE LF@p0 LF@%0 \n"             \
                "WRITE LF@p0 \n"                  \
                "POPFRAME \n"                     \
                "RETURN \n");
}

/*
 * @brief   Generates built-in function tointeger().
 *          function tointeger(f : number) : integer
 *          params: f ... %0
 *          result: %res
 */
static void generate_func_tointeger() {
    ADD_INSTR(  "LABEL $TOINTEGER \n"             \
                "PUSHFRAME \n"                    \
                "DEFVAR LF@%res \n"               \
                "DEFVAR LF@p0 \n"                 \
                "MOVE LF@p0 LF@%0 \n"             \
                "FLOAT2INT LF@%res LF@p0 \n"      \
                "POPFRAME \n"                     \
                "RETURN \n");

}

/*
 * @brief   Generates built-in function chr().
 *          function chr(i : integer) : string
 *          params: i ... %0
 *          result: %res
 */
static void generate_func_chr() {
        ADD_INSTR(  "LABEL $CHR \n"                                     \
                "PUSHFRAME \n"                                          \
                "DEFVAR LF@%res \n"                                     \
                "MOVE LF@%res nil@nil \n"                               \
                "DEFVAR LF@p0 \n"                                       \
                "MOVE LF@p0 LF@%0 \n"                                   \
                "DEFVAR LF@check \n"                                    \
                "LT LF@check LF@p0 int@0 \n"                            \
                "JUMPIFEQ $CHR$end LF@check bool@true \n"               \
                "GT LF@check LF@p0 int@255 \n"                          \
                "JUMPIFEQ $CHR$end LF@check bool@true \n"               \
                "INT2CHAR LF@%res LF@p0 \n"                             \
                "LABEL $CHR$end \n"                                     \
                "POPFRAME \n"                                           \
                "RETURN \n");
}

/*
 * @brief   Generates built-in function ord().
 *          function ord(s : string, i : integer) : integer
 *          params: s ... %0, i ... %1
 *          result: %res
 */
static void generate_func_ord() {
    ADD_INSTR(  "LABEL $ORD \n"                                         \
                "PUSHFRAME \n"                                          \
                "DEFVAR LF@%res \n"                                     \
                "MOVE LF@%res nil@nil \n"                               \
                "DEFVAR LF@p0 \n"                                       \
                "MOVE LF@p0 LF@%0 \n"                                   \
                "DEFVAR LF@p1 \n"                                       \
                "MOVE LF@p1 LF@%1 \n"                                   \
                "DEFVAR LF@str_len \n"                                  \
                "STRLEN LF@str_len LF@p0 \n"                            \
                "DEFVAR LF@check \n"                                    \
                "LT LF@check LF@p1 int@1 \n"                            \
                "JUMPIFEQ $ORD$end LF@check bool@true \n"               \
                "GT LF@check LF@p1 LF@str_len \n"                       \
                "JUMPIFEQ $ORD$end LF@check bool@true \n"               \
                "SUB LF@p1 LF@p1 int@1 \n"                              \
                "STRI2INT LF@%res LF@p0 LF@p1 \n"                       \
                "LABEL $ORD$end \n"                                     \
                "POPFRAME \n"                                           \
                "RETURN \n");
}

/*
 * @brief   Generates built-in function substr().
 *          function substr(s : string, i : number, j : number) : string
 *          params: s ... %0, i ... %1, j ... %2
 *          result: %res
*/
static void generate_func_substr() {
    ADD_INSTR(  "LABEL $SUBSTR \n"                                      \
                "PUSHFRAME \n"                                          \
                "# define vars for params and result\n"                 \
                "DEFVAR LF@%res \n"                                     \
                "MOVE LF@%res nil@nil \n"                               \
                "DEFVAR LF@p0 \n"                                       \
                "MOVE LF@p0 LF@%0 \n"                                   \
                "DEFVAR LF@p1 \n"                                       \
                "MOVE LF@p1 LF@%1 \n"                                   \
                "DEFVAR LF@p2 \n"                                       \
                "MOVE LF@p2 LF@%2 \n"                                   \
                "# check params \n"                                     \
                "DEFVAR LF@check \n"                                    \
                "LT LF@check LF@p1 int@1 \n"                            \
                "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"            \
                "LT LF@check LF@p2 int@1 \n"                            \
                "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"            \
                "DEFVAR LF@str_len \n"                                  \
                "STRLEN LF@str_len LF@p0 \n"                            \
                "GT LF@check LF@p1 LF@str_len \n"                       \
                "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"            \
                "GT LF@check LF@p2 LF@str_len \n"                       \
                "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"            \
                "LT LF@check LF@p2 LF@p1 \n"                            \
                "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"            \
                "# for (i = i; i < j; i++) \n"                          \
                "SUB LF@p1 LF@p1 int@1 \n"                              \
                "DEFVAR LF@tmp_char \n"                                 \
                "MOVE LF@%res string@ \n"                               \
                "LABEL LOOP \n"                                         \
                    "GETCHAR LF@tmp_char LF@p0 LF@p1 \n"                \
                    "CONCAT LF@%res LF@%res LF@tmp_char \n"             \
                    "ADD LF@p1 LF@p1 int@1 \n"                          \
                "# if (i < j) goto LOOP \n"                             \
                "LT LF@check LF@p1 LF@p2 \n"                            \
                "JUMPIFEQ LOOP LF@check bool@true \n"                   \
                "LABEL $SUBSTR$end \n"                                  \
                "POPFRAME \n"                                           \
                "RETURN \n");
}

/*
 * @brief Generates function call.
 */
static void generate_func_call() {
    ADD_INSTR_PART("CALL $");
    // add function id
    ADD_INSTR_PART("main");
    ADD_INSTR_TMP;
}

/*
 * @brief Generates function start.
 */
static void generate_func_start() {
    ADD_INSTR("LABEL $main");
    ADD_INSTR("CREATEFRAME");
    ADD_INSTR("PUSHFRAME");
}

/*
 * @brief Generates function end.
 */
static void generate_func_end() {
    ADD_INSTR("LABEL $main$end");
    ADD_INSTR("POPFRAME");
    ADD_INSTR("RETURN");
}

/*
 * @brief Defines return value of return parameter with index.
 * generates sth like:
 *      DEFVAR LF@%result0
 */
static void generate_func_return_value(unsigned index) {
    ADD_INSTR_PART("DEFVAR LF@%result");
    ADD_INSTR_INT(index);
    ADD_INSTR_TMP;
}

/*
 * @brief Generates code with value of the token.
 */
static void generate_func_param_value(token_t token) {
    char str[MAX_CHAR];
    memset(str, '\0', MAX_CHAR);
    switch (token.type) {
        case TOKEN_STR:
            ADD_INSTR_PART("string@");
            // transform the string format
            unsigned str_len = Dynstring.len(token.attribute.id);
            char *str_id = Dynstring.get_str(token.attribute.id);
            for (unsigned i = 0; i < str_len; i++) {
                // check format (check what to do with not printable chars?)
                if (str_id[i] <= 32 || str_id[i] == 35 || str_id[i] == 92) {
                    // print as an escape sequence
                    sprintf(str, "\\%03d", str_id[i]);
                } else {
                    sprintf(str, "%c", str_id[i]);
                }
                ADD_INSTR_PART(str);
                memset(str, '\0', MAX_CHAR);
            }
            break;
        case TOKEN_NUM_F:
            ADD_INSTR_PART("float@");
            sprintf(str, "%a", token.attribute.num_f);
            ADD_INSTR_PART(str);
            break;
        case TOKEN_NUM_I:
            ADD_INSTR_PART("int@");
            sprintf(str, "%lu", token.attribute.num_i);
            ADD_INSTR_PART(str);
            break;
        case KEYWORD_nil:
            ADD_INSTR_PART("nil@nil");
            break;
        case TOKEN_ID:
            ADD_INSTR_PART("LF@");
            ADD_INSTR_PART(Dynstring.get_str(token.attribute.id));
            break;
        default:
            break;
    }
}

/*
 * @brief Generates parameter pass to a function.
 * generates sth like:
 *          DEFVAR TF@%0
 *          MOVE TF@%0 int@42
 */
static void generate_func_pass_param(token_t token, int param_index) {
    ADD_INSTR_PART("DEFVAR TF@%");
    ADD_INSTR_INT(param_index);
    ADD_INSTR_TMP;

    ADD_INSTR_PART("MOVE TF@%");
    ADD_INSTR_INT(param_index);
    ADD_INSTR_PART(" ");
    generate_func_param_value(token);
    ADD_INSTR_TMP;
}

/*
 * @brief Generates creation of a frame before passing parameters to a function
 */
static void generate_func_createframe() {
    ADD_INSTR("CREATEFRAME");
}

/*
 * @brief Generates program start (adds header, define built-in functions).
 */
static void generate_prog_start() {
    ADD_INSTR(".IFJcode21");
    generate_func_ord();
    generate_func_chr();
    generate_func_substr();
    generate_func_reads();
    generate_func_readi();
    generate_func_readn();
    generate_func_write();
    generate_func_tointeger();

    // create main function, JUMP
}

/**
 * Interface to use when dealing with code generator.
 * Functions are in struct so we can use them in different files.
 */
const struct code_generator_interface_t Generator = {
        .prog_start = generate_prog_start,
        .func_start = generate_func_start,
        .func_end = generate_func_end,
        .func_return_value = generate_func_return_value,
        .func_pass_param = generate_func_pass_param,
        .func_createframe = generate_func_createframe,
};
