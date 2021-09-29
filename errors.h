/********************************************
 * Project name: IFJ - projekt
 * File: errors.h
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 *
 *
 *  @package errors
 *  @file errors.h
 *  @brief Contain enum of all the error codes.
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */
#pragma once

/**
 * All error codes.
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
// todo: shall we create functinos like get_error(), which returns an error code and prints a message?



/**
 * A structure that store pointers to all the functions from errors.c. So we can use them in different files.
 */
struct error {
    int (*return_error)(int);
};

extern const struct error Errors;

