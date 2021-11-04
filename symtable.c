#include "symtable.h"
#include "tests/tests.h"
#include "errors.h"

#include "parser.h"


typedef struct symtable {
    struct symtable *left, *right;
    symbol_t symbol;
    int depth;
} symtable_t;


// symbol table
static void *ST_Ctor() {
    symtable_t *table = calloc(1, sizeof(symtable_t));
    soft_assert(table, ERROR_INTERNAL);
    return table;
}

static bool ST_Get(symtable_t *table, dynstring_t *id, symbol_t *container) {
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
    int res = Dynstring.cmp(id, table->symbol.id);
    if (res == 0) {
        return;
    }
    symtable_t **next = res > 0 ? &table->right : &table->left;

    if (*next == NULL) {
        *next = ST_Ctor();
        soft_assert(*next, ERROR_INTERNAL);
        (*next)->symbol.id = Dynstring.ctor(Dynstring.c_str(id));
        (*next)->symbol.type = type;
        return;
    }
    ST_Put(*next, id, type);
}

static void ST_Dtor(void *table) {
    if (table == NULL) {
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
    symstack_t *newstack = SS_Init();
    newstack->next = (*stack);
    newstack->table = table;
    (*stack) = newstack;
    debug_msg("New symtable pushed on the stack.\n");
}

static void SS_Pop(symstack_t **stack) {
    symstack_t *st = *stack;
    if ((*stack)->next == NULL) {
        debug_msg("Popping the last(global) stack.\n");
    }
    *stack = (*stack)->next;
    free(st);
}

static void SS_Dtor(void **stack) {
    while (*stack != NULL) {
        Symtable.dtor(((symstack_t *) (*stack))->table);
        SS_Pop((symstack_t **) stack);
    }
    free(*stack);
    debug_msg("Symstack destroyed.\n");
}

static bool SS_Get(symstack_t *stack, dynstring_t *id, symbol_t *sym) {
    symstack_t *st = stack;
    while (st != NULL) {
        if (Symtable.get(st->table, id, sym)) {
            return true;
        }
        st = st->next;
    }
    return false;
}

static void SS_Put(symstack_t *stack, dynstring_t *id, id_type_t type) {
    soft_assert(stack != NULL, ERROR_INTERNAL);
    if (stack->table == NULL) {
        fprintf(stderr, "There was no symtable on the stack.\nInitializing a global frame automatically\n");
        stack->table = Symtable.ctor();
    }
    Symtable.put(stack->table, id, type);
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
