#pragma once

#include "dynstring.h"
#include "list.h"
#include "scanner.h"

#define MAX_CHAR 23         // maximum characters when converting num to string

list_t *instrList;          // global list of instructions
dynstring_t *tmp_instr;     // instruction that is currently being generated

typedef struct {
    list_t *startList;
    list_t *instrListFunctions;
    list_t *mainList;
    list_item_t *before_loop_start;
    bool in_loop;
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
 * Converts integer to string and adds it to tmp_instr
 */
#define ADD_INSTR_INT(num)                  \
do {                                        \
    char str[MAX_CHAR] = "\0";              \
    sprintf(str, "%d", num);                \
    ADD_INSTR_PART(str);                    \
} while (0)                                 \

/*
 * Change active list of instructions.
 */
#define INSTR_CHANGE_ACTIVE_LIST(newList)   \
do {                                        \
    instrList->head = (newList)->head;      \
    instrList->tail = (newList)->tail;      \
} while (0)

/**
 * A structure that store pointers to the functions from code_generator.c. So we can use them in different files as interface.
 */
struct code_generator_interface_t {
    void (*prog_start)(void);
    void (*func_start)(void);
    void (*func_end)(void);
    void (*func_return_value)(unsigned index);
    void (*func_pass_param)(int param_index);
    void (*func_createframe)(void);
    void (*main_end)(void);
    void (*func_call)(void);
    void (*var_declaration)(void);
    void (*var_definition)(token_t token);
};

// Functions from code_generator.c will be visible in different file under Generator name.
extern const struct code_generator_interface_t Generator;
