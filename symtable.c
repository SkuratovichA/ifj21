#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#include "symtable.h"
#include "tests/tests.h"
#include "errors.h"

// TODO: move global_table, local_table here.

static size_t __unique__id;

// private structures
typedef struct node {
    symbol_t symbol;
    struct node *left, *right;
} node_t;

typedef struct symtable {
    node_t *root;
} symtable_t;

typedef struct stack_el {
    struct stack_el *next;
    symtable_t *table;
    dynstring_t *fun_name; // is not NULL iff info == SCOPE_TYPE_function
    scope_info_t info;
} stack_el_t;

typedef struct symstack {
    stack_el_t *head;
} symstack_t;


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
        default:
            return ID_TYPE_UNDEF;
    }
}

static char *scope_to_str(scope_type_t scope) {
    switch (scope) {
        #define X(s) case SCOPE_TYPE_##s: return #s;
        SCOPE_TYPE_T(X)
        #undef X
        default:
            return "undefined";
    }
}

static char *type_to_str(id_type_t type) {
    switch (type) {
        #define X(t) case ID_TYPE_##t: return #t;
        ID_TYPE_T(X)
        #undef X
        default:
            return "undefined";
    }
}

// symbol table
static symtable_t *ST_Ctor() {
    symtable_t *table = calloc(1, sizeof(symtable_t));
    soft_assert(table, ERROR_INTERNAL);
    debug_msg("[create] symtable.\n");
    return table;
}

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
 * @param id
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

static bool builtin_name(dynstring_t *name) {
    //TODO add more builtin names? or suppress the function.
    return strcmp(Dynstring.c_str(name), "read") == 0 ||
           strcmp(Dynstring.c_str(name), "write") == 0;
}

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
                debug_msg("\t[put] function declared '%s'\n", Dynstring.c_str(id));
            } else if (type == ID_TYPE_func_def) {
                Semantics.define((*iterator)->symbol.function_semantics);
                debug_msg("\t[put] function defined '%s'\n", Dynstring.c_str(id));
            } else {
                debug_msg("\t[put] {%s, %d} is already in the table\n", Dynstring.c_str(id), type);
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

    debug_msg("[put] new on { .id = '%s', .type = '%s' }.\n",
              Dynstring.c_str((*iterator)->symbol.id), type_to_str((*iterator)->symbol.type)
    );

    return &(*iterator)->symbol;
}

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

static void ST_Dtor(symtable_t *self) {
    if (self == NULL) {
        return;
    }
    _st_dtor(self->root);
    free(self);
    debug_msg("[dtor] Symtable deleted\n");
}

static void *SS_Init() {
    debug_msg("\n[ctor] Init a symstack.\n");
    return calloc(1, sizeof(symstack_t));
}

static void SS_Push(symstack_t *self, symtable_t *table, scope_type_t scope_type, char *fun_name) {
    // create a new elment.
    stack_el_t *stack_element = calloc(1, sizeof(stack_el_t));
    soft_assert(stack_element != NULL, ERROR_INTERNAL);

    // map a symtable.
    stack_element->table = table;
    // set info.
    stack_element->info.scope_type = scope_type;
    // set scope level. Either 0 or 1 + previous level.
    stack_element->info.scope_level = self->head != NULL ? self->head->info.scope_level + 1 : 0;
    // set a unique id for code generation.
    stack_element->info.unique_id = __unique__id++;

    if (scope_type == SCOPE_TYPE_function) {
        if (fun_name != NULL) {
            stack_element->fun_name = Dynstring.ctor(fun_name);
        } else {
            debug_msg("[push] Is this function really nameless?\n");
            stack_element->fun_name = Dynstring.ctor("nameless_function");
        }
    }

    // prepend new stack element.
    stack_element->next = self->head;
    self->head = stack_element;

    //debug_msg("[push] { .unique_id = '%zu', scope_level = '%zu', scope_type = '%s' }\n",
    //          stack_element->info.unique_id,
    //          stack_element->info.scope_level,
    //          scope_to_str(stack_element->info.scope_type)
    //);
}

static void SS_Pop(symstack_t *self) {
    if (self == NULL) {
        debug_msg("\tTrying to pop from uninitialized stack. DONT!\n");
        return;
    }

    if (self->head == NULL) {
        debug_msg("\tTrying to pop from a headless stack. DONT!\n");
        return;
    }

    ST_Dtor(self->head->table);
    self->head = self->head->next;
    debug_msg("[pop] popped a symtable\n");
}

static void SS_Dtor(symstack_t *self) {
    stack_el_t *iter = self->head, *ptr;

    // iterate the whole stack and delete each element.
    while (iter != NULL) {
        ptr = iter->next;
        if (iter->info.scope_type == SCOPE_TYPE_function) {
            Dynstring.dtor(iter->fun_name);
        }
        ST_Dtor(iter->table);
        free(iter);
        iter = ptr;
    }

    free(self);
    debug_msg("[dtor] deleted symstack.\n");
}

/** Get a symbol from the symbol stack.
 *
 * @param self symstack.
 * @param id key.
 * @param sym a pointer to symbol to store a pointer to the object if we find it.
 * @return
 */
static bool
SS_Get_symbol(
        symstack_t *self, dynstring_t *id,
        symbol_t **sym, stack_el_t **def_scope
) {
    debug_msg("\n");
    if (self == NULL) {
        debug_msg("[getter] Stack is null. Returning false.\n");
        return false;
    }

    stack_el_t *st = self->head;
    while (st != NULL) {
        if (ST_Get(st->table, id, sym)) {
            debug_msg("[getter] symbol found\n");
            if (def_scope != NULL) {
                *def_scope = st;
            }
            return true;
        }
        st = st->next;
    }
    return false;
}

static symtable_t *SS_Top(symstack_t *self) {
    if (self == NULL) {
        debug_msg("\tStack is null.\n");
        return NULL;
    }
    if (self->head == NULL) {
        debug_msg("\tHead is null.\n");
        return NULL;
    }
    return self->head->table;
}

static symbol_t *SS_Put_symbol(symstack_t *self, dynstring_t *id, id_type_t type) {
    soft_assert(self != NULL, ERROR_INTERNAL);

    // stack did not have a head.
    if (self->head == NULL) {
        debug_msg("\tStack has been empty. Create a frame(Symstack.push) before putting a symbol.\n");
        return NULL;
    }

    return ST_Put(self->head->table, id, type);
}

static scope_info_t SS_Get_scope_info(symstack_t *self) {
    debug_msg("\n");
    return self != NULL && self->head != NULL
           ? self->head->info
           : (scope_info_t) {.scope_type = SCOPE_TYPE_UNDEF, .scope_level = 0};
}

static void Add_builtin_function(symtable_t *self, char *name, char *params, char *returns) {
    if ((bool) self && (bool) name == 0) {
        debug_msg("\tnull passed into a function...\n");
        return;
    }
    dynstring_t *dname = Dynstring.ctor(name);
    dynstring_t *paramvec = Dynstring.ctor(params);
    dynstring_t *returnvec = Dynstring.ctor(returns);

    ST_Put(self, dname, ID_TYPE_func_decl);
    ST_Put(self, dname, ID_TYPE_func_def);

    symbol_t *symbol;
    ST_Get(self, dname, &symbol);

    Semantics.builtin(symbol->function_semantics);
    Semantics.set_params(symbol->function_semantics->definition, paramvec);
    Semantics.set_returns(symbol->function_semantics->definition, returnvec);

    Semantics.set_params(symbol->function_semantics->declaration, paramvec);
    Semantics.set_returns(symbol->function_semantics->declaration, returnvec);

    Dynstring.dtor(dname);
    Dynstring.dtor(paramvec);
    Dynstring.dtor(returnvec);
    debug_msg("\t[BUILTIN]: builtin function is set.\n");
}

/** Get a parent function name.
 *
 * @param self
 * @return NULl if there is a global frame.
 */
static char *SS_Get_parent_func_name(symstack_t *self) {
    if (self == NULL) {
        return NULL;
    }
    stack_el_t *iter = self->head;

    while (iter != NULL) {
        if (iter->info.scope_type == SCOPE_TYPE_function) {
            return Dynstring.c_str(iter->fun_name);
        }
        iter = iter->next;
    }
    return NULL;
}

//=================================================
const struct symstack_interface_t Symstack = {
        .init = SS_Init,
        .push = SS_Push,
        .pop = SS_Pop,
        .dtor = SS_Dtor,
        .get_symbol = SS_Get_symbol,
        .put_symbol = SS_Put_symbol,
        .top = SS_Top,
        .get_scope_info = SS_Get_scope_info,
        .get_parent_func_name = SS_Get_parent_func_name,
};

const struct symtable_interface_t Symtable = {
        .get_symbol = ST_Get,
        .put = ST_Put,
        .dtor = ST_Dtor,
        .ctor = ST_Ctor,
        .id_type_of_token_type = id_type_of_token_type,
        .add_builtin_function = Add_builtin_function,
};


#ifdef SELFTEST_symtable
#define HELLO "HELLO"

int main() {
    debug_msg("symtable selfdebug\n");

    return 0;
}

#endif

#pragma clang diagnostic pop
