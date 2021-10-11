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
 * @brief Create binare tree allocate space on heap for first(root) node.
 *
 * @param tree Pointer to first node of tree.
 * @param token name that we are looking for.
 *
 * @return Pointer to node or NULL if not find.
 */
static node * find_node(node * tree, string name){
    struct node *help_var = tree;
    char *c = Dynstring.c_str(&name);

    while(true){ //Break if leaf level reached
        if((strcmp(help_var->data->i_name, Dynstring.c_str(&name))) == 0){
            return help_var; //FINDED
        }else if ((strcmp(help_var->data->i_name, Dynstring.c_str(&name)) > 0)){
            if(help_var->l_child == NULL){
                break; //not in tree
            }
            else{
                help_var = help_var->l_child;
            }
        }else{
            if(help_var->r_child == NULL){
                break; //Not in tree
            }else{
                help_var = help_var->r_child;
            }
        }
    }
    return NULL;
}


/**
 * @brief Create binare tree allocate space on heap for first(root) node.
 *
 * @param root Pointer to first node.
 * @param token Token generated from lexical analysis.
 * @param validity Information used to find identificator validity level.
 *
 * @return Pointer to first bin. tree node. If allocation failed return NULL
 */
static node * create_tree(token_t *token, unsigned int validity){

    struct node *root = calloc(1, sizeof(struct node));
    if (root == NULL)
        goto error1;

    root->data = calloc(1, sizeof(struct data));
    if (root->data == NULL)
        goto error1;

    /******** Set given values */
    root->validity_field = validity;
    root->r_child = NULL;
    root->l_child = NULL;

    root->data->type = token->type; //TODO VARIABLE OF FUNCTION
    root->data->i_name = Dynstring.c_str(&token->attribute.id);
    /***********/

    return root;

    /*** Calloc error */
error1:
    fprintf(stderr,"calloc() error");
    Errors.return_error(ERROR_INTERNAL);
    return NULL;
}

/**
 * @brief Delete given node from binary tree. Do not check if node is on leaf level. Free allocated space.
 *
 * @param node Pointer to one of tree nodes, that will be deleted.
 * @return void
 */
static void delete_node(node *node){
    if(node == NULL)
        return;
    if(node->data != NULL)
        free(node->data);
    free(node);
}

/**
 * @brief Recursive function that deletes all nodes in tree. Free allocated space.
 *
 * @param node root node from tree that we want to allocate.
 * @return void
 */
static void delete_tree(node * root){
    if(root == NULL)
        return;
    delete_tree(root->r_child);
    delete_tree(root->l_child);

    free(root->data);
    free(root);
}

/**
 * @brief Allocate space for node and write data to it and returns it.
 *
 * @param
 * @return pointer to node that can be stored to tree.
 */
static node * create_node(token_t *token){

    node *node = calloc(1, sizeof(struct node));
    if (node == NULL)
        goto error1;

    node->data = calloc(1, sizeof(struct data));
    if (node->data == NULL)
        goto error1;

    /**  Writes data */
    node->r_child = NULL;
    node->l_child = NULL;

    node->data->type = token->type;
    node->data->i_name = Dynstring.c_str(&token->attribute.id);
    /*******/
    return node;

    /** Calloc error */
error1:
    fprintf(stderr,"calloc() error");
    Errors.return_error(ERROR_INTERNAL);
    return NULL;
}

/**
 * @brief It insert node to binary tree. If string value of node is smaller go left else go right till reach leaf level.
 *
 * @param node Defines in what tree.
 * @param token Data that ll be stored.
 * @param validity Validity level of data (scope hiearchy)
 * @return false if validity doesn´t match given tree, else return true.
 */
static bool insert(node *node, token_t *token, unsigned int validity) {

    if(node->validity_field != validity){
        return false;
    }

    struct node *help_var = node;
    char *c = Dynstring.c_str(&token->attribute.id);

    while(true){ //Break if pointer reach leaf level so node can be inserted.
        if ((strcmp(help_var->data->i_name, Dynstring.c_str(&token->attribute.id)) > 0)){
            if(help_var->l_child != NULL){
                help_var = help_var->l_child;
            }else{
               help_var->l_child = create_node(token);
               break;
            }
        }else{
            if(help_var->r_child != NULL){
                help_var = help_var->r_child;
            }else{
                help_var->r_child = create_node(token);
                break;
            }
        }
    }
    return true;
}


/**
 *
 * Tree interface.
 */
const struct tree_op_struct Tree = {
        .find_node       = find_node,
        .print_tree      = print_tree,
        .insert          = insert,
        .delete_node     = delete_node,
        .delete_tree     = delete_tree,
        .create_tree     = create_tree,
        .create_node     = create_node,
};