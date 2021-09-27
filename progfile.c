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
 *  @brief //TODO WHAT IS THIS FILE FOR WHERE ARE THESE FUNCTIONS USED
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */


#include "progfile.h"

#include "progfile.h"
#include "macros.h"
#include <assert.h>
#include "dynstring.h"

// opaque structure
struct c_progfile {
    size_t size; // File size
    size_t pos;  // position in file
    char tape[];
};

/**
 * @brief Move char pointer (*pfile->tape) by step.
 *
 * @param pfile char array
 * @param step
 * @return pfile->tape[pos+step] if empty char* or out of bounds return EOF.
 */
static char Peek_at(progfile_t *pfile, size_t step) {  //MAYBE RENAME TO MOVE_FILE_POINTER
    if (!pfile) {
        return EOF;
    }
    size_t newpos = pfile->pos + step;

    if (newpos < pfile->size)
        return pfile->tape[newpos];
    else
        return EOF;

    //return (newpos < pfile->size) ? pfile->tape[newpos] : EOF;
}

/**
 * @brief Get actual character and move to next one.
 *
 * @param pfile
 * @return Actual character. If end or empty return EOF.
 */
static int Getc(progfile_t *pfile) {
    if (pfile != NULL && pfile->pos < pfile->size) {
        return pfile->tape[pfile->pos++];
    } else {
        pfile->pos++;
    }

    return EOF;;
}

/**
 * @brief Get to character before. Tape must be the part of pfile->tape, otherwise insanity happens.
 *
 * @param pfile
 * @return character before or EOF
 */
static int Ungetc(progfile_t *pfile) {
    return (pfile != NULL && pfile->pos > 0) ? pfile->tape[--pfile->pos] : EOF;
}

/**
 * @brief Free data structure.
 *
 * @param pfile struct that will be freed
 * @return void
 */
static void Free(progfile_t *pfile) {
    free(pfile);
}

/**
 * @brief Returns pointer to start.
 *
 * @param pfile
 * @return pfile->tape
 */
static char *Get_tape(progfile_t *pfile) {
    return pfile->tape;
}

/**
 * @brief Returns pointer to actual position.
 *
 * @param pfile
 * @return pfile->tape + pfile->pos
 */
static char *Get_tape_current(progfile_t *pfile) {
    return (pfile->tape + pfile->pos);
}

/**
 * @brief Set pfile to tape. Tape must be the part of pfile->tape, otherwise insanity happens.
 *
 * @param pfile
 * @param tape where to move
 * @return new pfile->pos on tape pos.
 *
 */
static void Set_tape(progfile_t *pfile, char *tape) {
    if (pfile == NULL) {
        return;
    }
    size_t diff = tape - pfile->tape;
    if (diff > 0 && diff < pfile->size - 1) {
        pfile->pos = diff;
    }
}

/**
 * @brief Reads a file from stdin.
 *
 * @return File stored in pfile structur. If error return NULL.
 */
static progfile_t *Getfile_stdin() {
    progfile_t *pfile;
    size_t size;

    string filetape;
    if (!Dynstring.create_onheap(&filetape)) {
        return false;
    }

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
    return pfile;
    err:
    free(pfile);
    return NULL;
}

/**
 * @brief Store file to pfile.
 *
 * @param filename
 * @param mode File opening mode.
 * @return pfile where file is stored. If error returns NULL.
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
    return pfile;
    err2:
    free(pfile);
    err1:
    fclose(fp);
    return NULL;
}


/**
 * Functions are in struct so we can use them in different files.
 */
const struct progfile_op_struct_t Progfile = {
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
