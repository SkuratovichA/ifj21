/**
 * @file list.c
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Lucie Svobodova <xsvobo1x@vutbr.cz>
 */
#include "list.h"


/**
 * @brief List constructor.
 *
 * @return Pointer to the allocated memory.
 */
static list_t *Ctor(void) {
    list_t *l = calloc(1, sizeof(list_t));
    soft_assert(l != NULL, ERROR_INTERNAL);
    return l;
}

/**
 * @brief Insert first element to a list.
 *
 * @param list Singly linked list.
 * @param data Data to insert.
 */
static void Prepend(list_t *list, void *data) {
    soft_assert(list != NULL, ERROR_INTERNAL);

    list_item_t *new_item = calloc(1, sizeof(list_item_t));
    soft_assert(new_item != NULL, ERROR_INTERNAL);

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
    soft_assert(list != NULL, ERROR_INTERNAL);

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
    soft_assert(item != NULL, ERROR_INTERNAL);

    list_item_t *new_item = calloc(1, sizeof(list_item_t));
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
    if (list == NULL) {
        return;
    }
    soft_assert(clear_fun != NULL, ERROR_INTERNAL);

    list_item_t *tmp = list->head;
    if (tmp == NULL) {
        return;
    }

    list->head = list->head->next;
    clear_fun(tmp->data);
    free(tmp);
}

static void Print_list(list_t *list, char *(*pp_fun)(void *)) {
    if (list == NULL) {
        return;
    }
    soft_assert(pp_fun != NULL, ERROR_INTERNAL);

    list_item_t *iter = list->head;

    while (iter != NULL) {
        printf("%s\n", pp_fun(iter->data));
        iter = iter->next;
    }
}

/**
 * @brief Delete all items in list.
 *
 * @param list singly linked list
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Clear(list_t *list, void (*clear_fun)(void *)) {
    if (list == NULL) {
        return;
    }
    soft_assert(clear_fun != NULL, ERROR_INTERNAL);

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
    if (list == NULL) {
        return;
    }
    soft_assert(clear_fun != NULL, ERROR_INTERNAL);
    
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
static void *Get_head(list_t *list) {
    soft_assert(list != NULL, ERROR_INTERNAL);

    if (!list->head) {
        return NULL;
    }
    return list->head->data;
}

/**
 * @brief Returns the last item's data via data pointer.
 *
 * @param list singly linked list
 */
static void *Get_tail(list_t *list) {
    soft_assert(list != NULL, ERROR_INTERNAL);

    if (!list->tail) {
        return NULL;
    }
    return list->tail->data;
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

/*
 * @brief Reverses the list.
 * @param l singly linked list to be reversed
 */
static void Reverse(list_t *l) {
    list_item_t *current = l->head;
    list_item_t *prev = NULL;
    list_item_t *next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    l->head = prev;
}

/*
 * @brief Finds the length of the list.
 * @param l singly linked list
 */
static size_t Len(list_t *l) {
    size_t len = 0;
    list_item_t *current = l->head;
    while (current != NULL) {
        len++;
        current = current->next;
    }
    return len;
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
        .get_head = Get_head,
        .get_tail = Get_tail,
        .copy_data = NULL,
        .ctor = Ctor,
        .dtor = Dtor,
        .equal = Equal,
        .reverse = Reverse,
        .len = Len,
        .insert_after = Insert_after,
        .print_list = Print_list,
};

#ifdef SELFTEST_list

int main() {
    printf("Selfdebug: %s\n", __FILE__);

    return 0;
}

#endif
