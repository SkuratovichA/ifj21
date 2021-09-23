//
// Created by xskura01 on 23.09.2021.
//

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "dynstring.h"


typedef struct c_progfile progfile_t;

extern const struct progfile_op_struct_t Progfile;

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
