#include "bintree.h"


/**
 * @brief Finds a node in a tree. The function uses a string to find the node.
 *
 * @param tree Pointer to first node of the tree.
 * @param token string name that we are looking for.
 *
 * @return Pointer to node or NULL if can not find.
 */
static node * find_node(node *tree, dynstring_t name) {
    struct node *help_var = tree;

    // Breaks when leaf level is reached.
    while (true) {
        int res = Dynstring.cmp(help_var->data->i_name, name);
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
 * @brief Create binare tree on heap. Tree is created when we create first node.
 *
 * @param root Pointer to the first node, also pointer to whole tree.
 * @param token Token generated from lexical analysis. 
 * @param validity Information used to find identificator validity level.
 *
 * @return Pointer to the first bin. tree node. If allocation failed return NULL
 */
static node *create_tree(token_t *token, unsigned int validity) {

    struct node *root = calloc(1, sizeof(struct node));
    if (root == NULL) {
        goto error_calloc;
    }

    root->data = calloc(1, sizeof(struct data));
    if (root->data == NULL) {
        goto error_calloc_data;
    }

    /******** Set given values */
    root->validity_field = validity;
    root->r_child = NULL;
    root->l_child = NULL;

    root->data->type = token->type;
    root->data->i_name = Dynstring.create(token->attribute.id.str);
    /***********/

    return root;

error_calloc_data:
    free(root);

error_calloc:
    fprintf(stderr, "calloc() error\n");
    Errors.set_error(ERROR_INTERNAL);
    return NULL;


}

/**
 * @brief Delete given node from binary tree. Do not check if node is on leaf level. Free allocated space.
 *
 * @param node Pointer to one of tree nodes, that will be deleted.
 */
static void delete_node(node *node) {
    if (node == NULL) {
        return;
    }
    if (node->data != NULL) {
        free(node->data);
    }
    free(node);
}

/**
 * @brief Recursive function that deletes all nodes in tree. Free allocated space.
 *
 * @param node root node from tree that we want to allocate.
 */
static void delete_tree(node *root) {
    if (root == NULL) {
        return;
    }
    delete_tree(root->r_child);
    delete_tree(root->l_child);

    Dynstring.free(&root->data->i_name);
    free(root->data);
    free(root);
}

/**
 * @brief Create new node on heap.
 *
 * @param token is structure that stores data.
 * @return pointer to the node that can be stored to tree.
 */
static node *create_node(token_t *token) {

    node *node = calloc(1, sizeof(struct node));
    if (node == NULL) {
        goto error_calloc;
    }

    node->data = calloc(1, sizeof(struct data));
    if (node->data == NULL) {
        goto error_calloc_data;
    }

    /**  Writes data */
    node->r_child = NULL;
    node->l_child = NULL;

    node->data->type = token->type;
    node->data->i_name = Dynstring.create(token->attribute.id.str);
    /*******/

    return node;


error_calloc_data:
    free(node);

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
static bool insert(node *root_node, token_t *token, unsigned int validity) {

    if (root_node->validity_field != validity) {
        return false;
    }

    struct node *help_var = root_node;

    // Break if pointer reach leaf level so node can be inserted.
    while (true) {
        int res = Dynstring.cmp(help_var->data->i_name, token->attribute.id);
        if (res > 0) { // Store as left child
            if (help_var->l_child != NULL) {
                help_var = help_var->l_child;
            } else {
                help_var->l_child = create_node(token);
                break;
            }
        } else { // store as right child.
            if (help_var->r_child != NULL) {
                help_var = help_var->r_child;
            } else {
                help_var->r_child = create_node(token);
                break;
            }
        }
    }
    return true;
}

/**
 * Tree interface.
 */
const struct tree_op_struct Tree = {
        .find_node       = find_node,
        .insert          = insert,
        .delete_node     = delete_node,
        .delete_tree     = delete_tree,
        .create_tree     = create_tree,
        .create_node     = create_node,
};
