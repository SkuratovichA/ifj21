//
// Created by xskua01 on 23.09.2021.
//


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
// todo: shall we create functinos like get_error(), which returns an error code and prints a message?

struct error {
    int (*return_error)(int);
};

extern const struct error Errors;

