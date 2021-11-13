/**
 * @file semantics.c
 *
 * @brief Implementation of different semantic controls.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */
#pragma once

#include "list.h"
#include <stdbool.h>
#include "debug.h"
#include "dynstring.h"


/** Information about a function datatypes.
 */
typedef struct func_info {
    dynstring_t *returns; ///< vector with enum(int) values representing types of return values.
    dynstring_t *params; ///< vector wish enum(int) values representing types of function arguments.
} func_info_t;

/** Semantic information about a function.
 */
typedef struct func_semantics {
    func_info_t declaration; ///< function info from the function declaration.
    func_info_t definition; ///< function info from the function definition.
    bool is_declared;
    bool is_defined;
    bool is_builtin;
} func_semantics_t;


//typedef struct func_semantics func_semantics_t;
//typedef struct func_info func_info_t;

/** Python-like interface
 */
extern const struct semantics_interface_t Semantics;

struct semantics_interface_t {
    /** Function checks if return values and parameters
     *  of the function are equal.
     *
     * @param func function definition or declaration semantics.
     * @return bool.
     */
    bool (*check_signatures)(func_semantics_t *);

    /** A predicate.
     *
     * @param self an atom.
     * @return the truth.
     */
    bool (*is_declared)(func_semantics_t *);

    /** A predicate.
     *
     * @param self an atom.
     * @return the truth.
     */
    bool (*is_defined)(func_semantics_t *);

    /** A predicate.
     *
     * @param self an atom.
     * @return the truth.
     */
    bool (*is_builtin)(func_semantics_t *);


    /** Set is_declared.
     *
     * TODO: depricate?
     *
     * @param self semantics to change it_declared flag.
     * @return void.
     */
    void (*declare)(func_semantics_t *);

    /** Set is_defined.
     *
     * TODO: depricate?
     *
     * @param self semantics to change it_defined flag.
     * @return void.
     */
    void (*define)(func_semantics_t *);

    /** Set is_builtin.
     *
     * TODO: depricate?
     *
     * @param self semantics to change it_builtin flag.
     * @return void.
     */
    void (*builtin)(func_semantics_t *);

    /** Add a return type to a function semantics.
     *
     * @param self info to add a param.
     * @param type param to add.
     */
    void (*add_return)(func_info_t *, int);

    /** Add a function parameter to a function semantics.
     *
     * @param self info to add a param.
     * @param type param to add.
     */
    void (*add_param)(func_info_t *, int);

    /** Add a function parameter to a function semantics.
     *
     * @param self info to set a vector.
     * @param vec vector to add.
     */
    void (*set_returns)(func_info_t *, dynstring_t *);

    /** Directly set a vector with function parameters.
     *
     * @param self info to set a vector.
     * @param vec vector to add.
     */
    void (*set_params)(func_info_t *, dynstring_t *);

    /** Function semantics destructor.
     *
     * @param self a victim.
     */
    void (*dtor)(func_semantics_t *);

    /** Function semantics constructor.
     *
     * @param is_defined flag to set.
     * @param is_declared flag to set.
     * @param is_builtin flag to set.
     * @return new function semantics.
     */
    func_semantics_t *(*ctor)(bool, bool, bool);
};
