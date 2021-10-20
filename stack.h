#pragma once

#include "list.h"

/**
 * A structure that store pointers to all functions from stack.c.
 * So we can use them in different files as interface.
 */
struct stack_interface_t {

    /**
     * @brief Pushes new item to the stack.
     *
     * @param stack
     * @param data Data to push to the stack.
     */
    void (*push)(list_t *stack, void *data);

    /**
     * @brief Pops a top item of the stack.
     *
     * @param stack
     */
    void (*pop)(list_t *stack);

    /**
     * @brief Checks if the stack is empty or not.
     *
     * @param stack
     * @return true if the stack is empty else false.
     */
    bool (*is_empty)(list_t *stack);

    /**
     * @brief Deletes all items in the stack.
     *
     * @param stack
     */
    void (*clear)(list_t *stack);

    /**
     * @brief Returns data on the top of the stack.
     *
     * @param stack
     */
    void *(*peek)(list_t *stack);

    /**
     * @brief Stack constructor.
     *
     * @return  Pointer to the stack.
     */
    list_t *(*ctor)(void);

    /**
     * @brief Stack destructor.
     *
     * @param stack Stack to be destructed.
     */
    void (*dtor)(list_t *stack);
};

// Dont change. On apple macos it doesnt wrk because there's already a stack structure.
typedef list_t sstack_t;

/**
 * Functions from stack.c will be visible in different files under Stack name.
 */
extern const struct stack_interface_t Stack;
