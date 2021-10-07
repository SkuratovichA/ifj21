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
 *  @package progfile
 *  @file progfile.c
 *  @brief Program file - buffered binary file representation.
 *
 *
 *  @author Aliaksandr Skuratovich
 */

#include "progfile.h"

#include "progfile.h"
#include "macros.h"
#include <assert.h>
#include "dynstring.h"



// opaque structure
struct c_progfile {
    bool allocated;
    size_t size; // File size
    size_t pos;  // position in file
    char tape[];
};

/**
 * @brief Move the head of the tape by step @param step. 0 means the next char will be returned, because previous pgetc()
 * has post-increased pos.
 *
 * @param pfile char array
 * @param step
 * @return pfile->tape[pos+step] if empty char* or out of bounds return EOF.
 */
static char Peek_at(progfile_t *pfile, size_t step) {  //MAYBE RENAME TO MOVE_FILE_POINTER
    soft_assert(pfile != NULL, ERROR_INTERNAL);

    size_t newpos = pfile->pos + step;

    if (newpos < pfile->size) {
        return pfile->tape[newpos];
    } else {
        return EOF;
    }

    //return (newpos < pfile->size) ? pfile->tape[newpos] : EOF;
}

/**
 * @brief Get actual character and move to next one.
 *
 * @param pfile
 * @return Actual character. If end or empty return EOF.
 */
static int Getc(progfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    if (pfile->pos < pfile->size) {
        return pfile->tape[pfile->pos++];
    } else {
        pfile->pos++;
    }

    return EOF;;
}

/**
 * @brief Get to character before.
 *
 * @param pfile
 * @return character before or EOF
 */
static int Ungetc(progfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    return (pfile->pos > 0) ? pfile->tape[--pfile->pos] : EOF;
}

/**
 * @brief Free data structure.
 *
 * @param pfile struct that will be freed
 * @return void
 */
static void Free(progfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    pfile->size = 0;
    pfile->pos = 0;
    // pfile->tape = 0; // not sure about it
    if (pfile->allocated) {
        free(pfile);
        pfile->allocated = false;
    }
}

/**
 * @brief Get tape.
 *
 * @param pfile
 * @return pfile->tape
 */
static char *Get_tape(progfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    return pfile->tape;
}

/**
 * @brief Get current tape at the pos position.
 *
 * @param pfile
 * @return pfile->tape + pfile->pos
 */
static char *Get_tape_current(progfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    return (pfile->tape + pfile->pos);
}

/**
 * @brief Refresh pfile->tape to the new value.
 * NOTE: new @param tape must be the part of pfile->tape, e.g. it can be strchr(gettape(pfile), some_char).
 * Otherfise it won't work.
 *
 * @param pfile
 * @param tape
 * @return
 *
 */
static void Set_tape(progfile_t *pfile, char *tape) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);

    size_t diff = tape - pfile->tape;
    if (diff > 0 && diff < pfile->size - 1) {
        pfile->pos = diff;
    }
}

/**
 * @brief Reads a file from stdin to progfile structure.
 *
 * @return File stored in pfile structure. If error_interface return NULL.
 */
static progfile_t *Getfile_stdin() {
    progfile_t *pfile;

    int ch;
    size_t allocated, alloc_step = allocated = 128;

    pfile = calloc(1, sizeof(progfile_t) + alloc_step);
    if (!pfile) {
        return NULL;
    }

    while (EOF != (ch = fgetc(stdin))) {
        if (pfile->size + 2 >= allocated) {
            progfile_t *r = realloc(pfile, sizeof(progfile_t) + (allocated += alloc_step));
            if (!r) {
                goto err;
            }
            pfile = r;
        }
        pfile->tape[pfile->size++] = (char) ch;
        pfile->tape[pfile->size] = 0;
    }

    pfile->allocated = false;
    return pfile;
    err:
    free(pfile);
    return NULL;
}

/**
 * @brief Store file to progfile structure.
 *
 * @param filename
 * @param mode File opening mode.
 * @return pfile where file is stored. If error_interface returns NULL.
 */
static progfile_t *Getfile(const char *filename, const char *mode) {
    progfile_t *pfile;
    FILE *fp = fopen(filename, mode);
    size_t size;

    if (fp == NULL) {
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET); // return a pointer back to the beginning of the file

    if (NULL == (pfile = calloc(1, size + sizeof(progfile_t) + 1))) { // + 1 is for '\0' for "EOF"
        goto err1;
    }

    pfile->size = size;
    size_t rb;

    if (size != (rb = fread(pfile->tape, 1, size, fp))) {
        goto err2;
    }


    fclose(fp);
    pfile->allocated = true;
    return pfile;
    err2:
    free(pfile);
    err1:
    fclose(fp);
    return NULL;
}


/**
 * Progfile interface.
 */
const struct progfile_interface Progfile = {
        .getfile = Getfile,
        .getfile_stdin  = Getfile_stdin,
        .free = Free,
        .pgetc = Getc,
        .ungetc = Ungetc,
        .get_tape = Get_tape,
        .set_tape = Set_tape,
        .peek_at = Peek_at,
        .get_tape_current = Get_tape_current,
};
