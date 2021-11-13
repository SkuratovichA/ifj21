/**
 * @file errors.h
 *
 * @brief
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#pragma once

// assert a condition and exit with an error code if condition is false
#define soft_assert(cond, err) \
do { \
    if (!(cond)) { \
        debug_msg_s("\n "); \
        debug_msg(" "); \
        fprintf(stderr, __FILE__":%d in %s (soft)assertion failed ", __LINE__, __FUNCTION__); \
        fprintf(stderr, #cond); \
        fprintf(stderr, "\n"); \
        exit((err)); \
    } \
} while (0)

/** All error_interface codes.
 */
enum errors {
    ERROR_NOERROR,
    ERROR_LEXICAL,
    ERROR_SYNTAX,
    ERROR_DEFINITION,
    ERROR_TYPE_MISSMATCH,
    ERROR_FUNCTION_SEMANTICS,
    ERROR_SEMANTICS_TYPE_INCOMPATABLE,
    ERROR_SEMANTICS_OTHER,
    ERROR_RUNTIME_NIL,
    ERROR_RUNTIME_DIV_BY_ZERO,
    ERROR_INTERNAL = 99
};


struct error_interface {
    void (*set_error)(int);

    char *(*get_errmsg)(void);

    int (*get_error)(void);
};

extern const struct error_interface Errors;

