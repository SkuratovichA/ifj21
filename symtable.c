#include "symtable.h"
#include "tests/tests.h"
#include "errors.h"




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
    if(table == NULL){
        return;
    }
    for (int i = table->size-1; i <= 0; i--){
        Tree.delete_tree(table->scopes[i]->tree);
        free(table->scopes[i]);
    }
    free(table);
}

/**
 * @brief the function finds id in given scope and return true if id was found.
 * @param scope active scope.
 * @param id Id we are looking for.
 * @return true if id was found,
 */
static bool find_id_in_scope(scope_t *scope, dynstring_t *id){
    node_t *temp = Tree.find(scope->tree, id);

    if(temp == NULL){
        return false;
    } else{
        return true;
    }
}

/**
 * @brief the function finds id in scope and its parents.
 * @param scope active scope.
 * @param id Id we are looking for.
 * @return true if id was found,
 */
static bool find_id(scope_t* active, dynstring_t *id) {
    if (find_id_in_scope(active, id) == true) {
        return true;
    }

    scope_t *temp_scope = active->parent;
    node_t *temp_tree = NULL;

    // Go to parent try to find id, till you reach main scope.
    while(temp_scope != NULL){
        temp_tree = Tree.find(temp_scope->tree, id);
        if(temp_tree != NULL)
            return true;
        temp_scope = temp_scope->parent;
    }
    return false;
}

/**
 * @brief Store token id to given scope.
 * @param scope where we want to store.
 * @param id id that we want to store.
 * @param type id type. Could be function, if statement, int, number, string ...
 * @return 0 If success. -1 If there is id with same name return. -2 If allocation error return and exit the program.
 */
static int store_id(scope_t *scope, dynstring_t *id, int type){
    Tree.insert(scope->tree, id, type);
    return 0;
}

/**
 * @brief Create a new scope on the given symbol table. If scope doesn't have parent set parent parameter to null.
 * @param sym_t Symbol table. Array of all the scopes.
 * @param id That will be stored.
 * @param type id type. Could be function, if statement, int, number, string ...
 * @param parent scope parent. IF MAIN SCOPE ENTER NULL!!!
 * @return Pointer to scope. If fail call destructor and exit program.
 */
static scope_t *Ctor(sym_t *sym_t, dynstring_t *id, int type, scope_t *parent) {

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
    table->tree = Tree.ctor(id, type);
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
static int get_scope_id (scope_t * table) {
    return table->scope_index;

}

/**
 * @brief Function returns parent scope id.
 * @return returns parent scope id.
 */
static int get_parent_scope_id (scope_t * table) {
    if(table->parent != NULL){
        return table->parent->scope_index;
    }
    return 0;
}


/**
 * Symbol table interface.
 */
const struct symtable_interface_t Symt = {
        .Ctor                   = Ctor,
        .st_ctor                = st_ctor,
        .st_dtor                = st_dtor,
        .get_scope_id           = get_scope_id,
        .get_parent_scope       = get_parent_scope,
        .get_parent_scope_id    = get_parent_scope_id,
        .find_id_in_scope       = find_id_in_scope,
        .find_id                = find_id,
        .store_id               = store_id,

};

#ifdef SELFTEST_symtable

int main() {
    debug_msg("symtable selfdebug\n");
    token_t tok = { .type = TOKEN_STR, .attribute.id = Dynstring.ctor("\"Test name\"") };
    Symtable.ctor(NULL, tok);

    return 0;
}

#endif
