#include "dynstring.h"
#include <stdlib.h>
#include "errors.h"
#include <stdio.h>


#define STRSIZE 42

#ifndef DEBUG_dynstring
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"
// undef debug macros
#define debug_err(...)
#define debug_msg(...)
#define debug_msg_stdout(...)
#define debug_msg_stderr(...)
#define debug_todo(...)
#define debug_assert(cond)
#define debug_msg_s(...)
#define DEBUG_SEP
#pragma GCC diagnostic pop
#endif

/**
 * A structure represented dynstring_t
 */
typedef struct dynstring {
    size_t size;    /**< Allocated len on heap. */
    size_t len;              /**< String length. */
    char *str;              /**< String. */
} dynstring_t;

/**
 * @brief Create a dynstring_t from a c_string, with len(@param str) + STRSIZE
 *
 * @param s static c dynstring_t.
 * @param s Char that to convert to dynstring_t.
 * @return pointer to the dynstring_t object.
 */
static dynstring_t *Str_ctor(const char *s) {
    soft_assert(s, ERROR_INTERNAL); // dont deal with NULLptr

    size_t length = strlen(s);
    size_t alloc = length + STRSIZE + 1;

    dynstring_t *str = calloc(1, sizeof(dynstring_t));
    soft_assert(str, ERROR_INTERNAL);
    str->size = alloc;
    str->len = length;

    str->str = calloc(1, alloc);
    soft_assert(str->str, ERROR_INTERNAL);
    strcpy(str->str, s);
    debug_msg("Create dynstring: { .len = %zu .size = %zu .str = '%s'\n", str->len, str->size, str->str);
    return str;
}

/**
 * @brief Get char * (c dynstring_t ending with '\0') from dynstring_t.
 *
 * @param str dynstring_t object.
 * @return c dynstring_t representation.
 */
static char *Str_c_str(dynstring_t *str) {
    soft_assert(str, ERROR_INTERNAL);
    return str->str;
}

/**
 * @brief Return len of given dynstring_t.
 *
 * @param str dynstring_t object.
 * @return len of dynstring_t
 */
static size_t Str_length(dynstring_t *str) {
    soft_assert(str, ERROR_INTERNAL);
    return str->len;
}

/**
* @brief Clears the dynstring. Set everything to 0 except size.
*
* @param str string to clear.
*/
static void Str_clear(dynstring_t *str) {
    soft_assert(str, ERROR_INTERNAL);
    str->str[0] = '\0';
    str->len = 0;
}

/**
 * @brief Frees memory.
 *
 * @param str dynstring_t to dtor.
 */
static void Str_free(dynstring_t *str) {
    soft_assert(str, ERROR_INTERNAL);
    free(str->str);
    free(str);
}

/**
 * @brief Appends a character shrunk.
 *
 * @param str dynstring_t heap structure.
 * @param ch char to append.
 */
static void Str_append(dynstring_t *str, char ch) {
    soft_assert(str, ERROR_INTERNAL);
    if (str->len + 1 >= str->size) {
        size_t nsiz = str->size *= 2;
        char *tmp = realloc(str->str, nsiz + sizeof(dynstring_t));
        soft_assert(tmp, ERROR_INTERNAL);
        str->str = tmp;
    }
    str->str[str->len++] = ch;
    str->str[str->len] = '\0';
//    debug_msg("Append char: { .len = %zu .size = %zu .str = '%s'\n", str->len, str->size, str->str);
}

/**
 * @brief Compare dynstring_t and char* using strcmp.
 *
 * @param s1 dynstring_t object.
 * @param s2 dynstring_t object.
 * @returns -1, 0, 1 depends on lexicographical ordering of two strings.
 */
static int Str_cmp(dynstring_t *s1, dynstring_t *s2) {
    soft_assert(s2, ERROR_INTERNAL);
    soft_assert(s1, ERROR_INTERNAL);
    debug_msg("compare %s with %s gives %d\n", s1->str, s2->str, strcmp(s1->str, s2->str));

    return strcmp(s1->str, s2->str);
}

/**
 * @brief Concatenate two dynstrings in the not very efficient way.
 *
 * @param s1 dynstring_t object.
 * @param s2 dynstring_t object.
 * @returns new dysntring, which is product of s1 and s2.
 */
static dynstring_t *Str_cat(dynstring_t *s1, dynstring_t *s2) {
    soft_assert(s2, ERROR_INTERNAL);
    soft_assert(s1, ERROR_INTERNAL);

    dynstring_t *new = Str_ctor(s1->str);

    size_t diff = new->size - s1->len;
    if (diff <= 1) {
        size_t nsiz = new->size *= 2;
        char *tmp = realloc(new->str, nsiz + sizeof(dynstring_t));
        soft_assert(tmp, ERROR_INTERNAL);
        new->str = tmp;
    }
    strcat(new->str, s1->str);

    return new;
}

/**
 * Interface to use when dealing with dynstrings.
 * Functions are in struct so we can use them in different files.
 */
const struct dynstring_interface_t Dynstring = {
        /*@{*/
        .ctor = Str_ctor,
        .len = Str_length,
        .c_str = Str_c_str,
        .append = Str_append,
        .dtor = Str_free,
        .cmp = Str_cmp,
        .cat = Str_cat,
};

#ifdef SELFTEST_dynstring
#include "tests/tests.h"
int main() {
    fprintf(stderr, "Selftests: %s\n", __FILE__);
    if (SELFTEST_dynstring) {
        dynstring_t *hello = Dynstring.ctor("hello, ");
        debug_msg("hello string created with string '%s'\n", Dynstring.c_str(hello));
        dynstring_t *world = Dynstring.ctor("World. Aaaaaaaaaaaaaaaaaaaaaaaaa");
        debug_msg("world string created with string '%s'\n", Dynstring.c_str(world));
        dynstring_t *hw = Dynstring.cat(hello, world);
        debug_msg("The 2 strings concatenated into '%s'\nsize: %zu, len: %zu\n", Dynstring.c_str(hw), hw->size, hw->len);
        TEST_EXPECT(hw->size != 0, true, "Allocated size must not be equal 0");

        for (int i = 0; i < 1000; i++) {
             Dynstring.append(hw, (char)('A' + (i % 25)));
        }
        TEST_EXPECT(hw->size != 0, true, "After appending 1000 characters in hw. MUst not be equal 0");

        Dynstring.dtor(hello);
        Dynstring.dtor(world);
        Dynstring.dtor(hw);
    }
    return 0;
}
#endif
