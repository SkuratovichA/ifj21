#include "stack.h"

/**
 * @brief Stack initializer.
 *
 * @param stack Stack to initialise.
 */
static void stack_init(list_t *stack) {
    List.list_init(stack);
}

/**
 * @brief Pushes new item to the stack.
 *
 * @param stack
 * @param data Data to push to the stack.
 */
static void push(list_t *stack, void *data) {
    List.insert_first(stack, data);
}

/**
 * @brief Pops a top item of the stack.
 *
 * @param stack
 */
static void pop(list_t *stack) {
    List.delete_first(stack);
}

/**
 * @brief Checks if the stack is empty or not.
 *
 * @param stack
 * @return true if the stack is empty else false.
 */
static bool is_empty(list_t *stack) {
    return !(stack->head);
}

/**
 * @brief Deletes all items in the stack.
 *
 * @param stack
 */
static void empty_stack(list_t *stack) {
    List.delete_list(stack);
}

/**
 * @brief Returns data on the top of the stack.
 *
 * @param stack
 */
static void *peek(list_t *stack) {
    return List.read_first(stack);
}

/**
 * Interface to use when dealing with stack.
 * Functions are in struct so we can use them in different files.
 */
const struct stack_interface_t Stack = {
        .stack_init = stack_init,
        .push = push,
        .pop = pop,
        .is_empty = is_empty,
        .empty_stack = empty_stack,
        .peek = peek
};
