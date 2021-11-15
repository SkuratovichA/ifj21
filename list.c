/**
 * @file list.c
 *
 * @brief
 *
 * @author Svobodova Lucie
 */
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
        .delete_list = Clear,
        .delete_first = Delete_first,
        .insert = Insert,
        .gethead = Gethead,
        .copy_data = NULL,
        .ctor = Ctor,
        .dtor = Dtor,
        .equal = Equal,
};


/********************************* DOUBLY LINKED LIST *********************************/
/**
 * Doubly linked list item struct.
 */
struct dll_list_item {
    void *data;
    dll_list_item_t *next;
    dll_list_item_t *prev;
};

/**
 * @brief Doubly linked list constructor.
 *
 * @return Pointer to the allocated memory.
 */
static dll_list_t *DLL_Ctor(void) {
    return calloc(1, sizeof(dll_list_t));
}

/**
 * @brief Insert first element to a list.
 *
 * @param list doubly linked list.
 * @param data data to insert.
 */
static void DLL_Prepend(dll_list_t *list, void *data) {
    dll_list_item_t *new_item = calloc(1, sizeof(dll_list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    /*if (DLList.copy_data != NULL) {
        DLList.copy_data(new_item, data);
    } else {
        new_item->data = data;
    }
     */
    new_item->data = data;
    new_item->next = list->head;
    new_item->prev = NULL;
    if (list->head != NULL) {
        list->head->prev = new_item;
    } else {
        list->tail = new_item;
    }
    list->head = new_item;
}

/**
 * @brief Insert new element behind the last element in the list.
 *
 * @param list doubly linked list.
 * @param data data to insert.
 */
static void DLL_Append(dll_list_t *list, void *data) {
    dll_list_item_t *new_item = calloc(1, sizeof(dll_list_item_t));
    soft_assert(new_item, ERROR_INTERNAL);

    if (list->head == NULL) {
        list->head = new_item;
        list->tail = list->head;
    } else {
        list->tail->next = new_item;
        list->tail = new_item;
    }
    new_item->next = NULL;
}

/**
 * @brief Delete the first item in list.
 *
 * @param list doubly linked list.
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void DLL_Delete_first(dll_list_t *list, void (*clear_fun)(void *)) {
    soft_assert(list, ERROR_INTERNAL);
    dll_list_item_t *tmp = list->head;
    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->next;
        list->head->prev = NULL;
    }
    clear_fun(tmp->data);
    free(tmp);
}

/**
 * @brief Delete all items in list.
 *
 * @param list doubly linked list
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void DLL_Clear(dll_list_t *list, void (*clear_fun)(void *)) {
    while (list->head) {
        DLL_Delete_first(list, clear_fun);
    }
}

/**
 * @brief List destructor.
 *
 * @param list List to be destructed.
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void DLL_Dtor(dll_list_t *list, void (*clear_fun)(void *)) {
    DLL_Clear(list, clear_fun);
    free(list);
}

/**
 * @brief Returns the first item's data via data pointer.
 *
 * @param list doubly linked list
 */
static void *DLL_Gethead(dll_list_t *list) {
    if (!list->head) {
        return NULL;
    }
    return list->head->data;
}

/**
 * Interface to use when dealing with doubly linked list.
 * Functions are in struct so we can use them in different files.
 */
const struct dll_list_interface_t DLList = {
        .prepend = DLL_Prepend,
        .append = DLL_Append,
        .delete_list = DLL_Clear,
        .delete_first = DLL_Delete_first,
        .gethead = DLL_Gethead,
        .ctor = DLL_Ctor,
        .dtor = DLL_Dtor,
};



#ifdef SELFTEST_list

int main() {
    printf("Selfdebug: %s\n", __FILE__);

    return 0;
}

#endif
