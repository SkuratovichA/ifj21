/**
 * @file parser.h
 *
 * @brief Header file for parser.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#pragma once


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
#include "symtable.h"
#include "symstack.h"
#include "debug.h"

/** A symbol stack with symbol tables for the program.
 */
symstack_t *symstack;

/** Global scope(the first one) with function declarations and definitions.
 */
symtable_t *global_table;

/** A current table.
 */
symtable_t *local_table;

int nested_cycle_level;

extern const struct parser_interface_t Parser;

struct parser_interface_t {
    bool (*analyse)(pfile_t *);
};
