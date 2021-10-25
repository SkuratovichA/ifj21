#include "bintree.h"


/**
 * Binary tree structure. each node has pointer to two child nodes left and right.
 * Each node can have 0-2 child node.
 * Each node has just one parent.
 *
 * .data has to be greater than .left child .data
 * .data has to be less than .right child .data
 */
struct node {
    struct data data;           /**< Data stored in node.   */
    node_t *l_child;        /**< Pointer to left child.  */
    node_t *r_child;        /**< Pointer to right child. */
    unsigned int validity_field; /**< We are creating trees for each validity field */
};

/**
 * @brief Finds a node in a tree. The function uses a string to find the node.
 *
 * @param tree Pointer to first node of the tree.
 * @param token string name that we are looking for.
 *
 * @return Pointer to node or NULL if can not find.
 */
static node_t *Find(node_t *tree, dynstring_t *name) {
    node_t *help_var = tree;

    // Breaks when leaf level is reached.
    while (true) {
        int res = Dynstring.cmp(help_var->data.i_name, name);
        if (!res) {
            return help_var; //node found
        } else if (res > 0) {
            if (help_var->l_child == NULL) {
                break; // not in tree
            } else {
                help_var = help_var->l_child;
            }
        } else {
            if (help_var->r_child == NULL) {
                break; //Not in tree
            } else {
                help_var = help_var->r_child;
            }
        }
    }
    return NULL;
}

/**
 * @brief Recursive function that deletes all nodes in tree. Free allocated space.
 *
 * @param node root node from tree that we want to allocate.
 */
static void Dtor(node_t *root) {
    if (root == NULL) {
        return;
    }
    Dtor(root->r_child);
    Dtor(root->l_child);

    Dynstring.dtor(root->data.i_name);
    free(root);
}

/**
 * @brief Create a new node.
 *
 * @param token is structure that stores data.
 * @return pointer to the node that can be stored to tree.
 */
static node_t *Ctor(token_t *token) {
    node_t *node = calloc(1, sizeof(node_t));
    if (node == NULL) {
        goto error_calloc;
    }

    /**  Writes data */
    node->r_child = NULL;
    node->l_child = NULL;

    node->data.type = token->type;
    node->data.i_name = token->attribute.id;
    /*******/

    return node;

    error_calloc:
    fprintf(stderr, "calloc() error\n");
    Errors.set_error(ERROR_INTERNAL);
    return NULL;
}

/**
 * @brief It inserts a node into binary tree. If the string value of the node is smaller go left else go right till reach leaf level.
 *
 * @param root_node Defines tree we are searching in.
 * @param token Data that will be stored.
 * @param validity Validity level of data (scope hiearchy)
 * @return false if validity doesn't match given tree, else return true.
 */
static bool insert(node_t *root_node, token_t *token, unsigned int validity) {

    if (root_node->validity_field != validity) {
        return false;
    }

    node_t *help_var = root_node;

    // Break if pointer reach leaf level so node can be inserted.
    while (true) {
        int res = Dynstring.cmp(help_var->data.i_name, token->attribute.id);
        if (res > 0) { // Store as left child
            if (help_var->l_child != NULL) {
                help_var = help_var->l_child;
            } else {
                help_var->l_child = Ctor(token);
                break;
            }
        } else { // store as right child.
            if (help_var->r_child != NULL) {
                help_var = help_var->r_child;
            } else {
                help_var->r_child = Ctor(token);
                break;
            }
        }
    }
    return true;
}

/**
 * Tree interface.
 */
const struct bintree_interface_t Tree = {
        .find = Find,
        .insert = insert,
        .dtor = Dtor,
        .ctor = Ctor,
};

#ifdef SELFTEST_bintree
int main() {
    printf("Selfdebug: %s\n", __FILE__);


    return 0;
}
#endif
