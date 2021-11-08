#include "symtable.h"
#include "tests/tests.h"
#include "errors.h"

#include "parser.h"


typedef struct symtable {
    struct symtable *left, *right;
    symbol_t symbol;
} symtable_t;

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
static void *ST_Ctor() {
    debug_msg("\n");
    symtable_t *table = calloc(1, sizeof(symtable_t));
    soft_assert(table, ERROR_INTERNAL);
    debug_msg("Create a new symtable.\n");
    return table;
}

static bool ST_Get(symtable_t *table, dynstring_t *id, symbol_t *container) {
    debug_msg("\n");
    if (table == NULL) {
        return false;
    }
    int res = Dynstring.cmp(id, table->symbol.id);
    if (res == 0) {
        if (container != NULL) {
            *container = table->symbol;
        }
        return true;
    }
    return ST_Get(res > 0 ? table->right : table->left, id, container);
}

static void ST_Put(symtable_t *table, dynstring_t *id, id_type_t type) {
    debug_msg("\n");
    int res = Dynstring.cmp(id, table->symbol.id);
    if (res == 0) {
        debug_msg("Item is already in the table.\n");
        return;
    }
    symtable_t **next = res > 0 ? &table->right : &table->left;

    if (*next == NULL) {
        *next = ST_Ctor();
        soft_assert(*next, ERROR_INTERNAL);
        (*next)->symbol.id = Dynstring.ctor(Dynstring.c_str(id));
        (*next)->symbol.type = type;
        debug_msg("Base case: new node created with new data:"
                  "{ .id = '%s', .type = '%s' }.\n",
                  Dynstring.c_str(id), type_to_str(type));
        return;
    }
    ST_Put(*next, id, type);
}

static void ST_Dtor(void *table) {
    debug_msg("\n");
    if (table == NULL) {
        debug_msg("Base case. Returning :).\n");
        return;
    }
    Dynstring.dtor(((symtable_t *) table)->symbol.id);
    ST_Dtor(((symtable_t *) table)->left);
    ST_Dtor(((symtable_t *) table)->right);
    free(table);
    debug_msg("symtable node freed\n");
}


const struct symtable_interface_t Symtable = {
        .get = ST_Get,
        .put = ST_Put,
        .dtor = ST_Dtor,
        .ctor = ST_Ctor,
};


typedef struct symstack {
    struct symstack *next;
    symtable_t *table;
} symstack_t;

static void *SS_Init() {
    return calloc(1, sizeof(symstack_t));
}

static void SS_Push(symstack_t **stack, symtable_t *table) {
    debug_msg("\n");
    symstack_t *newstack = SS_Init();
    debug_msg("New stack created on '%p'.\n", (void *) newstack);

    newstack->next = (*stack);
    debug_msg("New stack->next pointing to old stack: '%p'->'%p'.\n",
              (void *) newstack, (void *) *stack);
    debug_assert(*stack == newstack->next);

    newstack->table = table;

    debug_msg("Stack points to old stack: '%p'->'%p'.\n",
              (void *) (*stack), (void *) (*stack)->next);
    (*stack) = newstack;

    debug_msg("New symtable pushed on the stack.\n");
}

static void SS_Pop(symstack_t **stack) {
    debug_msg("\n");
    symstack_t *st = *stack;
    if ((*stack)->next == NULL) {
        debug_msg("Popping the last(global) symtable. "
                  "So symstack is empty now.\n");
    }
    *stack = (*stack)->next;

    ST_Dtor(st->table);
    free(st);
}

static void SS_Dtor(void **stack) {
    debug_msg("\n");
    while (*stack != NULL) {
        ST_Dtor(((symstack_t *) (*stack))->table);
        SS_Pop((symstack_t **) stack);
    }
    free(*stack);
    debug_msg("Symstack destroyed.\n");
}

static bool SS_Get(symstack_t *stack, dynstring_t *id, symbol_t *sym) {
    debug_msg("\n");
    symstack_t *st = stack;
    while (st != NULL) {
        if (ST_Get(st->table, id, sym)) {
            return true;
        }
        st = st->next;
    }
    return false;
}

static void SS_Put(symstack_t *stack, dynstring_t *id, id_type_t type) {
    debug_msg("\n");

    soft_assert(stack != NULL, ERROR_INTERNAL);
    if (stack->table == NULL) {
        fprintf(stderr, "There was no symtable on the stack.\n"
                        "Initializing a global frame automatically\n");
        stack->table = Symtable.ctor();
    }
    ST_Put(stack->table, id, type);
}


const struct symstack_interface_t Symstack = {
        .init = SS_Init,
        .push = SS_Push,
        .pop = SS_Pop,
        .dtor = SS_Dtor,
        .get = SS_Get,
        .put = SS_Put,
};


#ifdef SELFTEST_symtable
int main() {
    debug_msg("symtable selfdebug\n");
    token_t tok = { .type = TOKEN_STR, .attribute.id = Dynstring.ctor("\"Test name\"") };
    Symtable.ctor(NULL, tok);

    return 0;
}

#endif
