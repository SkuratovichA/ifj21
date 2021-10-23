#include "tests.h"
#include <stdarg.h>
#include <stdio.h>

static FILE *out;

static void Set_out(FILE *nout) {
    out = nout;
}

static void Passed(const char *fmt, ...) {
    if (!out) {
        Set_out(stdout);
    }
    va_list args;
    va_start(args, fmt);
    fprintf(out, "[" GREEN("PASSED") "] ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

static void Warning(const char *fmt, ...) {
    if (!out) {
        Set_out(stdout);
    }
    va_list args;
    va_start(args, fmt);
    fprintf(out, "["  YELLOW("WARNING") "] ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

static void Failed(const char *fmt, ...) {
    if (!out) {
        Set_out(stdout);
    }
    va_list args;
    va_start(args, fmt);
    fprintf(out, "["  RED("FAILED") "] ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

/**
 * Tests interface.
 */
const struct c_tests_t Tests = {
        .failed = Failed,
        .passed = Passed,
        .warning = Warning,
        .set_out = Set_out,

};
