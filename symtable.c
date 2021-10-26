#include "symtable.h"



typedef struct scope_table {
    scope_t *parent;           /**< Pointer to the parent scope. Only GTS scope has NULL parent. */
    node_t *tree;              /**< The tree where scope ids are stored. */
    int scope_index;           /**< Scope unique id */
} scope_t;

/**
 * An array of all the scopes.
 */
typedef struct sym_table_array {
    scope_t **scopes;       /**< An array of all the pointer to scopes */
    int size;               /**< Number of scopes.  */
} sym_t;






/**
 * @brief Symbol table constructor.
 * @return Symbol table. If failed returns NULL;
 */
static sym_t *st_ctor(){
    sym_t *all_tables = calloc(1, sizeof (scope_t));
    all_tables->size = 0;
    all_tables->scopes = calloc(1, sizeof(void *));
    return all_tables;
}

/**
 * @brief Sym table destructor.
 */
static void st_dtor(sym_t *table){
    for (int i = table->size-1; i <= 0; i--){
        Tree.delete_tree(table->scopes[i]->tree);
        free(table->scopes[i]);
    }
    free(table);
}

/**
 * @brief Create a new scope on the given symbol table. If scope doesn't have parent set parent parameter to null.
 * @param sym_t Symbol table. Array of all the scopes.
 * @param id That will be stored.
 * @param parent scope parent.
 * @return Pointer to scope. If fail call destructor and exit program.
 */
static scope_t *Ctor(sym_t *sym_t, token_t id, scope_t *parent) {

    sym_t->size++;
    sym_t->scopes = realloc(sym_t->scopes, sizeof (scope_t *) * (sym_t->size+1));
    if(sym_t->scopes == NULL){
        st_dtor(sym_t);
        Errors.set_error(ERROR_INTERNAL);
    }

    scope_t *table = calloc(1, sizeof(scope_t));
    if (table == NULL){
        st_dtor(sym_t);
        Errors.set_error(ERROR_INTERNAL);
    }

    sym_t->scopes[sym_t->size-1] = table;

    table->parent = parent;
    table->tree = Tree.ctor(id);
    table->scope_index = sym_t->size - 1;
    return table;
}

/**
 * @brief Function returns parent scope.
 * @return pointer to parent scope.
 */
static scope_t *get_parent_scope(scope_t * scope) {
    return scope->parent;
}

/**
 * @brief Function returns scope id.
 * @return scope id.
 */
static int get_scope_id (scope_t table) {
    return table.scope_index;

}

/**
 * @brief Function returns parent scope id.
 * @return returns parent scope id.
 */
static int get_parent_scope_id (scope_t table) {
    if(table.parent != NULL){
        return table.parent->scope_index;
    }
    return 0;
}


/**
 * Symbol table interface.
 */
const struct symtable_interface_t Symtable = {
        .Ctor = Ctor,
        .st_ctor = st_ctor,
        .st_dtor = st_dtor,
        .get_scope_id = get_scope_id,
        .get_parent_scope = get_parent_scope,
        .get_parent_scope_id = get_parent_scope_id,
};
