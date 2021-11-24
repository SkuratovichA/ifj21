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
    list_t *startList;                  // instr that are first in the program
    list_t *instrListFunctions;         // instr list for defining functions
    list_t *mainList;                   // instr list for the main scope
    bool in_loop;                       // var that indicates whether we are in loop
    size_t outer_loop_id;               // id of scope of the most outer loop
    list_item_t *before_loop_start;     // pointer to the most outer loop (if !in_loop -> NULL)
    size_t outer_cond_id;               // id of scope of the most inner if
    size_t cond_cnt;                    // counter of elseif/else branches after if
    size_t label_cnt;                   // labels counter
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
     * @brief Generates program start (adds header, define built-in functions).
     */
    void (*prog_start)(void);

    void (*func_start)(dynstring_t *);
    void (*func_end)(char *);
    void (*func_start_param)(dynstring_t*, size_t);
    void (*func_return_value)(size_t);
    void (*func_pass_param)(size_t);

    /*
     * @brief Generates createframe instr before passing parameters to a function
     */
    void (*func_createframe)(void);

    /*
     * @brief Generates function end.
     */
    void (*main_end)(void);

    /*
     * @brief Generates function call.
     */
    void (*func_call)(dynstring_t *);

    void (*var_declaration)(dynstring_t *);
    void (*var_definition)(dynstring_t *);
    void (*var_assignment)(dynstring_t *);
    void (*cond_if)(size_t, size_t);
    void (*cond_elseif)(size_t, size_t);
    void (*cond_else)(size_t, size_t);
    void (*cond_end)(size_t, size_t);
    void (*while_header)(void);

    void (*while_cond)(void);

    void (*while_end)(void);

    void (*end)(void);

    void (*repeat_until_header)(void);

    void (*repeat_until_cond)(void);

    void (*for_header)(dynstring_t *);

    void (*for_cond)(void);

    void (*initialise)(void);

    void (*expression)(expr_semantics_t *);

    void (*expression_pop)(void);

    void (*dtor)(void);

    void (*print_instr_list)(instr_list_t);
};

// Functions from code_generator.c will be visible in different file under Generator name.
extern const struct code_generator_interface_t Generator;
