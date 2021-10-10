#pragma once

// assert a condition and exit with an error_interface code if condition is false
#define soft_assert(cond, err) \
do { \
    if (!(cond)) { \
        debug_msg_s("\n "); \
        debug_msg(" "); \
        fprintf(stderr, "(soft)assertion failed: "); \
        fprintf(stderr, #cond); \
        fprintf(stderr, "\n"); \
        exit((err)); \
    } \
} while (0)

/**
 * All error_interface codes.
 */
enum errors {
    ERROR_LEXICAL = 1,
    ERROR_SYNTAX,
    ERROR_UNDEF_FUN_OR_VAR,
    ERROR_TYPE_MISSMATCH,
    ERROR_SEMANTICS_NUMBER_PARAMETERS,
    ERROR_SEMANTICS_TYPE_INCOMPATABLE,
    ERROR_SEMANTICS_OTHER,
    ERROR_RUNTIME_NIL,
    ERROR_RUNTIME_DIV_BY_ZERO,
    ERROR_INTERNAL = 99
};


/**
 * A structure that store pointers to all the functions from errors.c. So we can use them in different files.
 */
struct error_interface {
    void (*set_error)(int);

    int (*get_error)(void);
};

extern const struct error_interface Errors;

