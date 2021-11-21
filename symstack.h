/**
 * @file symstack.h
 *
 * @brief Symbol table stack header.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */

#pragma once

#include "symtable.h"
#include "scanner.h"
#include "debug.h"
#include "errors.h"
#include "dynstring.h"
#include "semantics.h"


// types of scopes
#define SCOPE_TYPE_T(X) \
    X(function)          \
    X(cycle)             \
    X(do_cycle)          \
    X(condition)         \
    X(global)            \
    X(UNDEF)

typedef enum scope_type {
    #define X(a) SCOPE_TYPE_##a ,
    SCOPE_TYPE_T(X)
    #undef X
} scope_type_t;


/** Information about the scope
 */
typedef struct scope_info {
    scope_type_t scope_type; ///< to know where we are.
    size_t scope_level; ///< nested level.
    size_t unique_id; ///< unique scope id is unique for each scope in the program.
} scope_info_t;


/** Symbol stack opaque structure.
 */
typedef struct symstack symstack_t;

/** Symbol stack element opaque structure.
 */
typedef struct stack_el stack_el_t;

/** Python-like interface.
 */
extern const struct symstack_interface_t Symstack;

struct symstack_interface_t {
    /** Initialise a symbol stack.
     *
     * @return Pointer to allocated memory.
     */
    symstack_t *(*init)();

    /** Push a new symbol table on the stack.
     *
     * @param self stack of symbol tables.
     * @param table symbol table to push.
     * @param scope_type function, do_cycle, cycle, condition etc.
     * @param fun_name if scope_type is not function, then NULL.
     */
    void (*push)(symstack_t *, symtable_t *, scope_type_t, char *fun_name);

    /** Pop a symtable from a stack.
     *
     * @param self symstack.
     */
    void (*pop)(symstack_t *);

    /** Delete the whole stack with all symtables on it.
     *
     * @param self stack to delete.
     */
    void (*dtor)(symstack_t *);

    /** Get a symbol from the symbol stack.
     *
     * @param self symbol stack.
     * @param id key.
     * @param sym a pointer to symbol to store a pointer to the object if we find it.
     * @return true if symbol exists.
     */
    bool (*get_symbol)(symstack_t *, dynstring_t *, symbol_t **, stack_el_t **);

    /** Get top table.
     *
     * @param self symbol stack.
     * @return symbol table.
     */
    symtable_t *(*top)(symstack_t *);

    /** Put a symbol on the stack.
     *
     * @param self symstack.
     * @param id symbol that will be copied on the stack.
     * @param type type of an identifier.
     * @return pointer on symbol puched on the stack.
     */
    symbol_t *(*put_symbol)(symstack_t *, dynstring_t *, id_type_t);

    /** Get a parent function name.
     *
     * @param self
     * @return NULl if there is a global frame.
     */
    char *(*get_parent_func_name)(symstack_t *);

    /** Return information about the current scope.
     *
     * @param self symbol stack.
     * @return scope_info_t structure.
     */
    scope_info_t (*get_scope_info)(symstack_t *);

    /** Traverse a symstack and apply a predicate on all the symbols.
     *
     * Function store the conjunction of all predicates.
     *
     * @param self symstack to traverse.
     * @param predicate predicate to apply.
     * @return
     */
    bool (*traverse)(symstack_t *, bool (*predicate)(symbol_t *));

    /** Traverse a symstack without looking at the global frame.
     *
     * @param self symbol stack.
     * @param id key.
     * @param sym a pointer to symbol to store a pointer to the object if we find it.
     * @return true if symbol exists.
     */
    bool (*get_local_symbol)(symstack_t *, dynstring_t *, symbol_t **, stack_el_t **);
};
