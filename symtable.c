#include "symtable.h"
#include "errors.h"
#include "scanner.h"
#include "tests/tests.h"

struct symtable {
    symtable_t *parent;    /**< */
    symtable_t *child;     /**< */
    struct node_t *tree;   /**< */
    int scope_index;       /**< */
};

/**
 * @brief
 *
 * @param
 * @return
 */
static symtable_t *Ctor(symtable_t *table, token_t token) {
    debug_msg("implement me.\n");
    debug_msg("Got token: %s '%s'\n", Scanner.to_string(token.type),
              (token.type == TOKEN_STR || token.type == TOKEN_ID) ? Dynstring.c_str(token.attribute.id) : ""
    );

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
const struct symtable_interface_t Symtable = {
        .ctor = Ctor,
};

#ifdef SELFTEST_symtable

int main() {
    debug_msg("symtable selfdebug\n");
    token_t tok = { .type = TOKEN_STR, .attribute.id = Dynstring.ctor("\"Test name\"") };
    Symtable.ctor(NULL, tok);

    return 0;
}

#endif
