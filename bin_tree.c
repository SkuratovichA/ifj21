/********************************************
 * Project name: IFJ - projekt
 * File: bin_tree.c
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 *  Functions from bin_tree.c are called with: Tree.foo()
 *
 *  @package bin_tree
 *  @file bin_tree.c
 *  @brief File with all bin_tree functions.
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */

#include "bin_tree.h"

/**
 * @brief print whole tree
 *
 * @param
 * @return
 */
static void print_tree() {
    printf("I am tree.");
}


/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @return
 */
static int get_string_asci(char *str)
{
    printf("%c",str);
    return 0;
}

/**
 * @brief Create binare tree allocate space on heap for first(root) node.
 *
 * @param root Pointer to first node.
 * @param token
 * @param validity Information used to find identificator validity level.
 *
 * @return true. If error return false.
 *
 */
static bool create_tree(node *root, token_t *token, validity validity)
{
    root = calloc(1, sizeof(struct node));
    if (root == NULL)
        goto error1;

    root->data = calloc(1, sizeof(struct data));
    if (root->data == NULL)
        goto error1;


    root->data->i_name = token->attribute.id;
    root->data->asci_val = get_string_asci(root->data->i_name);

    root->data->type = validity.type;
    root->data->validity_field = validity.validity_field;
    root->data->validity_level = validity.validity_level;

    root->l_child = NULL;
    root->r_child = NULL;

    return true;

error1:
    fprintf(stderr,"calloc() error");
    Errors.return_error(ERROR_INTERNAL);
    return false;
}




/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @return
 */
static bool free_tree(node *root)
{
    if (root == NULL)
        return false;
}

/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @return
 */
static bool delete_node(node *node)
{
    if (node == NULL)
        return false;

}


/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @return
 */
static bool insert(token_t *token, validity validity) {
    printf("I am tree.");
    return true;
}



/**
 *
 * Tree interface.
 */
const struct tree_op_struct Tree = {
        .print_tree      = print_tree,
        .insert          = insert,
        .delete_node     = delete_node,
        .free_tree       = free_tree,
        .get_string_asci  = get_string_asci,
        .create_tree      = create_tree,

};