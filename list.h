#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "errors.h"
#include "debug.h"

/**
 * A structure that represents a list item.
 */

typedef struct list_item list_item_t;

/**
 * A structure that represents a singly linked list.
 */
 typedef struct list {
     list_item_t *head;
 } list_t;

/**
 * A structure that store pointers to all functions from list.c.
 * So we can use them in different files as interface.
 */
struct list_interface_t {
    /**
     * @brief List initialiser.
     *
     * @param list Singly linked list to initialise.
     */
    void (*list_init)(list_t *list);

    /**
     * @brief Insert first element to a list.
     *
     * @param list Singly linked list.
     * @param data Data to insert.
     */
    void (*insert_first)(list_t *list, void *data);

    /**
     * @brief Delete the first item in list.
     *
     * @param list singly linked list.
     */
    void (*delete_first)(list_t *list);

    /**
     * @brief Delete all items in list.
     *
     * @param list singly linked list
     */
    void (*delete_list)(list_t *list);

    /**
     * @brief Insert new item behind reference_item.
     *
     * @param reference_item Item, behind that the new item will be inserted.
     * @param data New item's data.
     */
    void (*insert_behind)(list_item_t *reference_item, void *data);

    /**
    * @brief copy data to the list item element.
    *
    * @param dst Destination item to copy the data to.
    * @param src Source to copy the data from.
    */
    void (*copy_data)(lsit_item_t *dst, void *src);
};

/**
 * Functions from list.c will be visible in different files under List name.
 */
extern const struct list_interface_t List;
