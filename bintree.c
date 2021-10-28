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
};

/**
 * @brief Finds a node in a tree. The function uses a string to find the node.
 *
 * @param tree Pointer to first node of the tree.
 * @param id string name that we are looking for.
 *
 * @return Pointer to node or NULL if can not find.
 */
static node_t *Find(node_t *tree, dynstring_t *id) {
    node_t *help_var = tree;

    // Breaks when leaf level is reached.
    while (true) {
        int res = Dynstring.cmp(help_var->data.i_name, id);
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
 * @param id That we store.
 * @param type id type. Could be function, if statement, int, number, string ...
 * @return pointer to the node that can be stored to tree.
 */
static node_t *Ctor(dynstring_t *id, int type) {
    node_t *node = calloc(1, sizeof(node_t));
    if (node == NULL) {
        goto error_calloc;
    }

    /**  Writes data */
    node->r_child = NULL;
    node->l_child = NULL;

    node->data.type = type;
    node->data.i_name = id ;
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
 * @param id That we store.
 * @param type id type. Could be function, if statement, int, number, string ...
 * @return false if failed.
 */
static bool insert(node_t *root_node, dynstring_t *id, int type) {

    node_t *help_var = root_node;

    // Break if pointer reach leaf level so node can be inserted.
    while (true) {
        int res = Dynstring.cmp(help_var->data.i_name, id);
        if (res > 0) { // Store as left child
            if (help_var->l_child != NULL) {
                help_var = help_var->l_child;
            } else {
                help_var->l_child = Ctor(id, type);
                break;
            }
        } else { // store as right child.
            if (help_var->r_child != NULL) {
                help_var = help_var->r_child;
            } else {
                help_var->r_child = Ctor(id, type);
                break;
            }
        }
    }
    return true;
}

/**
 * Tree interface.
 */
const struct tree_interface_t Tree = {
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
