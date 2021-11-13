/**
 * @file list.c
 *
 * @brief
 *
 * @author Svobodova Lucie
 */
#include "list.h"


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

    if (list->head == NULL) {
        list->tail = new_item;
    }

    list->head = new_item;
}

static void Append(list_t *list, void *data) {
    list_item_t *new_item = calloc(1, sizeof(list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    if (List.copy_data != NULL) {
        List.copy_data(new_item, data);
    } else {
        new_item->data = data;
    }
    if (list->head == NULL) {
        list->head = new_item;
        list->tail = list->head;
    } else {
        list->tail->next = new_item;
        list->tail = new_item;
    }
    new_item->next = NULL;
}

static void Insert_after(list_item_t *item, void *data) {
    soft_assert(item, ERROR_INTERNAL);
    list_item_t *new_item = calloc(1, sizeof (list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    if (List.copy_data != NULL) {
        List.copy_data(new_item, data);
    } else {
        new_item->data = data;
    }
    new_item->next = item->next;
    item->next = new_item;
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
 * @brief Recursively compare 2 lists.
 * @param l1
 * @param l2
 * @param cmp
 * @return bool.
 */
static bool _equal(list_item_t *l1, list_item_t *l2, int (*cmp)(void *, void *)) {
    if (((bool) l1 && (bool) l2) == false) {
        // if one is not NULL, function returns false.
        return ((bool) l1 || (bool) l2) == false;
    }

    return cmp(l1->data, l2->data) == 0 && _equal(l1->next, l2->next, cmp);
}

/**
 * @brief Equality of lists.
 *
 * @param l1 list
 * @param l2 list
 * @param cmp compare function.
 * @return bool.
 */
static bool Equal(list_t *l1, list_t *l2, int (*cmp)(void *, void *)) {
    // check for NULL, NULL case
    if ((bool) l1 && (bool) l2 == false) {
        // if one is not NULL, function returns false
        return ((bool) l1 || (bool) l2) == false;
    }

    return _equal(l1->head, l2->head, cmp);
}

/**
 * Interface to use when dealing with singly linked list.
 * Functions are in struct so we can use them in different files.
 */
const struct list_interface_t List = {
        .prepend = Prepend,
        .append = Append,
        .delete_list = Clear,
        .delete_first = Delete_first,
        .insert = Insert,
        .gethead = Gethead,
        .copy_data = NULL,
        .ctor = Ctor,
        .dtor = Dtor,
        .equal = Equal,
        .insert_after = Insert_after,
};

#ifdef SELFTEST_list

int main() {
    printf("Selfdebug: %s\n", __FILE__);

    return 0;
}

#endif
