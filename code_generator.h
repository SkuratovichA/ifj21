/**
 * @file code_generator.h
 *
 * @author Lucie Svobodova
 */

#pragma once

#include "dynstring.h"
#include "list.h"
#include "scanner.h"
#include "parser.h"

#define MAX_CHAR 23         // maximum characters when converting num to string

/*
 * Structure that holds information needed for the code generator.
 */
typedef struct {
    list_t *startList;                  // instrs that are first in the program
    list_t *instrListFunctions;         // instr list for defining functions
    list_t *mainList;                   // instr list for the main scope
    bool in_loop;                       // var that indicates whether we are in loop
    size_t outer_loop_id;               // id of scope of the most outer loop
    list_item_t *before_loop_start;     // ptr to instr before the most outer loop
                                        // if (!in_loop) before_loop_start == NULL
    size_t outer_cond_id;               // id of scope of the most outer if
    char cond_cnt;                      // counter of elseif/else branches after if
    dynstring_t *cond_info;             // dynstring with info about nested ifs
} instructions_t;

typedef enum instr_list {
    LIST_INSTR_START,
    LIST_INSTR_FUNC,
    LIST_INSTR_MAIN,
} instr_list_t;

extern instructions_t instructions;
extern list_t *instrList;

/**
 * A structure that store pointers to the functions from code_generator.c. So we can use them in different files as interface.
 */
struct code_generator_interface_t {

    /*
     * @brief Initialises the code generator.
     */
    void (*initialise)(void);

    /*
     * @brief Cleans the code generator.
     */
    void (*dtor)(void);

    /*
     * @brief Prints the list of instructions.
     */
    void (*print_instr_list)(instr_list_t);

    /*
     * @brief Generates variable declaration.
     */
    void (*var_declaration)(dynstring_t *);

    /*
     * @brief Generates variable declaration.
     */
    void (*var_definition)(dynstring_t *);

    /*
     * @brief Generates variable used for code generating.
     */
    void (*tmp_var_definition)(char *);

    /*
     * @brief Generates assignment to a variable
     *        MOVE LF@%0%i GF@%expr_result
     */
    void (*var_assignment)(dynstring_t *);

    /*
     * @brief Converts GF@%expr_result int -> float
     */
    void (*recast_expression_to_bool)(void);

    /*
     * @brief Generates expressions reduce.
     * @param expr stores info about the expr to be processed.
     */
    void (*expression_operand)(void);

    /*
     * @brief Generates expressions reduce.
     * @param expr stores info about the expr to be processed.
     */
    void (*expression_unary)(void);

    /*
     * @brief Generates expressions reduce.
     * @param expr stores info about the expr to be processed.
     */
    void (*expression_binary)(void);

    /*
     * @brief Generates pop from the stack to GF@%expr_result.
     */
    void (*expression_pop)(void);

    /*
     * @brief Gets info about current cond scope from
     *        global dynstring instructions.cond_info
     */
    void (*push_cond_info)(void);

    /*
     * @brief Saves info about current cond scope into
     *        global dynstring instructions.cond_info
     */
    void (*pop_cond_info)(void);

    /*
     * @brief Generates start of if block. The result of expression
     *        in the condition is expected in LF@%result variable.
     */
    void (*cond_if)(size_t, size_t);

    /*
     * @brief Generates start of else if block - JUMP $end
     *        from the previous block and LABEL for new elseif block.
     */
    void (*cond_elseif)(size_t, size_t);

    /*
     * @brief Generates start of else statement.
     */
    void (*cond_else)(size_t, size_t);

    /*
     * @brief Generates end of if statement.
     */
    void (*cond_end)(size_t, size_t);

    /*
     * @brief Generates break instruction.
     */
    void (*instr_break)(void);

    /*
     * @brief Generates while loop header.
     */
    void (*while_header)(void);

    /*
     * @brief Generates while loop condition check.
     */
    void (*while_cond)(void);

    /*
     * @brief Generates while loop end.
     */
    void (*while_end)(void);

    /*
     * @brief Generates repeat until loop header.
     */
    void (*repeat_until_header)(void);

    /*
     * @brief Generates repeat until loop condition check and end label.
     */
    void (*repeat_until_cond)(void);

    /*
     * @brief Generates variable for default step in for loop,
     *        initialise it to 1.
     */
    void (*for_default_step)(void);

    /*
     * @brief Generates for loop condition check.
     */
    void (*for_cond)(dynstring_t *);

    /*
     * @brief Generates for loop end.
     */
    void (*for_end)(dynstring_t *);

    /*
     * @brief Generates function definition start.
     */
    void (*func_start)(dynstring_t *);

    /*
     * @brief Generates function definition end.
     */
    void (*func_end)(char *);

    /*
     * @brief Generates passing param from TF to LF.
     */
    void (*func_start_param)(dynstring_t*, size_t);

    /*
     * @brief Generates passing return value.
     */
    void (*func_pass_return)(size_t);

    /*
     * @brief Generates return value of return parameter with index.
     */
    void (*func_return_value)(size_t);

    /*
     * @brief Generates creation of a frame before passing parameters to a function
     */
    void (*func_createframe)(void);

    /*
     * @brief Generates parameter pass to a function.
     */
    void (*func_call_pass_param)(size_t);

    /*
     * @brief Generates function call.
     */
    void (*func_call)(char *);

    /*
     * @brief Generates getting return value after function call.
     */
    void (*func_call_return_value)(dynstring_t *, size_t);

    /*
     * @brief Generates end of main scope.
     */
    void (*main_end)(void);

    /*
     * @brief Generates program start (adds header, define built-in functions).
     */
    void (*prog_start)(void);

    /*
     * @brief Generates comment.
     */
    void (*comment)(char *);
};

// Functions from code_generator.c will be visible in different file under Generator name.
extern const struct code_generator_interface_t Generator;
