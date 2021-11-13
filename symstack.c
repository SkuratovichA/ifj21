/**
 * @file symstack.c
 *
 * @brief The file contains implementation of the stack of symbol tables.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */

#include "symstack.h"
#include "symtable.h"


/** Symbol stack element containing not only a symbol table,
 *  but also some information for semantic controls and code generation.
 */
typedef struct stack_el {
    struct stack_el *next; ///< next element in a list.
    symtable_t *table; ///< pointer on the binary tree with symtable.
    dynstring_t *fun_name; ///< if info != SCOPE_TYPE_function is NULL.
    scope_info_t info; ///< information about the scope.
} stack_el_t;

/** Stack of symbol tables.
 */
typedef struct symstack {
    stack_el_t *head; ///< head of the stack.
} symstack_t;

/** Unique id counter.
 */
static size_t __unique__id;


/** Pretty print function.
 *
 * @param type identifier type.
 * @return string representation of the identifier type.
 */
static char *scope_to_str(scope_type_t scope) {
    switch (scope) {
        #define X(s) case SCOPE_TYPE_##s: return #s;
        SCOPE_TYPE_T(X)
        #undef X
        default:
            return "undefined";
    }
}

/** Initialise a symbol stack.
 *
 * @return Pointer to allocated memory.
 */
static symstack_t *SS_Init() {
    debug_msg("\n[ctor] Init a symstack.\n");
    symstack_t *stack = calloc(1, sizeof(symstack_t));
    soft_assert(stack != NULL, ERROR_INTERNAL);
    return stack;
}

/** Push a new symbol table on the stack.
 *
 * @param self stack of symbol tables.
 * @param table symbol table to push.
 * @param scope_type function, do_cycle, cycle, condition etc.
 * @param fun_name if scope_type is not function, then NULL.
 */
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

/** Pop a symtable from a stack.
 *
 * @param self symstack.
 */
static void SS_Pop(symstack_t *self) {
    if (self == NULL) {
        debug_msg("\tTrying to pop from uninitialized stack. DONT!\n");
        return;
    }

    if (self->head == NULL) {
        debug_msg("\tTrying to pop from a headless stack. DONT!\n");
        return;
    }

    Symtable.dtor(self->head->table);
    Dynstring.dtor(self->head->fun_name);

    stack_el_t *del = self->head;
    self->head = self->head->next;
    free(del);
    debug_msg("[pop] popped a symtable\n");
}

/** Delete the whole stack with all symtables on it.
 *
 * @param self stack to delete.
 */
static void SS_Dtor(symstack_t *self) {
    stack_el_t *iter = self->head;
    stack_el_t *ptr = NULL;

    // iterate the whole stack and delete each element.
    while (iter != NULL) {
        ptr = iter->next;
        Dynstring.dtor(iter->fun_name);
        Symtable.dtor(iter->table);

        free(iter);
        iter = ptr;
    }

    free(self);
    debug_msg("[dtor] deleted symstack.\n");
}

/** Get a symbol from the symbol stack.
 *
 * @param self symbol stack.
 * @param id key.
 * @param sym a pointer to symbol to store a pointer to the object if we find it.
 * @return true if symbol exists.
 */
static bool SS_Get_symbol(symstack_t *self, dynstring_t *id,
                          symbol_t **sym, stack_el_t **def_scope) {
    debug_msg("\n");
    if (self == NULL) {
        debug_msg("[getter] Stack is null. Returning false.\n");
        return false;
    }

    stack_el_t *st = self->head;
    while (st != NULL) {
        if (Symtable.get_symbol(st->table, id, sym)) {
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

/** Get top table.
 *
 * @param self symbol stack.
 * @return symbol table.
 */
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

/** Put a symbol on the stack.
 *
 * @param self symstack.
 * @param id symbol that will be copied on the stack.
 * @param type type of an identifier.
 * @return pointer on symbol puched on the stack.
 */
static symbol_t *SS_Put_symbol(symstack_t *self, dynstring_t *id, id_type_t type) {
    soft_assert(self != NULL, ERROR_INTERNAL);

    // stack did not have a head.
    if (self->head == NULL) {
        debug_msg("\tStack has been empty. Create a frame(Symstack.push) before putting a symbol.\n");
        return NULL;
    }

    return Symtable.put(self->head->table, id, type);
}

/** Return information about the current scope.
 *
 * @param self symbol stack.
 * @return scope_info_t structure.
 */
static scope_info_t SS_Get_scope_info(symstack_t *self) {
    return self != NULL && self->head != NULL
           ? self->head->info
           : (scope_info_t) {.scope_type = SCOPE_TYPE_UNDEF, .scope_level = 0};
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

/** Traverse a symstack and apply a predicate on all the symbols.
 *
 * Function store the conjunction of all predicates.
 *
 * @param self symstack to traverse.
 * @param predicate predicate to apply.
 * @return
 */
static bool Traverse(symstack_t *self, bool (*predicate)(symbol_t *)) {
    if (self && predicate == NULL) {
        return false;
    }
    stack_el_t *stack_iter = self->head;
    bool acc = true;

    while (stack_iter != NULL) {
        acc &= Symtable.traverse(stack_iter->table, predicate);
    }

    return acc;
}


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
        .traverse = Traverse,
};
