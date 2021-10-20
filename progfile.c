#include "progfile.h"

#include "progfile.h"
#include "macros.h"
#include <assert.h>
#include "dynstring.h"


// opaque structure
/**
 * An opaque structure representing a file.
 */
struct c_progfile {
    size_t size; // File size
    size_t pos;  // position in file
    char tape[];
};

/**
 * @brief Pfile constructor. Create a pfile object using memcpy from a char *. FIts for tests.
 *
 * @param pfile char array which become a tape.
 * @return new pfile object.
 */
static pfile_t *Ctor(char *tape) {
    pfile_t *pfile;
    if (!tape) {
        goto err0;
    }
    size_t len = strlen(tape);

    pfile = calloc(1, len + 1 + sizeof(pfile_t));
    if (!pfile) {
        goto err0;
    }

    memcpy(pfile->tape, tape, len + 1);
    pfile->size = len;

    return pfile;
    err0:
    Errors.set_error(ERROR_INTERNAL);
    return NULL;
}

/**
 * @brief Move the head of the tape by step @param step. 0 means the next char will be returned, because previous pgetc()
 * has post-increased pos.
 *
 * @param pfile char array
 * @param step
 * @return pfile->tape[pos+step] if empty char* or out of bounds return EOF.
 */
static char Peek_at(pfile_t *pfile, size_t step) {  //MAYBE RENAME TO MOVE_FILE_POINTER
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
static int Getc(pfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    if (pfile->pos < pfile->size) {
        return pfile->tape[pfile->pos++];
    } else {
        pfile->pos++;
    }

    return EOF;;
}

/**
 * @brief move tape head one character backward, and return a current character
 *
 * @param pfile
 * @return character before or EOF
 */
static int Ungetc(pfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    return (pfile->pos > 0) ? pfile->tape[--pfile->pos] : EOF;
}

/**
 * @brief Free data structure.
 *
 * @param pfile struct that will be freed
 * @return void
 */
static void Free(pfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    pfile->size = 0;
    pfile->pos = 0;
    // pfile->tape = 0; // not sure about it
    free(pfile);
}

/**
 * @brief Get tape.
 *
 * @param pfile
 * @return pfile->tape
 */
static char *Get_tape(pfile_t *pfile) {
    soft_assert(pfile != NULL, ERROR_INTERNAL);
    return pfile->tape;
}

/**
 * @brief Get current tape at the pos position.
 *
 * @param pfile
 * @return pfile->tape + pfile->pos
 */
static char *Get_tape_current(pfile_t *pfile) {
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
static void Set_tape(pfile_t *pfile, char *tape) {
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
static pfile_t *Getfile_stdin() {
    pfile_t *pfile;

    int ch;
    size_t allocated, alloc_step = allocated = 128;

    pfile = calloc(1, sizeof(pfile_t) + alloc_step);
    if (!pfile) {
        goto err0;
    }

    while (EOF != (ch = fgetc(stdin))) {
        if (pfile->size + 2 >= allocated) {
            pfile_t *r = realloc(pfile, sizeof(pfile_t) + (allocated += alloc_step));
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
    err0:
    Errors.set_error(ERROR_INTERNAL);
    return NULL;
}

/**
 * @brief Store file to progfile structure.
 *
 * @param filename
 * @param mode File opening mode.
 * @return pfile where file is stored. If error_interface returns NULL.
 */
static pfile_t *Getfile(const char *filename, const char *mode) {
    pfile_t *pfile;
    FILE *fp = fopen(filename, mode);
    size_t size;

    if (fp == NULL) {
        goto err0;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET); // return a pointer back to the beginning of the file

    if (NULL == (pfile = calloc(1, size + sizeof(pfile_t) + 1))) { // + 1 is for '\0' for "EOF"
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
    err0:
    Errors.set_error(ERROR_INTERNAL);
    return NULL;
}


/**
 * Pfile interface.
 */
const struct pfile_interface_t Pfile = {
        .getfile = Getfile,
        .getfile_stdin  = Getfile_stdin,
        .dtor = Free,
        .pgetc = Getc,
        .ungetc = Ungetc,
        .get_tape = Get_tape,
        .set_tape = Set_tape,
        .peek_at = Peek_at,
        .get_tape_current = Get_tape_current,
        .ctor = Ctor,
};
