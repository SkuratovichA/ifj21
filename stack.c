#include "stack.h"


/**
 * @brief Pushes new item to the stack.
 *
 * @param stack
 * @param data Data to push to the stack.
 */
static void Push(list_t *stack, void *data) {
    List.prepend(stack, data);
}

/**
 * @brief Pops a top item of the stack.
 *
 * @param stack
 */
static void Pop(list_t *stack) {
    List.delete_first(stack);
}

/**
 * @brief Checks if the stack is empty or not.
 *
 * @param stack
 * @return true if the stack is empty else false.
 */
static bool Is_empty(list_t *stack) {
    soft_assert(stack, ERROR_INTERNAL);
    return !(stack->head);
}

/**
 * @brief Deletes all items in the stack.
 *
 * @param stack
 */
static void Clear(list_t *stack) {
    List.delete_list(stack);
}

/**
 * @brief Returns data on the top of the stack.
 *
 * @param stack
 */
static void *Peek(list_t *stack) {
    return List.gethead(stack);
}

/**
 * @brief Stack constructor.
 *
 * @return  Pointer to the stack.
 */
static list_t *Ctor(void) {
    return List.ctor();
}

/**
 * @brief Stack destructor.
 *
 * @param stack Stack to be destructed.
 */
static void Dtor(list_t *stack) {
    List.dtor(stack);
}

/**
 * Interface to use when dealing with stack.
 * Functions are in struct so we can use them in different files.
 */
const struct stack_interface_t Stack = {
        .push = Push,
        .pop = Pop,
        .is_empty = Is_empty,
        .clear = Clear,
        .peek = Peek,
        .ctor = Ctor,
        .dtor = Dtor
};

#ifdef SELFTEST_STACK
#define MAIN main
#else
#define MAIN STACK_MAIN
#endif

int MAIN() {
    printf("Selfdebug: %s\n", __FILE__);

    return 0;
}
