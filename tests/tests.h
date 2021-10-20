
#pragma once
#include "../errors.h"

#define RESET      "\x1b[0m"
#define RED(s)     "\x1b[31m"s RESET
#define GREEN(s)   "\x1b[32m"s RESET
#define YELLOW(s)  "\x1b[33m"s RESET
#define BLUE(s)    "\x1b[34m"s RESET
#define MAGENTA(s) "\x1b[35m"s RESET
#define CYAN(s)    "\x1b[36m"s RESET

extern const struct c_tests_t Tests;


#define TEST_EXPECT(expected, cond, msg) \
do {                                     \
    if ((expected) != (cond)) { \
        Tests.failed("%s\n", msg); \
    } else \
        Tests.passed("%s\n", msg); \
    fprintf(stdout, "%s\n", Errors.get_errmsg()); \
} while(0)

/**
 * Interface to use when dealing with strings.
 */
struct c_tests_t {
    void (*failed)(const char *, ...);

    void (*warning)(const char *, ...);

    void (*passed)(const char *, ...);
};
