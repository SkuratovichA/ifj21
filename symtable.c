/**
 * @file symtable.c
 *
 * @brief The file contains implementation of the symbol table using a BST.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Kuznik Jakub <xkuzni04@vutbr.cz>
 *
 * TODO: move global_table, local_table here. From parser.c
 */

#include "symtable.h"
#include "tests/tests.h"
#include "errors.h"


/** A node of a symbol table which is a binary search tree.
 */
typedef struct node {
    symbol_t symbol; ///< symbol structure with information about the variable.
    struct node *left, *right; ///< children.
} node_t;

/** Symbol table shell to represent program scopes.
 */
typedef struct symtable {
    node_t *root; ///< root the the binary tree.
} symtable_t;


/** Enum types casting function.
 *
 * @param token_type token from scanner.
 * @return id_type_t from a token_type.
 */
static id_type_t id_type_of_token_type(int token_type) {
    switch (token_type) {
        case KEYWORD_string:
            return ID_TYPE_string;
        case KEYWORD_boolean:
            return ID_TYPE_boolean;
        case KEYWORD_number:
            return ID_TYPE_number;
        case KEYWORD_integer:
            return ID_TYPE_integer;
        case KEYWORD_nil:
            return ID_TYPE_nil;
        default:
            return ID_TYPE_UNDEF;
    }
}

/** Pretty print function used for debugging purposes.
 *
 * @param type id_type
 * @return string representation.
 */
static char *type_to_str(id_type_t type) {
    switch (type) {
        #define X(t) case ID_TYPE_##t: return #t;
        ID_TYPE_T(X)
        #undef X
        default:
            return "undefined";
    }
}

/** Create a symbol table.
 *
 * @return pointer on initialized memory.
 */
static symtable_t *ST_Ctor() {
    symtable_t *table = calloc(1, sizeof(symtable_t));
    soft_assert(table, ERROR_INTERNAL);
    debug_msg("[create] symtable.\n");
    return table;
}

/** Helping function to find a symbol in the symtable.
 *  Using binary search tree as a data structure.
 *
 * @param node current node.
 * @param id name to search the node by.
 * @param storage will contain a pointer on the symbol in the binary tree.
 * @return true/false.
 */
static bool _st_get(node_t *node, dynstring_t *id, symbol_t **storage) {
    if (node == NULL) {
        return false;
    }

    int res = Dynstring.cmp(id, node->symbol.id);
    if (res == 0) {
        if (storage != NULL) {
            *storage = &node->symbol;
        }
        return true;
    }
    return _st_get(res > 0 ? node->right : node->left, id, storage);
}

/** Get a symbol from the symtable.
 *
 * @param self symtable.
 * @param id name to search the node by.
 * @param storage storage will contain a pointer to the symbol.
 * @return bool.
 */
static bool ST_Get(symtable_t *self, dynstring_t *id, symbol_t **storage) {
    if (self == NULL) {
        return false;
    }

    bool found = _st_get(self->root, id, storage);

    debug_msg("[symtab_get] %s\n", found ? "found" : "not found");
    return found;
}

/** Check if name is builtin function.
 *
 * @param name str to check.
 * @return bool.
 */
static bool builtin_name(dynstring_t *name) {
    #define BUILTINS 10
    static const char *builtins[BUILTINS] = {
            "readi", "readn", "reads", "tointeger",
            "substr", "ord", "chr", "write", NULL
    };

    for (int i = 0; builtins[i] != NULL; i++) {
        if (strcmp(Dynstring.c_str(name), builtins[i]) == 0) {
            return true;
        }
    }
    #undef BUILTINS

    return false;
}

/** Put a symbol into the symbol table.
 *
 * @param self BST.
 * @param id symbol name.
 * @param type symbol type.
 * @return pointer on the symbol in the binary tree. Newly created or already existed.
 */
static symbol_t *ST_Put(symtable_t *self, dynstring_t *id, id_type_t type) {
    if (self == NULL) {
        return NULL;
    }

    node_t **iterator = &self->root;
    int res;

    while ((*iterator) != NULL) {
        res = Dynstring.cmp(id, (*iterator)->symbol.id);
        if (res == 0) {
            if (type == ID_TYPE_func_decl) {
                Semantics.declare((*iterator)->symbol.function_semantics);
                debug_msg("function declared '%s'\n", Dynstring.c_str(id));
            } else if (type == ID_TYPE_func_def) {
                Semantics.define((*iterator)->symbol.function_semantics);
                debug_msg("function defined '%s'\n", Dynstring.c_str(id));
            } else {
                debug_msg("{%s, %d} is already in the table\n", Dynstring.c_str(id), type);
            }
            return &(*iterator)->symbol;
        }
        iterator = res > 0 ? &(*iterator)->right : &(*iterator)->left;
    }

    // base case with no root
    (*iterator) = calloc(1, sizeof(node_t));
    soft_assert((*iterator) != NULL, ERROR_INTERNAL);

    (*iterator)->symbol.id = Dynstring.ctor(Dynstring.c_str(id));
    (*iterator)->symbol.type = type;

    if (type == ID_TYPE_func_decl || type == ID_TYPE_func_def) {
        (*iterator)->symbol.function_semantics =
                Semantics.ctor(type == ID_TYPE_func_def, type == ID_TYPE_func_decl, builtin_name(id));
    }

    debug_msg_s("\t[put] new { .id = '%s', .type = '%s' }.\n",
                Dynstring.c_str((*iterator)->symbol.id), type_to_str((*iterator)->symbol.type)
    );

    return &(*iterator)->symbol;
}

/** Recursive helping function to destroy all nodes in the tree.
 *
 * @param node node to destroy(with 2 children).
 */
static void _st_dtor(node_t *node) {
    if (node == NULL) {
        return;
    }
    if (node->symbol.type == ID_TYPE_func_decl ||
                                               node->symbol.type == ID_TYPE_func_def
            ) {
        Semantics.dtor(node->symbol.function_semantics);
    }
    Dynstring.dtor(node->symbol.id);

    _st_dtor(node->left);
    _st_dtor(node->right);
    free(node);
}

/** Symbol table destructor.
 *
 * @param self symbol table to free memory.
 */
static void ST_Dtor(symtable_t *self) {
    if (self == NULL) {
        return;
    }
    _st_dtor(self->root);
    free(self);
    debug_msg("[dtor] Symtable deleted\n");
}

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
static void Add_builtin_function(symtable_t *self, char *name, char *params, char *returns) {
    if ((bool) self && (bool) name == 0) {
        debug_msg("\tnull passed into a function...\n");
        return;
    }
    dynstring_t *dname = Dynstring.ctor(name);
    dynstring_t *defparamvec = Dynstring.ctor(params);
    dynstring_t *defreturnvec = Dynstring.ctor(returns);

    dynstring_t *declparamvec = Dynstring.ctor(params);
    dynstring_t *declreturnvec = Dynstring.ctor(returns);


    ST_Put(self, dname, ID_TYPE_func_decl);
    ST_Put(self, dname, ID_TYPE_func_def);

    symbol_t *symbol;
    ST_Get(self, dname, &symbol);
    soft_assert(symbol != NULL, ERROR_INTERNAL);

    Semantics.set_params(&symbol->function_semantics->definition, defparamvec);
    Semantics.set_returns(&symbol->function_semantics->definition, defreturnvec);

    Semantics.set_params(&symbol->function_semantics->declaration, declparamvec);
    Semantics.set_returns(&symbol->function_semantics->declaration, declreturnvec);


    Dynstring.dtor(dname);
    //debug_msg("\t[BUILTIN]: builtin function is set.\n");
}

/** auxilary function to traverse a symtable(BST)
 *
 * Function returns the conjunction of @param acc and its application on the children.
 *
 * @param self symtable to traverse.
 * @param predicate predicate to apply.
 * @param acc accumulator.
 * @return
 */
static bool _traverse(node_t *self, bool (*predicate)(symbol_t *), bool acc) {
    if (self == NULL) {
        return true;
    }

    acc &= predicate(&self->symbol);
    acc &= _traverse(self->left, predicate, acc);
    acc &= _traverse(self->right, predicate, acc);
    return acc;
}

/** Traverse a symtable and apply a predicate on all the symbols.
 *
 * Function store the conjunction of all predicates.
 *
 * @param self symtable to traverse.
 * @param predicate predicate to apply.
 * @return
 */
static bool Traverse(symtable_t *self, bool (*predicate)(symbol_t *)) {
    if (self && predicate == NULL) {
        return false;
    }
    bool acc = true;
    return _traverse(self->root, predicate, acc);
}


const struct symtable_interface_t Symtable = {
        .get_symbol = ST_Get,
        .put = ST_Put,
        .dtor = ST_Dtor,
        .ctor = ST_Ctor,
        .id_type_of_token_type = id_type_of_token_type,
        .add_builtin_function = Add_builtin_function,
        .traverse = Traverse,
};


#ifdef SELFTEST_symtable
#define HELLO "HELLO"

int main() {
    debug_msg("symtable selfdebug\n");

    return 0;
}
#endif

