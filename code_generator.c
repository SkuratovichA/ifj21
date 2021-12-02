/**
 * @file code_generator.c
 *
 * @author Lucie Svobodova
 */

#include "code_generator.h"

/*
 * Global variables used for code generator.
 */
list_t *instrList;                      // pointer to the list of instr that is currently used
static dynstring_t *tmp_instr;          // instruction that is currently being generated
instructions_t instructions;            // structure that holds info about generated code

/*
 * Adds new instruction to the list of instructions.
 */
void ADD_INSTR(char *instr) {
    dynstring_t *instr_ds = Dynstring.ctor(instr);

    List.append(instrList, instr_ds);
}

/*
 * Adds part of an instruction to global tmp_instr.
 * Converts instrPart to a dynstring_t*.
 */
void ADD_INSTR_PART(char *instrPart) {
    dynstring_t *newInstrPart = Dynstring.ctor(instrPart);
    Dynstring.cat(tmp_instr, newInstrPart);
    Dynstring.dtor(newInstrPart);
}

/*
 * Adds part of an instruction to global tmp_instr.
 * instrPartDynstr already is a dyntring_t*.
 */
void ADD_INSTR_PART_DYN(dynstring_t *instrPartDyn) {
    Dynstring.cat(tmp_instr, instrPartDyn);
}

/*
 * Adds tmp_inst to the list of instructions.
 */
void ADD_INSTR_TMP() {
    List.append(instrList,
                Dynstring.ctor(Dynstring.c_str(tmp_instr))
    );
    Dynstring.clear(tmp_instr);
}

/*
 * Inserts tmp_inst before while loop.
 */
void ADD_INSTR_WHILE() {
    List.insert_after(instructions.before_loop_start,
                      Dynstring.ctor(Dynstring.c_str(tmp_instr))
    );
    Dynstring.clear(tmp_instr);
}

/*
 * Converts integer to string and adds it to tmp_instr
 */
void ADD_INSTR_INT(uint64_t num) {
    char str[MAX_CHAR] = "\0";
    sprintf(str, "%lu", num);
    ADD_INSTR_PART(str);
}

/*
 * Change active list of instructions.
 */
void INSTR_CHANGE_ACTIVE_LIST(list_t *newList) {
    instrList = (newList);
}

/*
 * @brief   Generates built-in function reads().
 *          function reads() : string
 *          result: %res
 */
static void generate_func_reads() {
    ADD_INSTR("LABEL $reads \n"
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
    ADD_INSTR("LABEL $readi \n"
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
    ADD_INSTR("LABEL $readn \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "READ LF@%res float \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function write().
 *          (only for one term)
 *          function write (term1, term2, ..., termn)
 */
static void generate_func_write() {
    ADD_INSTR("LABEL $write \n"
              "WRITE GF@%expr_result \n"
              "RETURN \n");
}

/*
 * @brief   Generates built-in function tointeger().
 *          function tointeger(f : number) : integer
 *          params: f ... %0
 *          result: %res
 */
static void generate_func_tointeger() {
    ADD_INSTR("LABEL $tointeger \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%return0 \n"
              "FLOAT2INT LF@%return0 LF@%0 \n"
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
    ADD_INSTR("LABEL $chr \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "MOVE LF@%res nil@nil \n"
              "DEFVAR LF@p0 \n"
              "MOVE LF@p0 LF@%0 \n"
              "DEFVAR LF@check \n"
              "LT LF@check LF@p0 int@0 \n"
              "JUMPIFEQ $chr$end LF@check bool@true \n"
              "GT LF@check LF@p0 int@255 \n"
              "JUMPIFEQ $chr$end LF@check bool@true \n"
              "INT2CHAR LF@%res LF@p0 \n"
              "LABEL $chr$end \n"
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
    ADD_INSTR("LABEL $ord \n"
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
              "JUMPIFEQ $ord$end LF@check bool@true \n"
              "GT LF@check LF@p1 LF@str_len \n"
              "JUMPIFEQ $ord$end LF@check bool@true \n"
              "SUB LF@p1 LF@p1 int@1 \n"
              "STRI2INT LF@%res LF@p0 LF@p1 \n"
              "LABEL $ord$end \n"
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
    ADD_INSTR("LABEL $substr \n"
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
              "JUMPIFEQ $substr$end LF@check bool@true \n"
              "LT LF@check LF@p2 int@1 \n"
              "JUMPIFEQ $substr$end LF@check bool@true \n"
              "DEFVAR LF@str_len \n"
              "STRLEN LF@str_len LF@p0 \n"
              "GT LF@check LF@p1 LF@str_len \n"
              "JUMPIFEQ $substr$end LF@check bool@true \n"
              "GT LF@check LF@p2 LF@str_len \n"
              "JUMPIFEQ $substr$end LF@check bool@true \n"
              "LT LF@check LF@p2 LF@p1 \n"
              "JUMPIFEQ $substr$end LF@check bool@true \n"
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
              "LABEL $substr$end \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief Generates nil check function.
 */
static void nil_check_func() {
    ADD_INSTR("LABEL $$nil_check \n"
              "POPS GF@%expr_result2 \n"
              "POPS GF@%expr_result \n"
              "JUMPIFEQ $$ERROR_NIL GF@%expr_result nil@nil \n"
              "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
              "PUSHS GF@%expr_result \n"
              "PUSHS GF@%expr_result2 \n"
              "RETURN \n");
}

/*
 * @brief Generates to_bool type casting function.
 */
static void recast_to_bool_func() {
    ADD_INSTR("LABEL $$recast_to_bool \n"
              "JUMPIFNEQ $$recast_to_bool$not_nil GF@%expr_result nil@nil \n"
              "MOVE GF@%expr_result bool@false \n"
              "JUMP $$recast_to_bool$end \n"
              "LABEL $$recast_to_bool$not_nil \n"
              "MOVE GF@%expr_result bool@true \n"
              "LABEL $$recast_to_bool$end \n"
              "RETURN \n");
}

/*
 * @brief Generates function for computing the power.
 */
static void generate_power_func() {
    ADD_INSTR("LABEL $$power \n"
              "PUSHFRAME \n"
              "DEFVAR LF@%res \n"
              "MOVE LF@%res float@0x1p+0 \n"
              "DEFVAR LF@%exp \n"
              "POPS LF@%exp \n"
              "DEFVAR LF@%base \n"
              "POPS LF@%base \n"
              "# make sure exp has zero decimal part \n"
              "FLOAT2INT LF@%exp LF@%exp \n"
              "INT2FLOAT LF@%exp LF@%exp \n"
              "# if exp < 0 -> \n"
              "LT GF@%expr_result LF@%exp float@0x0p+0 \n"
              "JUMPIFEQ $$power$while GF@%expr_result bool@false \n"
              "# check base != 0 \n"
              "JUMPIFEQ $$ERROR_DIV_BY_ZERO LF@%base float@0x0p+0 \n"
              "# base = 1 / base, exp = exp * (-1) \n"
              "DIV LF@%base float@0x1p+0 LF@%base \n"
              "MUL LF@%exp LF@%exp float@-0x1p+0 \n"
              "# while (exp != 0) \n"
              "LABEL $$power$while \n"
              "JUMPIFEQ $$power$end LF@%exp float@0x0p+0 \n"
              "     MUL LF@%res LF@%res LF@%base \n"
              "     SUB LF@%exp LF@%exp float@0x1p+0 \n"
              "     JUMP $$power$while \n"
              "LABEL $$power$end \n"
              "PUSHS LF@%res \n"
              "POPFRAME \n"
              "RETURN \n");
}

/*
 * @brief Generates function for short-circuit "or" evaluation.
 */
static void generate_ors_short() {
    ADD_INSTR("LABEL $$ors_short \n"
              "POPS GF@%expr_result \n"
              "POPS GF@%expr_result2 \n"
              "JUMPIFEQ $$ERROR_NIL GF@%expr_result nil@nil \n"
              "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
              "JUMPIFEQ $$ors$true GF@%expr_result2 bool@true \n"
              "JUMPIFEQ $$ors$true GF@%expr_result bool@true \n"
              "PUSHS bool@false \n"
              "JUMP $$ors$end \n"
              "LABEL $$ors$true \n"
              "PUSHS bool@true \n"
              "LABEL $$ors$end \n"
              "RETURN \n");
}

/*
 * @brief Generates function for short-circuit "and" evaluation.
 */
static void generate_ands_short() {
    ADD_INSTR("LABEL $$ands_short \n"
              "POPS GF@%expr_result \n"
              "POPS GF@%expr_result2 \n"
              "JUMPIFEQ $$ERROR_NIL GF@%expr_result nil@nil \n"
              "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
              "JUMPIFEQ $$ands$false GF@%expr_result2 bool@false \n"
              "JUMPIFEQ $$ands$false GF@%expr_result bool@false \n"
              "PUSHS bool@true \n"
              "JUMP $$ands$end \n"
              "LABEL $$ands$false \n"
              "PUSHS bool@false \n"
              "LABEL $$ands$end \n"
              "RETURN \n");
}

/*
 * @brief Initialises the code generator.
 */
static void initialise_generator() {
    debug_msg("\n");
    tmp_instr = Dynstring.ctor("");
    // initialise the instructions structure
    instructions.startList = List.ctor();
    instructions.instrListFunctions = List.ctor();
    instructions.mainList = List.ctor();
    instructions.in_loop = false;
    instructions.outer_loop_id = 0;
    instructions.before_loop_start = NULL;
    instructions.outer_cond_id = 0;
    instructions.cond_cnt = 1;
    instructions.cond_info = Dynstring.ctor("");
    // sets instructions list active
    instrList = instructions.startList;
}

/*
 * @brief Cleans the code generator.
 */
static void dtor() {
    debug_msg("\n");
    List.dtor(instructions.startList, (void (*)(void *)) (Dynstring.dtor));
    List.dtor(instructions.instrListFunctions, (void (*)(void *)) Dynstring.dtor);
    List.dtor(instructions.mainList, (void (*)(void *)) Dynstring.dtor);
    Dynstring.dtor(instructions.cond_info);
    Dynstring.dtor(tmp_instr);
}

/*
 * @brief Prints the list of instructions.
 */
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
                if (str_id[i] == ' ' || !isprint(str_id[i]) || str_id[i] == '#' || str_id[i] == '\\') {
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
            sprintf(str_tmp, "%lu", token.attribute.num_i);
            ADD_INSTR_PART(str_tmp);
            break;
        case KEYWORD_nil:
            ADD_INSTR_PART("nil@nil");
            break;
        case KEYWORD_0:
            ADD_INSTR_PART("bool@false");
            break;
        case KEYWORD_1:
            ADD_INSTR_PART("bool@true");
            break;
        case TOKEN_ID:
            ADD_INSTR_PART("LF@%");
            symbol_t *symbol;
            if (!Symstack.get_local_symbol(symstack, token.attribute.id, &symbol)) {
                ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
            } else {
                ADD_INSTR_INT(symbol->id_of_parent_scope);
            }
            ADD_INSTR_PART("%");
            ADD_INSTR_PART_DYN(token.attribute.id);
            break;
        default:
            ADD_INSTR_PART("unexpected_token");
            break;
    }
}

/*
 * @brief Generates the name of variable.
 *        scope_id%name
 * @param new_def true if the variable is being declared now
 *        false if it should be found in the symtable
 */
static void generate_var_name(dynstring_t *var_name, bool new_def) {
    symbol_t *symbol = NULL;
    if (new_def || !Symstack.get_local_symbol(symstack, var_name, &symbol)) {
        ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    } else {
        ADD_INSTR_INT(symbol->id_of_parent_scope);
    }
    ADD_INSTR_PART("%");
    ADD_INSTR_PART_DYN(var_name);
}

/*
 * @brief Generates DEFVAR LF@%var.
 */
static void generate_defvar(dynstring_t *var_name) {
    ADD_INSTR_PART("DEFVAR LF@%");
    generate_var_name(var_name, true);  // true == new variable
    if (instructions.in_loop) {
        ADD_INSTR_WHILE();
    } else {
        ADD_INSTR_TMP();
    }
}

/*
 * @brief Generates variable declaration.
 */
static void generate_var_declaration(dynstring_t *var_name) {
    generate_defvar(var_name);

    // initialise to nil
    ADD_INSTR_PART("MOVE LF@%");
    generate_var_name(var_name, true);  // true == new variable
    ADD_INSTR_PART(" nil@nil");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates variable definition.
 */
static void generate_var_definition(dynstring_t *var_name) {
    generate_defvar(var_name);

    ADD_INSTR_PART("MOVE LF@%");
    generate_var_name(var_name, true);   // true == new variable
    ADD_INSTR_PART(" GF@%expr_result");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates variable used for code generating.
 */
static void generate_tmp_var_definition(char *var_name) {
    dynstring_t *name = Dynstring.ctor(var_name);
    generate_defvar(name);

    ADD_INSTR_PART("MOVE LF@%");
    generate_var_name(name, true);  // true == new variable
    ADD_INSTR_PART(" GF@%expr_result");
    ADD_INSTR_TMP();
    Dynstring.dtor(name);
}

/*
 * @brief Generates assignment to a variable
 *        MOVE LF@%0%i GF@%expr_result
 */
static void generate_var_assignment(dynstring_t *var_name) {
    ADD_INSTR_PART("MOVE LF@%");
    generate_var_name(var_name, false); // false == already declared var
    ADD_INSTR_PART(" GF@%expr_result");
    ADD_INSTR_TMP();
}

/*
 * @brief Converts GF@%expr_result int -> float
 */
static void recast_expression_to_bool(void) {
    ADD_INSTR("CALL $$recast_to_bool");
}

/*
 * @brief Generates division check.
 * @param is_integer specifies whether the number
 *        to check is integer (true) or float (false).
 */
static void generate_division_check(bool is_integer) {
    ADD_INSTR("# zero division check");
    ADD_INSTR("POPS GF@%expr_result");
    if (is_integer) {
        ADD_INSTR("JUMPIFEQ $$ERROR_DIV_BY_ZERO GF@%expr_result int@0");
    } else {
        ADD_INSTR("JUMPIFEQ $$ERROR_DIV_BY_ZERO GF@%expr_result float@0x0p+0");
    }
    ADD_INSTR("PUSHS GF@%expr_result");
}

/*
 * @brief Generates nil check.
 */
static void generate_nil_check() {
    ADD_INSTR("# nil check");
    ADD_INSTR("CALL $$nil_check");
}

/*
 * @brief Converts first or both int expressions to float.
 * @param expr stores info about the expr to be converted.
 */
//static void retype_first_or_both(expr_semantics_t *expr) {
//    ADD_INSTR("# convert int -> float");
//    ADD_INSTR("POPS GF@%expr_result2 \n"
//              "POPS GF@%expr_result \n"
//              "INT2FLOAT GF@%expr_result GF@%expr_result");
//    if (expr->conv_type == CONVERT_BOTH) {
//        ADD_INSTR("# convert int -> float");
//        ADD_INSTR("INT2FLOAT GF@%expr_result2 GF@%expr_result2");
//    }
//    ADD_INSTR("PUSHS GF@%expr_result \n"
//              "PUSHS GF@%expr_result2");
//}

/*
 * @brief Converts second int expression to float.
 * @param expr stores info about the expr to be converted.
 */
static void retype_second() {
    ADD_INSTR("# convert int -> float");
    ADD_INSTR("POPS GF@%expr_result \n"
              "INT2FLOAT GF@%expr_result GF@%expr_result \n"
              "PUSHS GF@%expr_result");
}

/*
 * @brief Generates conversion int -> float if needed
 *        and pushes operands on the stack.
 * @param expr stores info about the expr to be converted.
 */
//static void retype_and_push(expr_semantics_t *expr) {
//    if (expr->conv_type == CONVERT_FIRST || expr->conv_type == CONVERT_BOTH) {
//        retype_first_or_both(expr);
//    }
//    if (expr->conv_type == CONVERT_SECOND) {
//        retype_second();
//    }
//}

/*
 * @brief Generates code for pushing operand on the stack (with nil check).
 */
static void generate_expression_operand(token_t token) {
    ADD_INSTR_PART("PUSHS ");
    generate_var_value(token);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates binary operation.
 */
static void generate_expression_binary(op_list_t op) {
    switch (op) {
        case OP_ADD:    // '+'
            generate_nil_check();
            ADD_INSTR("ADDS");
            break;
        case OP_SUB:    // '-'
            generate_nil_check();
            ADD_INSTR("SUBS");
            break;
        case OP_MUL:    // '*'
            generate_nil_check();
            ADD_INSTR("MULS");
            break;
        case OP_DIV_I:  // '/'
            generate_nil_check();
            generate_division_check(true); // true == int div check
            ADD_INSTR("IDIVS");
            break;
        case OP_DIV_F:  // '//'
            generate_nil_check();
            generate_division_check(false); // false == float div check
            ADD_INSTR("DIVS");
            break;
        case OP_LT:     // '<'
            generate_nil_check();
            ADD_INSTR("LTS");
            break;
        case OP_LE:     // '<='
            ADD_INSTR("POPS GF@%expr_result2 \n"
                      "POPS GF@%expr_result \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result nil@nil \n"
                      "LT GF@%expr_result3 GF@%expr_result GF@%expr_result2 \n"
                      "EQ GF@%expr_result2 GF@%expr_result GF@%expr_result2 \n"
                      "OR GF@%expr_result GF@%expr_result2 GF@%expr_result3 \n"
                      "PUSHS GF@%expr_result");
            break;
        case OP_GT:     // '>'
            generate_nil_check();
            ADD_INSTR("GTS");
            break;
        case OP_GE:     // '>='
            ADD_INSTR("POPS GF@%expr_result2 \n"
                      "POPS GF@%expr_result \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result nil@nil \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
                      "GT GF@%expr_result3 GF@%expr_result GF@%expr_result2 \n"
                      "EQ GF@%expr_result2 GF@%expr_result GF@%expr_result2 \n"
                      "OR GF@%expr_result GF@%expr_result2 GF@%expr_result3 \n"
                      "PUSHS GF@%expr_result");
            break;
        case OP_EQ:     // '=='
            ADD_INSTR("EQS");
            break;
        case OP_NE:     // '~='
            ADD_INSTR("EQS \n"
                      "NOTS");
            break;
        case OP_AND:    // 'and'
            ADD_INSTR("CALL $$ands_short");
            break;
        case OP_OR:     // 'or'
            ADD_INSTR("CALL $$ors_short");
            break;
        case OP_STRCAT: // '..'
            ADD_INSTR("POPS GF@%expr_result2 \n"
                      "POPS GF@%expr_result \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result nil@nil \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
                      "CONCAT GF@%expr_result GF@%expr_result GF@%expr_result2 \n"
                      "PUSHS GF@%expr_result");
            break;
        default:
            ADD_INSTR("# unrecognized_operation");
    }

}

/*
 * @brief Generates unary operation.
 */
static void generate_expression_unary(op_list_t op) {
    switch (op) {
        case OP_NOT:    // 'not'
            ADD_INSTR("POPS GF@%expr_result2 \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
                      "PUSHS GF@%expr_result2");
            ADD_INSTR("NOTS");
            break;
        case OP_HASH:   // '#'
            ADD_INSTR("POPS GF@%expr_result2 \n"
                      "JUMPIFEQ $$ERROR_NIL GF@%expr_result2 nil@nil \n"
                      "STRLEN GF@%expr_result GF@%expr_result2 \n"
                      "PUSHS GF@%expr_result");
            break;
        default:
            ADD_INSTR("# unrecognized_operation");
    }

}

/*
 * @brief Generates pop from the stack to GF@%expr_result.
 */
static void generate_expression_pop() {
    ADD_INSTR("POPS GF@%expr_result");
}

/*
 * @brief Saves info about current cond scope into
 *        global dynstring instructions.cond_info
 *        - gets info from instructions struct
 */
static void push_cond_info() {
    Dynstring.append(instructions.cond_info, instructions.cond_cnt);
    char cond_id_str[6] = "\0";
    sprintf(cond_id_str, "%.5lu", instructions.outer_cond_id);
    dynstring_t *new_id_str = Dynstring.ctor(cond_id_str);
    Dynstring.cat(instructions.cond_info, new_id_str);
    Dynstring.dtor(new_id_str);
}

/*
 * @brief Gets info about current cond scope from
 *        global dynstring instructions.cond_info
 *        - saves info to outer_cond_id and cond_cnt
 */
static void pop_cond_info() {
    long unsigned num = strtoul(&Dynstring.c_str(instructions.cond_info)
                [Dynstring.len(instructions.cond_info) - 5], NULL, 10);
    instructions.outer_cond_id = num;
    instructions.cond_cnt = Dynstring.c_str(instructions.cond_info)
                                        [Dynstring.len(instructions.cond_info) - 6];
    Dynstring.trunc_to_len(instructions.cond_info, Dynstring.len(instructions.cond_info) - 6);
}

/*
 * @brief Generates condition label.     LABEL $if$id$scope_num
 */
static void generate_cond_label(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("LABEL $if$");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$");
    ADD_INSTR_INT(cond_num);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates start of if block. The result of expression
 *        in the condition is expected in LF@%result variable.
 *         generates: JUMPIFNEQ $if$id$next_cond LF@%result bool@true
 */
static void generate_cond_if(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR("\n# condition - if check");
    ADD_INSTR_PART("JUMPIFNEQ $if$");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$");
    ADD_INSTR_INT(cond_num);
    ADD_INSTR_PART(" GF@%expr_result bool@true");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates start of else if block - JUMP $end
 *        from the previous block and LABEL for new elseif block.
 * generates sth like: JUMP $if$id$end
 *                     LABEL $if$id$scope_num
 */
static void generate_cond_elseif(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("JUMP $if$");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    ADD_INSTR("\n# condition - elseif part");
    generate_cond_label(if_scope_id, cond_num);
}

/*
 * @brief Generates start of else statement.
 *          JUMP $if$id$end
 *          LABEL $if$id$scope_num
 */
static void generate_cond_else(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR_PART("JUMP $if$");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    ADD_INSTR("\n# condition - else part");
    generate_cond_label(if_scope_id, cond_num - 1);
}

/*
 * @brief Generates end of if statement.
 * generates sth like: LABEL $if$id$end
 *                     LABEL $if$id$scope_num
 */
static void generate_cond_end(size_t if_scope_id, size_t cond_num) {
    ADD_INSTR("\n# condition end");
    ADD_INSTR_PART("LABEL $if$");
    ADD_INSTR_INT(if_scope_id);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    generate_cond_label(if_scope_id, cond_num);
    ADD_INSTR("");
}

/*
 * @brief Generates break instruction.
 */
static void generate_break() {
    ADD_INSTR_PART("\n# break \n");
    ADD_INSTR_PART("JUMP $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
    ADD_INSTR("");
}

/*
* @brief Generates label end.
*        LABEL $end$id
*/
static void generate_end() {
    ADD_INSTR("#generate_end");
    ADD_INSTR_PART("LABEL $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
    ADD_INSTR("");
}

/*
 * @brief Generates while loop header.
 * generates sth like: LABEL $while$id
 */
static void generate_while_header() {
    ADD_INSTR("\n# while");
    ADD_INSTR_PART("LABEL $while$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates while loop condition check.
 * generates sth like: JUMPIFNEQ $end$id GF@%expr_result bool@true
 */
static void generate_while_cond() {
    ADD_INSTR_PART("JUMPIFNEQ $end$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART(" GF@%expr_result bool@true");
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

    generate_end();
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
 * @brief Generates repeat until loop condition check and end label.
 * generates sth like: JUMPIFEQ $repeat$id GF@%expr_result bool@true
 *                     LABEL $end$id
 */
static void generate_repeat_until_cond() {
    ADD_INSTR_PART("JUMPIFNEQ $repeat$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART(" GF@%expr_result bool@true");
    ADD_INSTR_TMP();

    generate_end();
}

/*
 * @brief Generates variable for default step in for loop,
 *        initialise it to 1.
 */
static void generate_for_default_step() {
    dynstring_t *var_name = Dynstring.ctor("step");
    generate_defvar(var_name);

    ADD_INSTR_PART("MOVE LF@%");
    generate_var_name(var_name, true);
    ADD_INSTR_PART(" int@1");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates for loop condition check.
 * @param var_name name of the control variable
 */
static void generate_for_cond(dynstring_t *var_name) {
    size_t scope_id = Symstack.get_scope_info(symstack).unique_id;
    ADD_INSTR("\n# for");
    ADD_INSTR_PART("LABEL $for$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART(  "\n# check if step is < 0 \n"
                     "LT GF@%expr_result LF@%");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("%step int@0 \n"
                   "JUMPIFEQ $for$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("$step_le GF@%expr_result bool@true \n"
                   "    # step >= 0 \n"
                   "    # if i <= cond then break \n"
                   "    PUSHS LF@%");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("%");
    ADD_INSTR_PART_DYN(var_name);
    ADD_INSTR_PART("\n    PUSHS LF@%");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("%terminating_cond");
    ADD_INSTR_PART("\n    POPS GF@%expr_result2 \n"
                   "    POPS GF@%expr_result \n"
                   "    LT GF@%expr_result3 GF@%expr_result GF@%expr_result2 \n"
                   "    EQ GF@%expr_result2 GF@%expr_result GF@%expr_result2 \n"
                   "    OR GF@%expr_result GF@%expr_result2 GF@%expr_result3 \n"
                   "    PUSHS GF@%expr_result \n"
                   "    JUMPIFNEQ $end$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART(" GF@%expr_result bool@true \n"
                   "    JUMP $for$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("$body \n"
                   "LABEL $for$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("$step_le \n"
                   "    # step < 0 \n"
                   "    # if i >= cond then break \n"
                   "    PUSHS LF@%");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("%");
    ADD_INSTR_PART_DYN(var_name);
    ADD_INSTR_PART("\n    PUSHS LF@%cond \n"
                   "    POPS GF@%expr_result2 \n"
                   "    POPS GF@%expr_result \n"
                   "    GT GF@%expr_result3 GF@%expr_result GF@%expr_result2 \n"
                   "    EQ GF@%expr_result2 GF@%expr_result GF@%expr_result2 \n"
                   "    OR GF@%expr_result GF@%expr_result2 GF@%expr_result3 \n"
                   "    PUSHS GF@%expr_result \n"
                   "    JUMPIFNEQ $end$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART(" GF@%expr_result bool@true \n"
                   "\nLABEL $for$");
    ADD_INSTR_INT(scope_id);
    ADD_INSTR_PART("$body \n"
                   "# for loop body \n");
}

/*
 * @brief Generates for loop end.
 * generates sth like: ADD %var value
 *                     JUMP $for$id
 *                     LABEL $end$id
 */
static void generate_for_end(dynstring_t *var_name) {
    ADD_INSTR("# for loop end");
    ADD_INSTR_PART("ADD LF@%");\
    generate_var_name(var_name, false);
    ADD_INSTR_PART(" LF@%");
    generate_var_name(var_name, false);
    ADD_INSTR_PART(" LF@%");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_PART("%step");
    ADD_INSTR_TMP();
    ADD_INSTR_PART("JUMP $for$");
    ADD_INSTR_INT(Symstack.get_scope_info(symstack).unique_id);
    ADD_INSTR_TMP();

    generate_end();
}

/*
 * @brief Generates function definition start.
 * generates sth like: LABEL $foo
 *                     PUSHFRAME
 */
static void generate_func_start(dynstring_t *func_name) {
    INSTR_CHANGE_ACTIVE_LIST(instructions.instrListFunctions);
    ADD_INSTR_PART("\nLABEL $");   // add name of function
    ADD_INSTR_PART_DYN(func_name);
    ADD_INSTR_TMP();
    ADD_INSTR("PUSHFRAME");
}

/*
 * @brief Generates function definition end.
 */
static void generate_func_end(char *func_name) {
    ADD_INSTR("# function end");
    ADD_INSTR_PART("LABEL $");
    ADD_INSTR_PART(func_name);
    ADD_INSTR_PART("$end");
    ADD_INSTR_TMP();

    ADD_INSTR("POPFRAME");
    ADD_INSTR("RETURN\n");
    INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
}

/*
 * @brief Generates passing param from TF to LF.
 * generates sth like:
 *      DEFVAR LF@%param
 *      MOVE LF@%param LF@%0
 */
static void generate_func_start_param(dynstring_t *param_name, size_t index) {
    ADD_INSTR("# generate_function_start_param");
    ADD_INSTR_PART("DEFVAR LF@%");
    generate_var_name(param_name, true);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("MOVE LF@%");
    generate_var_name(param_name, true);
    ADD_INSTR_PART(" LF@%");
    ADD_INSTR_INT(index);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates passing return value.
 * generates sth like:
 *          MOVE LF@%return0 GF@%expr_type
 */
static void generate_func_pass_return(size_t index) {
    ADD_INSTR_PART("MOVE LF@%return");
    ADD_INSTR_INT(index);
    ADD_INSTR_PART(" GF@%expr_result");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates return value of return parameter with index.
 * generates sth like:
 *      DEFVAR LF@%return0
 *      MOVE LF@%return0 nil@nil
 */
static void generate_func_return_value(size_t index) {
    ADD_INSTR("# generate_function_return_value");
    ADD_INSTR_PART("DEFVAR LF@%return");
    ADD_INSTR_INT(index);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("MOVE LF@%return");
    ADD_INSTR_INT(index);
    ADD_INSTR_PART(" nil@nil");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates creation of a frame before passing parameters to a function
 */
static void generate_func_createframe() {
    ADD_INSTR("CREATEFRAME");
}

/*
 * @brief Generates parameter pass to a function.
 * generates sth like:
 *          DEFVAR TF@%0
 *          MOVE TF@%0 GF@%expr_result
 */
static void generate_func_call_pass_param(size_t param_index) {
    ADD_INSTR_PART("DEFVAR TF@%");
    ADD_INSTR_INT(param_index);
    ADD_INSTR_TMP();

    ADD_INSTR_PART("MOVE TF@%");
    ADD_INSTR_INT(param_index);
    ADD_INSTR_PART(" GF@%expr_result");
    ADD_INSTR_TMP();
}

/*
 * @brief Generates function call.
 */
static void generate_func_call(char *func_name) {
    ADD_INSTR_PART("CALL $");
    ADD_INSTR_PART(func_name);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates getting return value after function call.
 * generates sth like:  MOVE LF%id%res TF@%return0
 */
static void generate_func_call_return_value(size_t index) {
    //ADD_INSTR_PART("MOVE GF@%expr_result TF@%return");
    ADD_INSTR_PART("PUSHS TF@%return");
    ADD_INSTR_INT(index);
    ADD_INSTR_TMP();
}

/*
 * @brief Generates start of main scope.
 */
static void generate_main_start() {
    INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
    ADD_INSTR("\n# main scope");
    ADD_INSTR("LABEL $$MAIN");
    ADD_INSTR("CREATEFRAME");
    ADD_INSTR("PUSHFRAME");
}

/*
 * @brief Generates end of main scope.
 */
static void generate_main_end() {
    ADD_INSTR("LABEL $$MAIN$end");
    ADD_INSTR("CLEARS");

    // TODO remove
    ADD_INSTR("WRITE GF@%expr_result");
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
    ADD_INSTR("LABEL $$ERROR_NIL \n"
              "EXIT int@8 \n"
              "LABEL $$ERROR_DIV_BY_ZERO \n"
              "EXIT int@9");

    INSTR_CHANGE_ACTIVE_LIST(instructions.instrListFunctions);
    generate_func_ord();
    generate_func_chr();
    generate_func_substr();
    generate_func_reads();
    generate_func_readi();
    generate_func_readn();
    generate_func_write();
    generate_func_tointeger();
    generate_power_func();
    nil_check_func();
    recast_to_bool_func();
    generate_ors_short();
    generate_ands_short();

    INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
    generate_main_start();
}

/*
 * @brief Generates comment.
 */
static void generate_comment(char *comment) {
    ADD_INSTR_PART("# ");
    ADD_INSTR_PART(comment);
    ADD_INSTR_TMP();
}

/**
 * Interface to use when dealing with code generator.
 * Functions are in struct so we can use them in different files.
 */
const struct code_generator_interface_t Generator = {
        .initialise = initialise_generator,
        .dtor = dtor,
        .print_instr_list = Print_instr_list,
        .var_declaration = generate_var_declaration,
        .var_definition = generate_var_definition,
        .tmp_var_definition = generate_tmp_var_definition,
        .var_assignment = generate_var_assignment,
        .recast_expression_to_bool = recast_expression_to_bool,
        .expression_operand = generate_expression_operand,
        .expression_unary = generate_expression_unary,
        .expression_binary = generate_expression_binary,
        .expression_pop = generate_expression_pop,
        .push_cond_info = push_cond_info,
        .pop_cond_info = pop_cond_info,
        .cond_if = generate_cond_if,
        .cond_elseif = generate_cond_elseif,
        .cond_else = generate_cond_else,
        .cond_end = generate_cond_end,
        .instr_break = generate_break,
        .while_header = generate_while_header,
        .while_cond = generate_while_cond,
        .while_end = generate_while_end,
        .repeat_until_header = generate_repeat_until_header,
        .repeat_until_cond = generate_repeat_until_cond,
        .for_default_step = generate_for_default_step,
        .for_cond = generate_for_cond,
        .for_end = generate_for_end,
        .func_start = generate_func_start,
        .func_end = generate_func_end,
        .func_start_param = generate_func_start_param,
        .func_pass_return = generate_func_pass_return,
        .func_return_value = generate_func_return_value,
        .func_createframe = generate_func_createframe,
        .func_call_pass_param = generate_func_call_pass_param,
        .func_call = generate_func_call,
        .func_call_return_value = generate_func_call_return_value,
        .main_end = generate_main_end,
        .prog_start = generate_prog_start,
        .comment = generate_comment,
};
