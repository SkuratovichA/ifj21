//
// Created by xskura01 on 23.09.2021.
//

#include "progfile.h"

#include "progfile.h"
#include "macros.h"
#include <assert.h>
#include "dynstring.h"

// opaque structure
struct c_progfile {
    size_t size;
    size_t pos;
    char tape[];
};

static char Peek_at(progfile_t *pfile, size_t step) {
    if (!pfile) {
        return EOF;
    }
    size_t newpos = pfile->pos + step;
    return (newpos < pfile->size) ? pfile->tape[newpos] : EOF;
}

static int Getc(progfile_t *pfile) {
    if (pfile != NULL && pfile->pos < pfile->size) {
        return pfile->tape[pfile->pos++];
    } else {
        pfile->pos++;
    }

    return EOF;;
}

static int Ungetc(progfile_t *pfile) {
    return (pfile != NULL && pfile->pos > 0) ? pfile->tape[--pfile->pos] : EOF;
}

static void Free(progfile_t *pfile) {
    free(pfile);
}

static char *Get_tape(progfile_t *pfile) {
    return pfile->tape;
}

static char *Get_tape_current(progfile_t *pfile) {
    return (pfile->tape + pfile->pos);
}

// tape must be the part of pfile->tape, otherwise insanity happens
static void Set_tape(progfile_t *pfile, char *tape) {
    if (pfile == NULL) {
        return;
    }
    size_t diff = tape - pfile->tape;
    if (diff > 0 && diff < pfile->size - 1) {
        pfile->pos = diff;
    }
}

// very advanced method to read a file from an stdin
// my face is ready for tomatoes to throw in
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


const struct progfile_op_struct_t Progfile = {
        .getfile  = Getfile,
        .getfile_stdin  = Getfile_stdin,
        .free = Free,
        .pgetc = Getc,
        .ungetc = Ungetc,
        .get_tape = Get_tape,
        .set_tape = Set_tape,
        .peek_at = Peek_at,
        .get_tape_current = Get_tape_current,
};
