#include "code_generator.h"

size_t __ADDRESS_OF_START_LIST;

/*
 * Global variables used for code generator.
 */
list_t *instrList;              // pointer to the list of instr that is currently used
static dynstring_t *tmp_instr;         // instruction that is currently being generated
instructions_t instructions;    // structure that holds info about generated code

/*
 * Adds new instruction to the list of instructions.
 */
void ADD_INSTR(char *instr) {
    debug_msg("\n");
    __START_LIST_ASSERT();
    dynstring_t *instr_ds = Dynstring.ctor(instr);
    __START_LIST_ASSERT();

    List.append(instrList, instr_ds);
    __START_LIST_ASSERT();
}

/*
 * Adds part of an instruction to global tmp_instr.
 * Converts instrPart to a dynstring_t*.
 */
void ADD_INSTR_PART(char *instrPart) {
    __START_LIST_ASSERT();
    dynstring_t *newInstrPart = Dynstring.ctor(instrPart);
    __START_LIST_ASSERT();
    Dynstring.cat(tmp_instr, newInstrPart);
    __START_LIST_ASSERT();
    Dynstring.dtor(newInstrPart);
    __START_LIST_ASSERT();
}

/*
 * Adds part of an instruction to global tmp_instr.
 * instrPartDynstr already is a dyntring_t*.
 */
void ADD_INSTR_PART_DYN(dynstring_t *instrPartDyn) {
    __START_LIST_ASSERT();
    Dynstring.cat(tmp_instr, instrPartDyn);
    __START_LIST_ASSERT();
}

/*
 * Adds tmp_inst to the list of instructions.
 */
void ADD_INSTR_TMP() {
    __START_LIST_ASSERT();
    List.append(instrList,
                Dynstring.ctor(Dynstring.c_str(tmp_instr))
    );
    Dynstring.clear(tmp_instr);
    __START_LIST_ASSERT();
}

/*
 * Inserts tmp_inst before while loop.
 */
void ADD_INSTR_WHILE() {
    __START_LIST_ASSERT();
    List.insert_after(instructions.before_loop_start,
                      Dynstring.ctor(Dynstring.c_str(tmp_instr))
    );
    Dynstring.clear(tmp_instr);
    __START_LIST_ASSERT();
}

/*
 * Converts integer to string and adds it to tmp_instr
 */
void ADD_INSTR_INT(uint64_t num) {
    __START_LIST_ASSERT();
    char str[MAX_CHAR] = "\0";
    sprintf(str, "%*llu", MAX_CHAR - 1, num);
    ADD_INSTR_PART(str);
    __START_LIST_ASSERT();
}

/*
 * Change active list of instructions.
 */
void INSTR_CHANGE_ACTIVE_LIST(list_t *newList) {
    __START_LIST_ASSERT();
    debug_msg("\n");
    debug_msg_s("\tinstrList  =  newlist\n\t%p(old) -> ", (void *) instrList);
    instrList = (newList);
    debug_msg_s("%p(new)\n", (void *) instrList);
    __START_LIST_ASSERT();
}

/*
 * @brief   Generates built-in function reads().
 *          function reads() : string
 *          result: %res
 */
static void generate_func_reads() {
    ADD_INSTR("LABEL $READS \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "READ LF@%res string \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function readi().
 *          function readi() : integer
 *          result: %res
 */
static void generate_func_readi() {
    ADD_INSTR("LABEL $READI \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "READ LF@%res int \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function readn().
 *          function readn() : integer
 *          result: %res
 */
static void generate_func_readn() {
    ADD_INSTR("LABEL $READN \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "READ LF@%res float \n"
              "POPFRAME \n"
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
    ADD_INSTR("LABEL $WRITE \n"
              "PUSHFRAME \n"
              "DEFVAR LF@p0 \n"
              "MOVE LF@p0 LF@%0 \n"
              "WRITE LF@p0 \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function tointeger().
 *          function tointeger(f : number) : integer
 *          params: f ... %0
 *          result: %res
 */
static void generate_func_tointeger() {
    ADD_INSTR("LABEL $TOINTEGER \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "DEFVAR LF@p0 \n"
              "MOVE LF@p0 LF@%0 \n"
              "FLOAT2INT LF@%res LF@p0 \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function chr().
 *          function chr(i : integer) : string
 *          params: i ... %0
 *          result: %res
 */
static void generate_func_chr() {
    ADD_INSTR("LABEL $CHR \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "MOVE LF@%res nil@nil \n"
              "DEFVAR LF@p0 \n"
              "MOVE LF@p0 LF@%0 \n"
              "DEFVAR LF@check \n"
              "LT LF@check LF@p0 int@0 \n"
              "JUMPIFEQ $CHR$end LF@check bool@true \n"
              "GT LF@check LF@p0 int@255 \n"
              "JUMPIFEQ $CHR$end LF@check bool@true \n"
              "INT2CHAR LF@%res LF@p0 \n"
              "LABEL $CHR$end \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function ord().
 *          function ord(s : string, i : integer) : integer
 *          params: s ... %0, i ... %1
 *          result: %res
 */
static void generate_func_ord() {
    ADD_INSTR("LABEL $ORD \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "MOVE LF@%res nil@nil \n"
              "DEFVAR LF@p0 \n"
              "MOVE LF@p0 LF@%0 \n"
              "DEFVAR LF@p1 \n"
              "MOVE LF@p1 LF@%1 \n"
              "DEFVAR LF@str_len \n"
              "STRLEN LF@str_len LF@p0 \n"
              "DEFVAR LF@check \n"
              "LT LF@check LF@p1 int@1 \n"
              "JUMPIFEQ $ORD$end LF@check bool@true \n"
              "GT LF@check LF@p1 LF@str_len \n"
              "JUMPIFEQ $ORD$end LF@check bool@true \n"
              "SUB LF@p1 LF@p1 int@1 \n"
              "STRI2INT LF@%res LF@p0 LF@p1 \n"
              "LABEL $ORD$end \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function substr().
 *          function substr(s : string, i : number, j : number) : string
 *          params: s ... %0, i ... %1, j ... %2
 *          result: %res
*/
static void generate_func_substr() {
    ADD_INSTR("LABEL $SUBSTR \n"
              "PUSHFRAME \n"
              "# define vars for params and result\n"
              "DEFVAR LF@%res \n"
              "MOVE LF@%res nil@nil \n"
              "DEFVAR LF@p0 \n"
              "MOVE LF@p0 LF@%0 \n"
              "DEFVAR LF@p1 \n"
              "MOVE LF@p1 LF@%1 \n"
              "DEFVAR LF@p2 \n"
              "MOVE LF@p2 LF@%2 \n"
              "# check params \n"
              "DEFVAR LF@check \n"
              "LT LF@check LF@p1 int@1 \n"
              "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"
              "LT LF@check LF@p2 int@1 \n"
              "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"
              "DEFVAR LF@str_len \n"
              "STRLEN LF@str_len LF@p0 \n"
              "GT LF@check LF@p1 LF@str_len \n"
              "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"
              "GT LF@check LF@p2 LF@str_len \n"
              "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"
              "LT LF@check LF@p2 LF@p1 \n"
              "JUMPIFEQ $SUBSTR$end LF@check bool@true \n"
              "# for (i = i; i < j; i++) \n"
              "SUB LF@p1 LF@p1 int@1 \n"
              "DEFVAR LF@tmp_char \n"
              "MOVE LF@%res string@ \n"
              "LABEL LOOP \n"
              "GETCHAR LF@tmp_char LF@p0 LF@p1 \n"
              "CONCAT LF@%res LF@%res LF@tmp_char \n"
              "ADD LF@p1 LF@p1 int@1 \n"
              "# if (i < j) goto LOOP \n"
              "LT LF@check LF@p1 LF@p2 \n"
              "JUMPIFEQ LOOP LF@check bool@true \n"
              "LABEL $SUBSTR$end \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief Generates function call.
 */
static void generate_func_call(dynstring_t *func_name) {
    ADD_INSTR_PART("CALL $");
    ADD_INSTR_PART_DYN(func_name);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates main scope start.
 */
static void generate_main_start() {
    INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
    ADD_INSTR("LABEL $$MAIN");
    ADD_INSTR("CREATEFRAME");
    ADD_INSTR("PUSHFRAME");
}

/*
 * @brief Generates function end.
 * FIXME is this function necessary?
 */
static void generate_main_end() {
    ADD_INSTR("LABEL $$MAIN$end");
    // TODO remove
    ADD_INSTR("WRITE string@\\010\\010SUCCESSFUL\\010");
}

/*
 * @brief Generates function start.
 * generates sth like: LABEL $foo
 *                     PUSHFRAME
 */
static void generate_func_start(dynstring_t *func_name) {
    INSTR_CHANGE_ACTIVE_LIST(instructions.instrListFunctions);
    ADD_INSTR_PART("LABEL $");   // add name of function
    ADD_INSTR_PART_DYN(func_name);
    ADD_INSTR_TMP();
    ADD_INSTR("PUSHFRAME");

    // TODO remove this and make another function - we need to define LF@%result... after starting function definition
    ADD_INSTR("DEFVAR LF@%result");
    ADD_INSTR("MOVE LF@%result nil@nil");
}

/*
 * @brief Generates function end.
 */
static void generate_func_end(char *func_name) {
    debug_msg("\n");

    debug_msg_s("\tadd_instr_part: '%s' to tmp instr'%s'\n", func_name, Dynstring.c_str(tmp_instr));
    ADD_INSTR_PART("LABEL $");

    debug_msg_s("\tadd_instr_part, tmp_str is now '%s'\n", Dynstring.c_str(tmp_instr));
    ADD_INSTR_PART(func_name);

    debug_msg_s("\tadd_instr_part, tmp_str is now '%s'\n", Dynstring.c_str(tmp_instr));
    ADD_INSTR_PART("$end");

    debug_msg_s("\tadd_instr_tmp, tmp_str is now '%s'\n", Dynstring.c_str(tmp_instr));
    ADD_INSTR_TMP();

    // FIXME - remove these instructions
    debug_msg_s("\tadd_instr, tmp_str is now '%s'\n", Dynstring.c_str(tmp_instr));
    ADD_INSTR("WRITE GF@%expr_result \n"
              "WRITE string@\\010\n"
              "DEFVAR LF@type_var \n"
              "TYPE LF@type_var GF@%expr_result \n"
              "WRITE LF@type_var");

    debug_msg_s("\tadd_instr, tmp_str is now '%s'\n", Dynstring.c_str(tmp_instr));
    ADD_INSTR("POPFRAME");

    debug_msg_s("\tinstr_change_active_list,\n");
    ADD_INSTR("RETURN\n");
    INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
    debug_msg_s("\tdone.\n");
}

/*
 * @brief Generates saving param from TF to LF.
 * generates sth like:
 *      DEFVAR LF@%p0
 *      MOVE LF@%p0 LF@%0
 */
static void generate_func_start_param(dynstring_t *param_name, size_t index) {
    ADD_INSTR_PART("DEFVAR LF@%");
    ADD_INSTR_PART_DYN(param_name);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("MOVE LF@%");
    ADD_INSTR_PART_DYN(param_name);
    ADD_INSTR_PART(" LF@%");
    ADD_INSTR_INT(index);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates return value of return parameter with index.
 * generates sth like:
 *      DEFVAR LF@%return0
 *      MOVE LF@%return0 nil@nil
 */
static void generate_func_return_value(size_t index) {
    ADD_INSTR_PART("DEFVAR LF@%return");
    ADD_INSTR_INT(index);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("MOVE LF@%return");
    ADD_INSTR_INT(index);
    ADD_INSTR_PART(" nil@nil");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates code with value of the token.
 */
static void generate_var_value(token_t token) {
    char str_tmp[MAX_CHAR];
    memset(str_tmp, '\0', MAX_CHAR);
    switch (token.type) {
        case TOKEN_STR:
            ADD_INSTR_PART("string@");
            // transform the string format
            unsigned str_len = Dynstring.len(token.attribute.id);
            char *str_id = Dynstring.c_str(token.attribute.id);
            for (unsigned i = 0; i < str_len; i++) {
                // check format (check what to do with not printable chars?)
                if (!isprint(str_id[i]) || str_id[i] == '#' || str_id[i] == '\\') {
                    // print as an escape sequence
                    sprintf(str_tmp, "\\%03d", str_id[i]);
                } else {
                    sprintf(str_tmp, "%c", str_id[i]);
                }
                ADD_INSTR_PART(str_tmp);
                memset(str_tmp, '\0', MAX_CHAR);
            }
            break;
        case TOKEN_NUM_F:
            ADD_INSTR_PART("float@");
            sprintf(str_tmp, "%a", token.attribute.num_f);
            ADD_INSTR_PART(str_tmp);
            break;
        case TOKEN_NUM_I:
            ADD_INSTR_PART("int@");
            sprintf(str_tmp, "%llu", token.attribute.num_i);
            ADD_INSTR_PART(str_tmp);
            break;
        case KEYWORD_nil:
            ADD_INSTR_PART("nil@nil");
            break;
        case TOKEN_ID:
            ADD_INSTR_PART("LF@%");
            ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
            ADD_INSTR_PART("%");
            ADD_INSTR_PART_DYN(token.attribute.id);
            break;
        default:
            ADD_INSTR_PART("unexpected_token");
            break;
    }
}

/*
 * @brief Generates DEFVAR LF@%var.
 */
static void generate_defvar(dynstring_t *var_name) {
    ADD_INSTR_PART("DEFVAR LF@%");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART("%");
    ADD_INSTR_PART_DYN(var_name);
    if (instructions.in_loop) {
        ADD_INSTR_WHILE();
    } else {
        ADD_INSTR_TMP();
    }
}

/*
 * @brief Generates variable declaration.
 */
static void generate_var_definition(dynstring_t *var_name) {
    __START_LIST_ASSERT();
    debug_msg("\n\t- generate_var_definition: %s\n", Dynstring.c_str(var_name));
    generate_defvar(var_name);
    __START_LIST_ASSERT();

    ADD_INSTR_PART("MOVE LF@%");
    __START_LIST_ASSERT();
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    __START_LIST_ASSERT();
    ADD_INSTR_PART("%");
    __START_LIST_ASSERT();
    ADD_INSTR_PART_DYN(var_name);
    __START_LIST_ASSERT();
    ADD_INSTR_PART(" GF@%expr_result");
    __START_LIST_ASSERT();
    ADD_INSTR_TMP();
    __START_LIST_ASSERT();
}

/*
 * @brief Generates variable declaration.
 */
static void generate_var_declaration(dynstring_t *var_name) {
    debug_msg("\n\t- generate_var_declaration: %s\n", Dynstring.c_str(var_name));
    generate_defvar(var_name);

    // initialise to nil
    ADD_INSTR_PART("MOVE LF@%");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART("%");
    ADD_INSTR_PART_DYN(var_name);
    ADD_INSTR_PART(" nil@nil");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates parameter pass to a function.
 * generates sth like:
 *          DEFVAR TF@%0
 *          MOVE TF@%0 int@42
 */
static void generate_func_pass_param(size_t param_index) {
    ADD_INSTR_PART("DEFVAR TF@%");
    ADD_INSTR_INT(param_index);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("MOVE TF@%");
    ADD_INSTR_INT(param_index);
    ADD_INSTR_PART(" ");
    generate_var_value(Scanner.get_curr_token());
    ADD_INSTR_TMP();
}

/*
 * @brief Generates creation of a frame before passing parameters to a function
 */
static void generate_func_createframe() {
    ADD_INSTR("CREATEFRAME");

    // TODO remove this instruction when foo() parsing is handled
    ADD_INSTR("PUSHS int@2");
}

/*
 * @brief Generates condition label.
 */
static void generate_cond_label(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("LABEL $");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$");
    ADD_INSTR_INT(cond_num);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates start of if block. The result of expression in the condition
 *        is expected in LF@%result (LF@%result0?) variable.
 * generates sth like: JUMPIFNEQ $23$next_cond LF@%result bool@true
 */
static void generate_cond_if(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("JUMPIFNEQ $");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$");
    ADD_INSTR_INT(cond_num);
    ADD_INSTR_PART(" LF@%result bool@true");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates start of else if block - JUMP $end from previous block
 *        and LABEL for new elseif block.
 * generates sth like: JUMP $scope$end
 *                     LABEL $scope$new_scope_num
 */
static void generate_cond_elseif(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("JUMP $");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    generate_cond_label(if_scope_id, cond_num);
}

/*
 * @brief Generates start of else statement.
 *          JUMP $id_scope$end
 *          LABEL $id_scope$else
 *          JUMPIFNEQ $id_scope$end
 *          --- else body ---
 */
static void generate_cond_else(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("JUMP $");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    generate_cond_label(if_scope_id, cond_num - 1);

    ADD_INSTR_PART("JUMPIFNEQ $");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end LF@%result bool@true");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates end of if statement.
 * generates sth like: LABEL $scope$end
 *                     LABEL $scope$new_scope_num
 * Yes, I need two labels rn.
 */
static void generate_cond_end(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("LABEL $");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    generate_cond_label(if_scope_id, cond_num);
}

/*
 * @brief Generates while loop header.
 * generates sth like: LABEL $while$id
 */
static void generate_while_header() {
    ADD_INSTR_PART("LABEL $while$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates while loop condition check.
 * generates sth like: JUMPIFEQ $end$id LF@%result bool@true
 */
static void generate_while_cond() {
    ADD_INSTR_PART("JUMPIFEQ $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART(" LF@%result bool@true");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates while loop end.
 * generates sth like: JUMP $while$id
 *                     LABEL $end$id
 */
static void generate_while_end() {
    ADD_INSTR_PART("JUMP $while$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("LABEL $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates end.
 *        LABEL $end$id
 */
static void generate_end() {
    ADD_INSTR_PART("LABEL $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates repeat until loop header.
 * generates sth like: LABEL $repeat$id
 */
static void generate_repeat_until_header() {
    ADD_INSTR_PART("LABEL $repeat$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates repeat until loop condition check.
 * generates sth like: JUMPIFEQ $repeat$id LF@%result bool@true
 */
static void generate_repeat_until_cond() {
    ADD_INSTR_PART("JUMPIFEQ $repeat$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART(" LF@%result bool@true");
    ADD_INSTR_TMP();
}

/*
 * TODO
 * @brief Generates for loop header.
 * generates sth like: LABEL $for$id
 */
static void generate_for_header(dynstring_t *var_name) {
    generate_var_definition(var_name);
    ADD_INSTR_PART("LABEL $for$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates for loop condition check.
 * generates sth like: JUMPIFEQ $end$id LF@%result bool@true
 */
static void generate_for_cond() {
    ADD_INSTR_PART("JUMPIFNEQ $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART(" LF@%result bool@true");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates for loop end.
 * generates sth like: ADD %var value
 *                     JUMP $for$id
 *                     LABEL $end$id
 */
static void generate_for_end() {
    ADD_INSTR_PART("JUMP $for$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("LABEL $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates program start (adds header, define built-in functions).
 */
static void generate_prog_start() {
    INSTR_CHANGE_ACTIVE_LIST(instructions.startList);
    ADD_INSTR(".IFJcode21");
    ADD_INSTR("DEFVAR GF@%expr_result \n"
              "MOVE GF@%expr_result nil@nil");
    ADD_INSTR("DEFVAR GF@%expr_result2 \n"
              "MOVE GF@%expr_result2 nil@nil");
    ADD_INSTR("DEFVAR GF@%expr_result3 \n"
              "MOVE GF@%expr_result3 nil@nil");
    ADD_INSTR("JUMP $$MAIN");
    ADD_INSTR(  "LABEL $$ERROR_NIL \n"
                "EXIT int@8 \n"
                "LABEL $$ERROR_DIV_BY_ZERO \n"
                "EXIT int@9");


    INSTR_CHANGE_ACTIVE_LIST(instructions.instrListFunctions);
    // TODO: add built-in functions every time or when needed?
    generate_func_ord();
    generate_func_chr();
    generate_func_substr();
    generate_func_reads();
    generate_func_readi();
    generate_func_readn();
    generate_func_write();
    generate_func_tointeger();

    INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
    generate_main_start();


    __ADDRESS_OF_START_LIST = (size_t) ((void *) (instructions.startList->head));
    debug_msg_s("start list: %p\n", (void *) instructions.startList);
    debug_msg_s("(start list)->head %p\n", (void *) instructions.startList->head);
    debug_msg_s("(__ADDRESS_OF_START_LIST) %zx\n", __ADDRESS_OF_START_LIST);
    debug_msg_s("(start list)->head->next %p\n", (void *) instructions.startList->head->next);
}

void generate_division_check(bool integer) {
    ADD_INSTR("POPS GF@%expr_result");
    if (integer) {
        ADD_INSTR_PART("JUMPIFEQ $$ERROR_DIV_BY_ZERO GF@%expr_result int@0");
    } else {
        ADD_INSTR("JUMPIFEQ $$ERROR_DIV_BY_ZERO GF@%expr_result float@0x0p+0");
    }
    ADD_INSTR("PUSHS GF@%expr_result");
}

static void generate_nil_check(token_t token) {
    ADD_INSTR_PART("JUMPIFEQ $$ERROR_NIL ");
    generate_var_value(token);
    ADD_INSTR_PART(" nil@nil");
    ADD_INSTR_TMP();
}

static void retype_first_or_both(expr_semantics_t *expr) {
    ADD_INSTR("POPS GF@%expr_result2 \n"
              "POPS GF@%expr_result \n"
              "INT2FLOAT GF@%expr_result GF@%expr_result");
    if (expr->conv_type == CONVERT_BOTH) {
        ADD_INSTR("INT2FLOAT GF@%expr_result2 GF@%expr_result2");
    }
    ADD_INSTR("PUSHS GF@%expr_result \n"
              "PUSHS GF@%expr_result2");
}

static void retype_second() {
    ADD_INSTR("POPS GF@%expr_result \n"
              "INT2FLOAT GF@%expr_result GF@%expr_result \n"
              "PUSHS GF@%expr_result");
}

static void generate_expression_pop() {
    ADD_INSTR("POPS GF@%expr_result");
    ADD_INSTR("CLEARS");
}

static void retype_and_push(expr_semantics_t *expr) {
    // retype if needed and push operands
    if (expr->conv_type == CONVERT_FIRST || expr->conv_type == CONVERT_BOTH) {
        retype_first_or_both(expr);
    }
    if (expr->conv_type == CONVERT_SECOND) {
        retype_second();
    }
}

/*
 * get id
 * check nil
 * push
 */
static void generate_expression_operand(expr_semantics_t *expr) {
    // is it okay to check only TOKEN_ID?
    if (expr->first_operand.type == TOKEN_ID)
        generate_nil_check(expr->first_operand);

    ADD_INSTR_PART("PUSHS ");
    generate_var_value(expr->first_operand);
    ADD_INSTR_TMP();
}

static void generate_expression(expr_semantics_t *expr) {
    soft_assert(expr, ERROR_INTERNAL);

    if (expr->sem_state == SEMANTIC_OPERAND) {
        generate_expression_operand(expr);
        return;
    }

    retype_and_push(expr);
    // generate operation
    switch (expr->op) {
        case OP_ADD:
            ADD_INSTR("ADDS");
            break;
        case OP_SUB:
            ADD_INSTR("SUBS");
            break;
        case OP_MUL:
            ADD_INSTR("MULS");
            break;
        case OP_DIV_I:
            generate_division_check(true); // true means int division check
            ADD_INSTR("IDIVS");
            break;
        case OP_DIV_F:
            generate_division_check(false); // false means float division check
            ADD_INSTR("DIVS");
            break;
        case OP_LT:
            ADD_INSTR("LTS");
            break;
        case OP_LE:
            ADD_INSTR(  "POPS GF@%expr_result2 \n"
                        "POPS GF@%expr_result \n"
                        "LT GF@%expr_result3 GF@%expr_result GF@%expr_result2 \n"
                        "EQ GF@%expr_result2 GF@%expr_result GF@%expr_result2 \n"
                        "OR GF@%expr_result GF@%expr_result2 GF@%expr_result3 \n"
                        "PUSHS GF@%expr_result");
            break;
        case OP_GT:
            ADD_INSTR("GTS");
            break;
        case OP_GE:
            ADD_INSTR(  "POPS GF@%expr_result2 \n"
                        "POPS GF@%expr_result \n"
                        "GT GF@%expr_result3 GF@%expr_result GF@%expr_result2 \n"
                        "EQ GF@%expr_result2 GF@%expr_result GF@%expr_result2 \n"
                        "OR GF@%expr_result GF@%expr_result2 GF@%expr_result3 \n"
                        "PUSHS GF@%expr_result");
            break;
        case OP_EQ:
            ADD_INSTR("EQS");
            break;
        case OP_NE:
            ADD_INSTR( "EQS"
                        "NOTS");
            break;
        case OP_NOT:
            ADD_INSTR("NOTS");
            break;
        case OP_AND:
            ADD_INSTR("ANDS");
            break;
        case OP_OR:
            ADD_INSTR("ORS");
            break;
        case OP_HASH:
            ADD_INSTR("POPS GF@%expr_result2 \n"
                        "STRLEN GF@%expr_result GF@%expr_result2 \n"
                        "PUSHS GF@%expr_result");
            break;
        case OP_STRCAT:
            ADD_INSTR("POPS GF@%expr_result2 \n"
                        "POPS GF@%expr_result \n"
                        "CONCAT GF@%expr_result GF@%expr_result GF@%expr_result2 \n"
                        "PUSHS GF@%expr_result");
            break;
        default:
            ADD_INSTR("# Another instruction :(");
    }
}

/*
 * @brief Initialises the code generator.
 */
static void initialise_generator() {
    debug_msg("");
    // initialise tmp_instr to empty dynstring
    tmp_instr = Dynstring.ctor("");
    debug_msg_s("tmp_instr initialized %p\n", (void *) tmp_instr);

    // initialise the instructions structure
    instructions.startList = List.ctor();
    debug_msg_s("instructions.startList initialized %p\n", (void *) instructions.startList);

    instructions.instrListFunctions = List.ctor();
    debug_msg_s("instructions.instrListFunctions initialized %p\n", (void *) instructions.instrListFunctions);

    instructions.mainList = List.ctor();
    debug_msg_s("instructions.mainList initialized %p\n", (void *) instructions.mainList);

    instructions.in_loop = false;
    debug_msg_s("instructions.in_loop initialized with false\n");

    instructions.outer_loop_id = 0;
    debug_msg_s("instructions.outer_loop_id initialized with 0\n");

    instructions.before_loop_start = NULL;
    debug_msg_s("instructions.before_loop_start initialized with NULL\n");

    instructions.outer_cond_id = 0;
    debug_msg_s("instructions.outer_cond_id initialized with 0\n");

    instructions.cond_cnt = 0;
    debug_msg_s("instructions.cond_cnt initialized with 0\n");

    // sets active instructions list
    instrList = instructions.startList;
    debug_msg_s("instrList initialized with instructions.startList %p\n\n", (void *) instrList);
}

static void dtor() {
    debug_msg("\n");
    List.dtor(instructions.startList, (void (*)(void *)) (Dynstring.dtor)); // use Dynstring.dtor or free?
    debug_msg("instructions.startList freed.\n");

    List.dtor(instructions.instrListFunctions, (void (*)(void *)) Dynstring.dtor); // use Dynstring.dtor or free?
    debug_msg("instructions.instrListFunctions freed.\n");

    List.dtor(instructions.mainList, (void (*)(void *)) Dynstring.dtor); // use Dynstring.dtor or free?
    debug_msg("instructions.mainList freed.\n");

    Dynstring.dtor(tmp_instr);
    debug_msg("tmp_instr freed\n");
}

static void Print_instr_list(instr_list_t instr_list_type) {
    switch (instr_list_type) {
        case LIST_INSTR_START:
            List.print_list(instructions.startList, (char *(*)(void *)) Dynstring.c_str);
            break;
        case LIST_INSTR_FUNC:
            List.print_list(instructions.instrListFunctions, (char *(*)(void *)) Dynstring.c_str);
            break;
        case LIST_INSTR_MAIN:
            List.print_list(instructions.mainList, (char *(*)(void *)) Dynstring.c_str);
            break;
        default:
            printf("Undefined instruction list.\n");
            break;
    }
}


/**
 * Interface to use when dealing with code generator.
 * Functions are in struct so we can use them in different files.
 */
const struct code_generator_interface_t Generator = {
        .prog_start = generate_prog_start,
        .func_start = generate_func_start,
        .func_end = generate_func_end,
        .func_start_param = generate_func_start_param,
        .func_return_value = generate_func_return_value,
        .func_pass_param = generate_func_pass_param,
        .func_createframe = generate_func_createframe,
        .main_end = generate_main_end,
        .func_call = generate_func_call,
        .var_declaration = generate_var_declaration,
        .var_definition = generate_var_definition,
        .cond_if = generate_cond_if,
        .cond_elseif = generate_cond_elseif,
        .cond_else = generate_cond_else,
        .cond_end = generate_cond_end,
        .while_header = generate_while_header,
        .while_cond = generate_while_cond,
        .while_end = generate_while_end,
        .end = generate_end,
        .repeat_until_header = generate_repeat_until_header,
        .repeat_until_cond = generate_repeat_until_cond,
        .for_header = generate_for_header,
        .for_cond = generate_for_cond,
        .initialise = initialise_generator,
        .expression = generate_expression,
        .expression_pop = generate_expression_pop,
        .dtor = dtor,
        .print_instr_list = Print_instr_list,
};
