#pragma once

#include "dynstring.h"
#include "list.h"
#include "scanner.h"
#include "parser.h"

#define MAX_CHAR 23         // maximum characters when converting num to string

list_t *instrList;          // global list of instructions
dynstring_t *tmp_instr;     // instruction that is currently being generated

// TODO - use union
typedef struct {
    list_t *startList;
    list_t *instrListFunctions;
    list_t *mainList;
    list_item_t *before_loop_start;
    bool in_loop;
    size_t outer_loop_id;               // id of scope of the most outer loop
    size_t outer_cond_id;               // id of scope of the most inner if
    size_t cond_num;
} instructions_t;

instructions_t instructions;

/*
 * Adds new instruction to the list of instructions.
 */
#define ADD_INSTR(instr)                            \
do {                                                \
    dynstring_t *newInstr = Dynstring.ctor(instr);  \
    List.append(instrList, newInstr);               \
} while (0)

/*
 * Adds part of an instruction to global tmp_instr.
 */
#define ADD_INSTR_PART(instrPart)                               \
do {                                                            \
  dynstring_t * newInstrPart = Dynstring.ctor(instrPart);       \
  tmp_instr = Dynstring.cat(tmp_instr, newInstrPart);           \
  Dynstring.dtor(newInstrPart);                                 \
} while (0)

/*
 * Adds tmp_inst to the list of instructions.
 */
#define ADD_INSTR_TMP                                           \
do {                                                            \
    dynstring_t *newInstr = tmp_instr;                          \
    List.append(instrList, newInstr);                           \
    tmp_instr = Dynstring.ctor("");  /* Dynstring.clear? */     \
} while (0)

/*
 * Inserts tmp_inst before while loop.
 */
#define ADD_INSTR_WHILE                                             \
do {                                                                \
    dynstring_t *newInstr = tmp_instr;                              \
    List.insert_after(instructions.before_loop_start, newInstr);    \
    tmp_instr = Dynstring.ctor("");  /* Dynstring.clear? */         \
} while (0)

/*
 * Converts integer to string and adds it to tmp_instr
 */
#define ADD_INSTR_INT(num)                  \
do {                                        \
    char str[MAX_CHAR] = "\0";              \
    sprintf(str, "%lu", num);                \
    ADD_INSTR_PART(str);                    \
} while (0)                                 \

/*
 * Change active list of instructions.
 */
#define INSTR_CHANGE_ACTIVE_LIST(newList)   \
do {                                        \
    instrList = (newList);                  \
} while (0)

/**
 * A structure that store pointers to the functions from code_generator.c. So we can use them in different files as interface.
 */
struct code_generator_interface_t {
    void (*prog_start)(void);
    void (*func_start)(char *func_name);
    void (*func_end)(char *func_name);
    void (*func_start_param)(char *param_name, size_t index);
    void (*func_return_value)(size_t index);
    void (*func_pass_param)(size_t param_index);
    void (*func_createframe)(void);
    void (*main_end)(void);
    void (*func_call)(char *func_name);
    void (*var_declaration)(token_t token_id);
    void (*var_definition)(token_t token_id, token_t token_value);
    void (*cond_if)(size_t if_scope_id, size_t cond_num);
    void (*cond_elseif)(size_t if_scope_id, size_t cond_num);
    void (*cond_else)(size_t if_scope_id, size_t cond_num);
    void (*cond_end)(size_t if_scope_id, size_t cond_num);
    void (*while_header)(void);
    void (*while_cond)(void);
    void (*while_end)(void);
    void (*end)(void);
    void (*repeat_until_header)(void);
    void (*repeat_until_cond)(void);
    void (*for_header)(token_t a, token_t b);
    void (*for_cond)(void);
};

// Functions from code_generator.c will be visible in different file under Generator name.
extern const struct code_generator_interface_t Generator;
