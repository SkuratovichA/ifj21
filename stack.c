#include "stack.h"


/**
 * @brief Pushes new item to the stack.
 *
 * @param stack
 * @param data Data to push to the stack.
 */
static void Push(sstack_t *stack, void *data) {
    List.prepend(stack, data);
}

/**
 * @brief Pops a top item of the stack.
 *
 * @param stack
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Pop(sstack_t *stack, void (*clear_fun)(void *)) {
    List.delete_first(stack, clear_fun);
}

/**
 * @brief Checks if the stack is empty or not.
 *
 * @param stack
 * @return true if the stack is empty else false.
 */
static bool Is_empty(sstack_t *stack) {
    soft_assert(stack, ERROR_INTERNAL);
    return !(stack->head);
}

/**
 * @brief Deletes all items in the stack.
 *
 * @param stack
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Clear(sstack_t *stack, void (*clear_fun)(void *)) {
    List.delete_list(stack, clear_fun);
}

/**
 * @brief Returns data on the top of the stack.
 *
 * @param stack
 */
static void *Peek(sstack_t *stack) {
    return List.gethead(stack);
}

/**
 * @brief Stack constructor.
 *
 * @return  Pointer to the stack.
 */
static sstack_t *Ctor(void) {
    return List.ctor();
}

/**
 * @brief Stack destructor.
 *
 * @param stack Stack to be destructed.
 * @param clear_fun pointer to a function, which will free the list data.
 */
static void Dtor(sstack_t *stack, void (*clear_fun)(void *)) {
    List.dtor(stack, clear_fun);
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

#ifdef SELFTEST_stack
#include "tests/tests.h"
#include "dynstring.h"

int main() {
    printf("Selfdebug: %s\n", __FILE__);
    const static char test_string[] = "Hello World!";
    const static size_t test_str_len = sizeof(test_string);

//        .push = Push,
//        .pop = Pop,
//        .is_empty = Is_empty,
//        .peek = Peek,
//        .ctor = Ctor,
//        .dtor = Dtor
    if (1) {
        printf("Initialize a stack:\n");
        sstack_t *st = Stack.ctor();
        if (Stack.is_empty(st)) {
            Tests.passed("New initialized stack is empty.");
        } else {
            Tests.failed("New initialized stack is not empty, but it must be. Exiting");
            goto end_test1_nostack;
        }

        for (size_t i = 0; i < test_str_len; i++) {
            char *str = calloc(1, test_str_len + 10); // ctor a new entity.
            sprintf(str, "%s%zu", test_string, i);
            printf("Pushed on stack: '%s'\n", str);
            Stack.push(st, str); // must append on stack.
        }
        if (!Stack.is_empty(st)) {
            Tests.passed("Stack is not empty.");
        } else {
            Tests.failed("Stack must not be empty here.");
            goto end_test1;
        }

        for (size_t i = 0; i < test_str_len; i++) {
            char *str = Stack.peek(st); // get head.
            printf("Peek at the head of the stack: '%s'\n", str);
            Stack.pop(st, free); // pp an item
        }
        if (Stack.is_empty(st)) {
            Tests.passed("Stack is empty.");
        } else {
            Tests.failed("Stack must be empty after popping all items.");
        }
        end_test1:
        free(st);
        if (0) {
            end_test1_nostack:
            Tests.failed("Function Stack.ctor() returned NULL.");
        }

    }
//        .clear = Clear,
//  todo
    return 0;
}

#endif
