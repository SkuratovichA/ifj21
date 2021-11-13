/**
 * @file symtable.h
 *
 * @brief Symbol table header.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Kuznik Jakub <xkuzni04@vutbr.cz>
 */
#pragma once

#include "scanner.h"
#include "debug.h"
#include "errors.h"
#include "dynstring.h"
#include "semantics.h"


/** Magic with aliasing
 */
#define KEYWORD_func_def (KEYWORD_UNDEF + 1)
#define KEYWORD_func_decl (KEYWORD_func_def + 1)

#define ID_TYPE_T(X)   \
    X(string)          \
    X(boolean)         \
    X(number)          \
    X(integer)         \
    X(func_def)        \
    X(func_decl)       \
    X(nil)             \
    X(UNDEF)

/** Types of symbol ids
 */
typedef enum id_type {
    #define X(a) ID_TYPE_##a = KEYWORD_##a,
    ID_TYPE_T(X)
    #undef X
} id_type_t;


/** Symbol is not opaque because fuck it.
 */
typedef struct symbol {
    id_type_t type;
    dynstring_t *id;
    func_semantics_t *function_semantics;
} symbol_t;


/** Opaque structure.
 */
typedef struct symtable symtable_t;


/** Python-like interface.
 */
extern const struct symtable_interface_t Symtable;

struct symtable_interface_t {
    /** Create a symbol table.
     *
     * @return pointer on initialized memory.
     */
    symtable_t *(*ctor)();

    /** Enum types casting function.
     *
     * @param token_type token from scanner.
     * @return id_type_t from a token_type.
     */
    id_type_t (*id_type_of_token_type)(int);

    /** Get a symbol from the symtable.
     *
     * @param self symtable.
     * @param id name to search the node by.
     * @param storage storage will contain a pointer to the symbol.
     * @return bool.
     */
    bool (*get_symbol)(symtable_t *, dynstring_t *, symbol_t **);

    /** Put a symbol into the symbol table.
     *
     * @param self BST.
     * @param id symbol name.
     * @param type symbol type.
     * @return pointer on the symbol in the binary tree. Newly created or already existed.
     */
    symbol_t *(*put)(symtable_t *, dynstring_t *, id_type_t);

    /** Symbol table destructor.
     *
     * @param self symbol table to free memory.
     */
    void (*dtor)(symtable_t *);

    /** Function to add builtin functions on the symbol table.
     *
     * 's' for string
     * 'b' for boolean
     * 'i' for integer
     * 'f' for number
     * 'n' for nil
     *
     * @param self symbol table.
     * @param name name of the function.
     * @param params vector of the parameters of the function.
     * @param returns vector with return values of the function.
     */
    void (*add_builtin_function)(symtable_t *, char *, char *, char *);


    /** Traverse a symtable and apply a predicate on all the symbols.
     *
     * Function store the conjunction of all predicates.
     *
     * @param self symtable to traverse.
     * @param predicate predicate to apply.
     * @return
     */
    bool (*traverse)(symtable_t *, bool (*predicate)(symbol_t *));
};
