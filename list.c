#include "list.h"

/**
 * List item struct.
 */
struct list_item {
    void *data;
    list_item_t *next;
};

/**
 * @brief List initialiser.
 *
 * @param list Singly linked list to initialise.
 */
static void list_init(list_t *list) {
    list->head = NULL;
}

/**
 * @brief Insert first element to a list.
 *
 * @param list Singly linked list.
 * @param data Data to insert.
 */
static void insert_first(list_t *list, void *data) {
    list_item_t  *new_item = malloc(sizeof(list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->data = data;
    new_item->next = list->head;
    list->head = new_item;
}

/**
 * @brief Delete the first item in list.
 *
 * @param list singly linked list.
 */
static void delete_first(list_t *list) {
    list_item_t *tmp = list->head;
    list->head = list->head->next;
    free(tmp);
}

/**
 * @brief Delete all items in list.
 *
 * @param list singly linked list
 */
static void delete_list(list_t *list) {
    while (!(list->head)) {
        delete_first(list);
    }
}

/**
 * @brief Insert new item behind reference_item.
 *
 * @param reference_item Item, behind that the new item will be inserted.
 * @param data New item's data.
 */
static void insert_behind(list_item_t *reference_item, void *data) {
    soft_assert(reference_item, ERROR_INTERNAL);
    list_item_t *new_item = malloc(sizeof(list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->data = data;
    list_item_t  *tmp = reference_item->next;
    reference_item->next = new_item;
    new_item->next = tmp;
}

/**
 * Interface to use when dealing with singly linked list.
 * Functions are in struct so we can use them in different files.
 */
const struct list_interface_t List = {
        .list_init =  list_init,
        .insert_first = insert_first,
        .delete_first = delete_first,
        .delete_list = delete_list,
        .insert_behind = insert_behind
};
