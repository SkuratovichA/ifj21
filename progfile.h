/********************************************
 * Project name: IFJ - projekt
 * File: progfile.c
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
 *  @package progfile
 *  @file progfile.c
 *  @brief Contain enum of all the error codes.
 *
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
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "dynstring.h"


/**
 * Struct is hidden. definition is in dynstring.c
 */
typedef struct c_progfile progfile_t;

extern const struct progfile_op_struct_t Progfile;

/**
 * Interface to use when dealing with strings.
 */
struct progfile_op_struct_t {
    bool (*check_postfix)(const char *, const char *);
    progfile_t *(*getfile)(const char *, const char *);
    progfile_t *(*getfile_stdin)(void);
    void (*free)(progfile_t *);
    int (*ungetc)(progfile_t *);
    int (*pgetc)(progfile_t *);
    char *(*get_tape)(progfile_t *);
    char *(*get_tape_current)(progfile_t *);
    void (*set_tape)(progfile_t *, char *);
    char (*peek_at)(progfile_t *, size_t);
};
