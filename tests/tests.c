#include "tests.h"
#include <stdarg.h>
#include <stdio.h>

static void Passed(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[" GREEN("PASSED") "] "); \
      vfprintf(stdout, fmt, args);
    va_end(args);
}

static void Warning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "["  YELLOW("WARNING") "] "); \
      vfprintf(stdout, fmt, args);
    va_end(args);
}

static void Failed(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "["  RED("FAILED") "] "); \
      vfprintf(stdout, fmt, args);
    va_end(args);
}

/**
 * Tests interface.
 */
const struct c_tests_t Tests = {
        .failed = Failed,
        .passed = Passed,
        .warning = Warning
};
