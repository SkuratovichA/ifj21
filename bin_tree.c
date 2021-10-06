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
 * @param root Pointer to first node.
 * @param token
 * @param validity Information used to find identificator validity level.
 *
 * @return true. If error return NULL;
 *
 */
static node * create_tree(token_t *token, unsigned int validity){

    struct node *root = calloc(1, sizeof(struct node));
    if (root == NULL)
        goto error1;

    root->data = calloc(1, sizeof(struct data));
    if (root->data == NULL)
        goto error1;

    //TODO CALL CREATE NODE
    root->validity_field = validity;
    root->r_child = NULL;
    root->l_child = NULL;


    root->data->type = token->type; //TODO VARIABLE OF FUNCTION
    root->data->i_name = Dynstring.c_str(&token->attribute.id);

    //root->data->i_name_num = get_unique_id(root);

    return root;

error1:
    fprintf(stderr,"calloc() error");
    Errors.return_error(ERROR_INTERNAL);
    return NULL;
}




/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @return
 */
static bool delete_tree(node *root){
    if (root == NULL)
        return false;

    struct node *help_var = root;

    while (help_var != NULL){
        //delete right side
        while(help_var->r_child != NULL && help_var->l_child != NULL){
            if(help_var->r_child == NULL){
                help_var = help_var->l_child;
            }
        }
    }

    //delete left side

    return true;
    //TODO

}


/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @return
 */
static void delete_node(node *node){
    free(node->data);
    free(node);
}

/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @param
 * @param
 * @return false if validity doesn´t match
 */
static node * create_node(node *node, token_t *token){
    node = calloc(1, sizeof(struct node));
    if (node == NULL)
        goto error1;

    node->data = calloc(1, sizeof(struct data));
    if (node->data == NULL)
        goto error1;

    node->r_child = NULL;
    node->l_child = NULL;


    node->data->type = token->type; //TODO VARIABLE OF FUNCTION
    node->data->i_name = Dynstring.c_str(&token->attribute.id);
    return node;


error1:
    fprintf(stderr,"calloc() error");
    Errors.return_error(ERROR_INTERNAL);
    return NULL;
}

/**
 * @brief It insert node to binary tree.
 *
 * @param
 * @param
 * @param
 * @return false if validity doesn´t match
 */
static bool insert(node *node, token_t *token, unsigned int validity) {

    if(node->validity_field != validity){
        return false;
    }
    struct node *help_var = node;
    char *c = Dynstring.c_str(&token->attribute.id);

    while (true){
        /*If you are on leaf level.*/
        if (help_var == NULL){
            node = create_node(help_var, token);
            break;
        }
        /* If key is less go left*/
        else if ((strcmp(help_var->data->i_name, Dynstring.c_str(&token->attribute.id)) > 0)){
            help_var = help_var->l_child;
        }
        /* If key is more or equal go right*/
        else{
            help_var = help_var->r_child;
        }
    }

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
        .delete_tree       = delete_tree,
        .create_tree      = create_tree,
        .create_node      = create_node,

};