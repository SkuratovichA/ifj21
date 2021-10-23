#include "symtable.h"

/**
 * @brief
 *
 * @param
 * @return
 */
static sym_t *Ctor(sym_table *table, token_t *token) {
    /*
    node_t *node = calloc(1, sizeof(node_t));
    if (node == NULL) {
        goto error_calloc;
    }

    node->r_child = NULL;
    node->l_child = NULL;

    node->data.type = token->type;
    node->data.i_name = token->attribute.id;

    return node;

    error_calloc:
    fprintf(stderr, "calloc() error\n");
    Errors.set_error(ERROR_INTERNAL);
    */
    return NULL;
}




/**
 * Symtable interface.
 */

const struct sym_table_op_struct Symtable = {
        .ctor = Ctor,
};
