#include "symtable.h"
#include "tests/tests.h"
#include "errors.h"

// TODO: move global_table, local_table here.

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
    scope_info_t info;
} stack_el_t;

typedef struct symstack {
    stack_el_t *head;
} symstack_t;


static char *type_to_str(id_type_t type) {
    switch (type) {
        case TYPE_func_decl:
            return "func_decl";
        case TYPE_integer:
            return "integer";
        case TYPE_boolean:
            return "boolean";
        case TYPE_number:
            return "number";
        case TYPE_func_def:
            return "func_def";
        case TYPE_string:
            return "string";
        default:
            break;
    }
    return "undefined type :(";
}

// symbol table
static symtable_t *ST_Ctor() {
    debug_msg("\n");
    symtable_t *table = calloc(1, sizeof(symtable_t));
    soft_assert(table, ERROR_INTERNAL);

    debug_msg("Create a new symtable.\n");
    return table;
}

static bool _st_get(node_t *node, dynstring_t *id, symbol_t *storage) {
    if (node == NULL) {
        return false;
    }

    int res = Dynstring.cmp(id, node->symbol.id);
    if (res == 0) {
        if (storage != NULL) {
            *storage = node->symbol;
        }
        return true;
    }
    return _st_get(res > 0 ? node->right : node->left, id, storage);
}

static bool ST_Get(symtable_t *self, dynstring_t *id, symbol_t *storage) {
    debug_msg("\n");
    if (self == NULL) {
        return false;
    }

    return _st_get(self->root, id, storage);
}

static void ST_Put(symtable_t *self, dynstring_t *id, id_type_t type) {
    debug_msg("\n");
    if (self == NULL) {
        debug_msg("Null passed to a function.\n");
        return;
    }

    node_t **iterator = &self->root;
    int res;

    while ((*iterator) != NULL) {
        res = Dynstring.cmp(id, (*iterator)->symbol.id);
        if (res == 0) {
            debug_msg("Item {%s, %d} is already in the table\n", Dynstring.c_str(id), type);
            return;
        }
        iterator = res > 0 ? &(*iterator)->right : &(*iterator)->left;
    }

    // base case with no root
    (*iterator) = calloc(1, sizeof(node_t));
    soft_assert((*iterator) != NULL, ERROR_INTERNAL);
    (*iterator)->symbol.id = Dynstring.ctor(Dynstring.c_str(id));
    (*iterator)->symbol.type = type;
    debug_msg("Add new item to symtable: new root created with new data:"
              "{ .id = '%s', .type = '%s' }.\n",
              Dynstring.c_str((*iterator)->symbol.id), type_to_str((*iterator)->symbol.type)
    );
}

static void _st_dtor(node_t *node) {
    debug_msg("\n");
    if (node == NULL) {
        return;
    }
    _st_dtor(node->left);
    _st_dtor(node->right);
    Dynstring.dtor(node->symbol.id);
    free(node);
}

static void ST_Dtor(symtable_t *self) {
    debug_msg("\n");
    if (self == NULL) {
        debug_msg("Base case. Returning :).\n");
        return;
    }
    _st_dtor(self->root);
    free(self);
    debug_msg("symtable node freed\n");
}

static void *SS_Init() {
    return calloc(1, sizeof(symstack_t));
}

static void SS_Push(symstack_t *self, symtable_t *table, scope_type_t scope_type) {
    debug_msg("\n");

    stack_el_t *st = calloc(1, sizeof(stack_el_t));
    soft_assert(st != NULL, ERROR_INTERNAL);

    size_t level = self->head != NULL ? self->head->info.scope_level + 1 : 0;

    st->table = table;
    st->next = self->head;
    st->info.scope_type = scope_type, st->info.scope_level = level;

    self->head = st;

    debug_msg("New symtable pushed on the stack.\n");
}

static void SS_Pop(symstack_t *self) {
    debug_msg("\n");
    if (self == NULL) {
        debug_msg("Trying to pop from uninitialized stack. DONT!\n");
        return;
    }

    if (self->head == NULL) {
        debug_msg("Trying to pop from a headless stack. DONT!\n");
        return;
    }

    ST_Dtor(self->head->table);
    self->head = self->head->next;
    debug_msg("Popped from a stack\n");
}

static void SS_Dtor(symstack_t *self) {
    debug_msg("\n");
    stack_el_t *iter = self->head, *ptr;

    // iterate the whole stack and delete each element.
    while (iter != NULL) {
        ptr = iter->next;
        ST_Dtor(iter->table);
        free(iter);
        iter = ptr;
    }

    free(self);
    debug_msg("Symstack destroyed.\n");
}

static bool SS_Get_symbol(symstack_t *self, dynstring_t *id, symbol_t *sym) {
    debug_msg("\n");
    if (self == NULL) {
        debug_msg("Stack is null. Returning false.\n");
        return false;
    }

    stack_el_t *st = self->head;
    while (st != NULL) {
        if (ST_Get(st->table, id, sym)) {
            debug_msg("Found a name. Returning true.\n");
            return true;
        }
        st = st->next;
    }
    return false;
}

static symtable_t *SS_Top(symstack_t *self) {
    if (self == NULL) {
        debug_msg("Stack is null.\n");
        return NULL;
    }
    if (self->head == NULL) {
        debug_msg("Head is null.\n");
        return NULL;
    }
    return self->head->table;
}

static void SS_Put(symstack_t *self, dynstring_t *id, id_type_t type) {
    debug_msg("\n");
    soft_assert(self != NULL, ERROR_INTERNAL);

    // stack did not have a head.
    if (self->head == NULL) {
        debug_msg("Stack has been empty. Create a new head(global frame).\n");
        // create a new stack element.
        self->head = calloc(1, sizeof(stack_el_t));
        soft_assert(self->head != NULL, ERROR_INTERNAL);

        // initialize a new table
        self->head->table = ST_Ctor();
        soft_assert(self->head != NULL, ERROR_INTERNAL);
        self->head->info.scope_level = 0;
        debug_msg("Symtable on the stack created.\n");
        // global_frame = stack->head;
        self->head->info.scope_type = SCOPE_global;
    }
    ST_Put(self->head->table, id, type);
}

static scope_info_t SS_Get_scope_info(symstack_t *self) {
    return self != NULL && self->head != NULL
           ? self->head->info : (scope_info_t) {.scope_type = 0, .scope_level = 0};
}

//=================================================
const struct symtable_interface_t Symtable = {
        .get = ST_Get,
        .put = ST_Put,
        .dtor = ST_Dtor,
        .ctor = ST_Ctor,
};

const struct symstack_interface_t Symstack = {
        .init = SS_Init,
        .push = SS_Push,
        .pop = SS_Pop,
        .dtor = SS_Dtor,
        .get_symbol = SS_Get_symbol,

        .put = SS_Put,
        .top = SS_Top,
        .get_scope_info = SS_Get_scope_info,
};

#ifdef SELFTEST_symtable
#define HELLO "HELLO"

int main() {
    debug_msg("symtable selfdebug\n");

    symtable_t *t = Symtable.ctor();
    symstack_t *stack = Symstack.init();
    symbol_t symbol;
    dynstring_t *hello = Dynstring.ctor(HELLO);

    char *strs[6] = {"aaa", "bbb", "ccc", "ddd", "eee", "fff"};

    // put an item on the "global frame"
    Symstack.push(stack, t);
    Symstack.put(stack, hello, TYPE_number);

    // create a new frame.
    Symstack.push(stack, Symtable.ctor());

    // push on new frame.
    for (int i = 0; i < 6; i++) {
        Symstack.put(stack, Dynstring.ctor(strs[i]), TYPE_string);
    }

    // find elements.
    for (int i = 0; i < 6; i++) {
        if (Symstack.get_symbol(stack, Dynstring.ctor(strs[i]), &symbol)) {
            printf("found: %s\n", Dynstring.c_str(symbol.id));
        } else {
            printf("not found.\n");
        }
    }

    if (Symstack.get_symbol(stack, hello, &symbol)) {
        printf("Ready to push :)\n");
    }

    //Symstack.pop(stack);
    Symstack.dtor(stack);
    return 0;
}

#endif
