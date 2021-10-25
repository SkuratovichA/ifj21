#pragma once

#include <stdlib.h>
#include <stdio.h>
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
     * @brief Insert first element to a list.
     *
     * @param list Singly linked list.
     * @param data Data to insert.
     */
    void (*prepend)(list_t *list, void *data);

    /**
     * @brief Delete the first item in list.
     *
     * @param list singly linked list.
     * @param clear_fun pointer to a function, which will free the list data.
     */
    void (*delete_first)(list_t *list, void (*clear_fun)(void *));

    /**
     * @brief Delete all items in list.
     *
     * @param list singly linked list
     * @param clear_fun pointer to a function, which will free the list data.
     */
    void (*delete_list)(list_t *list, void (*clear_fun)(void *));

    /**
     * @brief Insert new item behind reference_item.
     *
     * @param reference_item Item, behind that the new item will be inserted.
     * @param data New item's data.
     */
    void (*insert)(list_item_t *reference_item, void *data);

    /**
     * @brief Returns data in the first item in list.
     *
     * @param list singly linked list
     */
    void *(*gethead)(list_t *list);

    /**
     * @brief copy data to the list item element.
     *
     * @param dst Destination item to copy the data to.
     * @param src Source to copy the data from.
     */
    void (*copy_data)(list_item_t *dst, void *src);

    /**
    * @brief List constructor.
    *
    * @return Pointer to the allocated memory.
    */
    list_t *(*ctor)(void);

    /**
    * @brief List destructor.
    *
    * @param list the list to be destructed.
     * @param clear_fun pointer to a function, which will free the list data.
    */
    void (*dtor)(list_t *list, void (*clear_fun)(void *));
};

/**
 * Functions from list.c will be visible in different files under List name.
 */
extern const struct list_interface_t List;
