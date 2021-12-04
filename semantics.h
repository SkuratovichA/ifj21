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
#include "scanner.h"
#include "expressions.h"


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

typedef enum type_recast {
    TYPE_RECAST_FIRST,
    TYPE_RECAST_SECOND,
    TYPE_RECAST_BOTH,
    NO_RECAST
} type_recast_t;

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

    bool (*check_signatures_compatibility)(dynstring_t *, dynstring_t *, int);

    char (*of_id_type)(int);

    int (*token_to_id_type)(int);

    /**
     * @brief Check semantics of binary operation.
     *
     * @param first_type type of the first operand.
     * @param second_type type of the second operand.
     * @param op binary operator.
     * @param expression_type initialized vector to store an expression type.
     * @param r_type variable to store a type of recast.
     * @return bool.
     */
    bool (*check_binary_compatibility)(dynstring_t *, dynstring_t *, op_list_t, dynstring_t *, type_recast_t*);

    /**
     * @brief Check semantics of unary operation.
     *
     * @param type type of the operand.
     * @param op unary operator.
     * @param expression_type initialized vector to store an expression type.
     * @return bool.
     */
    bool (*check_unary_compatibility)(dynstring_t *, op_list_t, dynstring_t *);

    /**
     * @brief Check semantics of single operand.
     *
     * @param operand
     * @param expression_type initialized vector to store an expression type.
     * @return bool.
     */
    bool (*check_operand)(token_t, dynstring_t *);

    /**
     * @brief Truncate signature to one type.
     *
     * @param type_signature is an initialized vector with expression type/s.
     */
    void (*trunc_signature)(dynstring_t *);

    /**
     * @brief Check semantics of two types.
     *
     * @param expected_type
     * @param received_char
     * @param r_type variable to store a type of recast.
     * @return bool.
     */
    bool (*check_type_compatibility)(const char, const char, type_recast_t *);
};
