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
static void *ST_Ctor(int depth) {
    symtable_t *table = calloc(1, sizeof(symtable_t));
    soft_assert(table, ERROR_INTERNAL);
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
}

static void SS_Pop(symstack_t **stack) {
    symstack_t *st = *stack;
    *stack = (*stack)->next;
    free(st);
}

static void SS_Dtor(symstack_t *stack) {
    symstack_t *st = stack, *nn;

    while (st != NULL) {
        Symtable.dtor(st->table);
        nn = st;
        st = st->next;
        free(nn);
    }
    free(stack);
}

static symbol_t *SS_Get(symstack_t *stack, dynstring_t *id) {
    symstack_t *st = stack;
    symbol_t *sym = calloc(1, sizeof(symbol_t *));
    while (st != NULL) {
        if (Symtable.get(st->table, id, sym)) {
            return sym;
        }
        st = st->next;
    }
    free(sym);
    return NULL;
}

static void SS_Put(symstack_t *stack, dynstring_t *id, id_type_t type) {
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
