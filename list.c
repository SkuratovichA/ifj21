#include "list.h"

/**
 * List item struct.
 */
struct list_item {
    void *data;
    list_item_t *next;
};

/**
 * @brief List constructor.
 *
 * @return Pointer to the allocated memory.
 */
static list_t *list_ctor(void) {
    return calloc(1, sizeof(list_t));
}

/**
 * @brief List initializer.
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
    list_item_t *new_item = calloc(1, sizeof(list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    if (List.copy_data != NULL) {
        List.copy_data(new_item, data);
    } else {
        new_item->data = data;
    }
    new_item->next = list->head;
    list->head = new_item;
}

/**
 * @brief Delete the first item in list.
 *
 * @param list singly linked list.
 */
static void delete_first(list_t *list) {
    soft_assert(list, ERROR_INTERNAL);
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
    while (list->head) {
        delete_first(list);
    }
}

/**
 * @brief List destructor.
 *
 * @param list List to be destructed.
 */
static void list_dtor(list_t *list) {
    delete_list(list);
    free(list);
}

/**
 * @brief Insert new item behind reference_item.
 *
 * @param reference_item Item, behind that the new item will be inserted.
 * @param data New item's data.
 */
static void insert_behind(list_item_t *reference_item, void *data) {
    soft_assert(reference_item, ERROR_INTERNAL);
    list_item_t *new_item = calloc(1, sizeof(list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->data = data;
    list_item_t  *tmp = reference_item->next;
    reference_item->next = new_item;
    new_item->next = tmp;
}

/**
 * @brief Returns the first item's data via data pointer.
 *
 * @param list singly linked list
 */
static void *read_first(list_t *list) {
    if (!list->head)
        return NULL;
    return list->head->data;
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
        .insert_behind = insert_behind,
        .read_first = read_first,
        .copy_data = NULL,
        .list_ctor = list_ctor,
        .list_dtor = list_dtor
};
