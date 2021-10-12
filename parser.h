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


extern const struct parser_interface_t Parser;

struct parser_interface_i {
    bool (*analyse)(pfile_t *);
};
