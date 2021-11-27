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

/** List of conversion types
 */
typedef enum conv_type {
    NO_CONVERSION,
    CONVERT_FIRST,
    CONVERT_SECOND,
    CONVERT_BOTH
} conv_type_t;

/** List of semantic states
 */
typedef enum semantic_state {
    SEMANTIC_IDLE,
    SEMANTIC_PARENTS,
    SEMANTIC_OPERAND,
    SEMANTIC_UNARY,
    SEMANTIC_BINARY,
    SEMANTIC_FUNCTION
} semantic_type_t;

/** Semantic information about an expression.
 */
typedef struct expr_semantics {
    semantic_type_t sem_state;
    token_t first_operand;
    token_t second_operand;
    op_list_t op;
    conv_type_t conv_type;
    int result_type;
    dynstring_t * func_types;
    dynstring_t * func_rets;
} expr_semantics_t;


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

    /** Expression semantics destructor.
     *
     * @param self expression semantics struct.
     */
//    void (*dtor_expr)(expr_semantics_t *);

    /** Expression semantics constructor.
     *
     * @return new expressions semantics.
     */
//    expr_semantics_t *(*ctor_expr)();

    /** Expression semantics add operand.
     *
     * @param self expression semantics struct.
     * @param tok operand.
     */
//    void (*add_operand)(expr_semantics_t *, token_t);

//    void (*add_operator)(expr_semantics_t *, op_list_t);

//    bool (*check_expression)(expr_semantics_t *);

    bool (*check_signatures_compatibility)(dynstring_t *, dynstring_t *, int);

    char (*of_id_type)(int);

    int (*token_to_id_type)(int);

    int (*keyword_to_id_type)(int);
};
