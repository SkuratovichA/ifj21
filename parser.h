/********************************************
 * Project name: IFJ - projekt
 * File: parser.h
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
 *  @package parser
 *  @file parser.h
 *  @brief Header for scanner.c with structures and macros.
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
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

#include "debug.h"


extern const struct parser_op_struct Parser;

struct parser_op_struct {
    bool (*analyse)(progfile_t *);
};