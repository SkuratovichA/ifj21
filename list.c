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
static list_t *Ctor(void) {
    return calloc(1, sizeof(list_t));
}

/**
 * @brief Insert first element to a list.
 *
 * @param list Singly linked list.
 * @param data Data to insert.
 */
static void Prepend(list_t *list, void *data) {
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
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Delete_first(list_t *list, void (*clear_fun)(void *)) {
    soft_assert(list, ERROR_INTERNAL);
    list_item_t *tmp = list->head;
    list->head = list->head->next;
    clear_fun(tmp->data);
    free(tmp);
}

/**
 * @brief Delete all items in list.
 *
 * @param list singly linked list
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Clear(list_t *list, void (*clear_fun)(void *)) {
    while (list->head) {
        Delete_first(list, clear_fun);
    }
}

/**
 * @brief List destructor.
 *
 * @param list List to be destructed.
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Dtor(list_t *list, void (*clear_fun)(void *)) {
    Clear(list, clear_fun);
    free(list);
}

/**
 * @brief Insert new item behind reference_item.
 *
 * @param reference_item Item, behind that the new item will be inserted.
 * @param data New item's data.
 */
static void Insert(list_item_t *reference_item, void *data) {
    soft_assert(reference_item, ERROR_INTERNAL);
    list_item_t *new_item = calloc(1, sizeof(list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    new_item->data = data;
    list_item_t *tmp = reference_item->next;
    reference_item->next = new_item;
    new_item->next = tmp;
}

/**
 * @brief Returns the first item's data via data pointer.
 *
 * @param list singly linked list
 */
static void *Gethead(list_t *list) {
    if (!list->head) {
        return NULL;
    }
    return list->head->data;
}

/**
 * Interface to use when dealing with singly linked list.
 * Functions are in struct so we can use them in different files.
 */
const struct list_interface_t List = {
        .prepend = Prepend,
        .delete_list = Clear,
        .delete_first = Delete_first,
        .insert = Insert,
        .gethead = Gethead,
        .copy_data = NULL,
        .ctor = Ctor,
        .dtor = Dtor
};

#ifdef SELFTEST_list

int main() {
    printf("Selfdebug: %s\n", __FILE__);

    return 0;
}

#endif
