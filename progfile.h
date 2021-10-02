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


//TODO add comments into an interface
/**
 * Struct is hidden. definition is in dynstring.c
 */
typedef struct c_progfile progfile_t;

extern const struct progfile_op_struct_t Progfile;

/**
 * Interface to use when dealing with strings.
 */
struct progfile_op_struct_t {
    /**
     * @brief Store file to progfile structure.
     *
     * @param filename
     * @param mode File opening mode.
     * @return pfile where file is stored. If error returns NULL.
     */
    progfile_t *(*getfile)(const char *, const char *);

    /**
     * @brief Reads a file from stdin to progfile structure.
     *
     * @return File stored in pfile structure. If error return NULL.
     */
    progfile_t *(*getfile_stdin)(void);

    void (*free)(progfile_t *);

    int (*ungetc)(progfile_t *);

    int (*pgetc)(progfile_t *);

    char *(*get_tape)(progfile_t *);

    char *(*get_tape_current)(progfile_t *);

    void (*set_tape)(progfile_t *, char *);

    char (*peek_at)(progfile_t *, size_t);
};
