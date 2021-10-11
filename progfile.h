
#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "dynstring.h"


/**
 * Struct is hidden. definition is in progfile.c
 */
typedef struct c_progfile pfile_t;

extern const struct pfile_interface_i Pfile;

/**
 * Interface to use when dealing with file.
 */
struct pfile_interface_i {
    /**
     * @brief Store file to progfile structure.
     *
     * @param filename
     * @param mode File opening mode.
     * @return pfile where file is stored. If error_interface returns NULL.
     */
    pfile_t *(*getfile)(const char *, const char *);

    /**
     * @brief Reads a file from stdin to progfile structure.
     *
     * @return File stored in pfile structure. If error_interface return NULL.
     */
    pfile_t *(*getfile_stdin)(void);

    /**
     * @brief Free data structure.
     *
     * @param pfile struct that will be freed
     * @return void
     */
    void (*free)(pfile_t *);

    /**
     * @brief move tape head one character backward, and return a current character
     *
     * @param pfile
     * @return character before or EOF
     */
    int (*ungetc)(pfile_t *);

    /**
     * @brief Get actual character and move to next one.
     *
     * @param pfile
     * @return Actual character. If end or empty return EOF.
     */
    int (*pgetc)(pfile_t *);

    /**
     * @brief Get tape.
     *
     * @param pfile
     * @return pfile->tape
     */
    char *(*get_tape)(pfile_t *);

    /**
     * @brief Get current tape at the pos position.
     *
     * @param pfile
     * @return pfile->tape + pfile->pos
     */
    char *(*get_tape_current)(pfile_t *);

    /**
     * @brief Refresh pfile->tape to the new value.
     * NOTE: new @param tape must be the part of pfile->tape, e.g. it can be strchr(gettape(pfile), some_char).
     * Otherfise it won't work.
     *
     * @param pfile
     * @param tape
     * @return
     */
    void (*set_tape)(pfile_t *, char *);

    /**
     * @brief Move the head of the tape by step @param step. 0 means the next char will be returned, because previous pgetc()
     * has post-increased pos.
     *
     * @param pfile char array
     * @param step
     * @return pfile->tape[pos+step] if empty char* or out of bounds return EOF.
     */
    char (*peek_at)(pfile_t *, size_t);


    /**
     * @brief Functoion for creates files directly from strings. Fits for testing.
     *
     * @param tape tape represents the file.
     * @return pointer on pfile_t or NULL
     */
    pfile_t *(*ctor)(char *);
};

