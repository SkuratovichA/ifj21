/**
 * @file dynstring.c
 *
 * @brief Dynamic string implementation.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */

#include "dynstring.h"
#include <stdlib.h>
#include "errors.h"
#include <stdio.h>


#define STRSIZE 42


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
    return str;
}

/**
 * @brief Create an empty dynstring_t of size length.
 *
 * @param s Length of the new dynstring.
 * @return non-null pointer to dynstring_t object.
 */
static dynstring_t *Str_ctor_empty(size_t length) {
    size_t alloc = length + STRSIZE + 1;

    dynstring_t *str = calloc(1, sizeof(dynstring_t));
    soft_assert(str, ERROR_INTERNAL);
    str->size = alloc;
    str->len = length;

    str->str = calloc(1, alloc);
    soft_assert(str->str, ERROR_INTERNAL);

    // TODO check bounds (length/length+1/length-1)
    memset(str->str, '\0', length);

    return str;
}

/**
* @brief Get char * (c dynstring_t ending with '\0') from dynstring_t.
*
* @param str dynstring_t object.
* @return c dynstring_t representation.
*/
static char *Str_c_str(dynstring_t *str) {
    if (str == NULL) {
        return NULL;
    }
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
    soft_assert(str != NULL, ERROR_INTERNAL);
    soft_assert(str->str != NULL, ERROR_INTERNAL);
    str->str[0] = '\0';
    str->len = 0;
}

/**
 * @brief Frees memory.
 *
 * @param str dynstring_t to dtor.
 */
static void Str_free(dynstring_t *str) {

    if (str != NULL) {
        if (str->str != NULL) {
            free(str->str);
        }
        free(str);
    }
}

/**
 * @brief Appends a character shrunk.
 *
 * @param str dynstring_t heap structure.
 * @param ch char to append.
 */
static void Str_append(dynstring_t *str, char ch) {
    soft_assert(str != NULL, ERROR_INTERNAL);
    soft_assert(str->str != NULL, ERROR_INTERNAL);

    if (str->len + 1 >= str->size) {
        size_t nsiz = str->size *= 2;
        char *tmp = realloc(str->str, nsiz + sizeof(dynstring_t));
        soft_assert(tmp, ERROR_INTERNAL);
        str->str = tmp;
    }
    str->str[str->len++] = ch;
    str->str[str->len] = '\0';
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
    soft_assert(s2->str != NULL, ERROR_INTERNAL);
    soft_assert(s1, ERROR_INTERNAL);
    soft_assert(s1->str != NULL, ERROR_INTERNAL);

    return strcmp(s1->str, s2->str);
}

/**
 * @brief Concatenate two dynstrings, save the result to s1.
 *
 * @param s1 dynstring_t object.
 * @param s2 dynstring_t object.
 * @returns new dynstring, which is product of s1 and s2.
 */
static void Str_cat(dynstring_t *s1, dynstring_t *s2) {
    soft_assert(s2 != NULL, ERROR_INTERNAL);
    soft_assert(s2->str != NULL, ERROR_INTERNAL);
    soft_assert(s1 != NULL, ERROR_INTERNAL);
    soft_assert(s1->str != NULL, ERROR_INTERNAL);

    bool needs_realloc = s1->size <= s1->len + s2->len;
    if (needs_realloc) {
        s1->size += s2->size;
        s1->str = realloc(s1->str, s1->size + sizeof(dynstring_t));
        soft_assert(s1->str, ERROR_INTERNAL);
    }

    strcat(s1->str, s2->str);
    s1->len = strlen(s1->str);
}

static void Trunc_to_len(dynstring_t *self, size_t new_len) {
    soft_assert(self != NULL, ERROR_INTERNAL);
    soft_assert(new_len <= self->len, ERROR_INTERNAL);
    self->len = new_len;
    self->str[self->len < self->size - 1 ? self->len : self->size - 1] = '\0';
}

/**
 * @brief Duplicates a dynstring.
 *
 * @param s dynstring to be duplicated.
 * @return pointer to the new dynstring_t object.
 */
static dynstring_t *Str_dup(dynstring_t *s) {
    if (s == NULL) {
        return Str_ctor("");
    }
    soft_assert(s->str != NULL, ERROR_INTERNAL);

    return Str_ctor(s->str);
}

static int Cmp_c_str(dynstring_t *s1, char *s2) {
    if (s2 == NULL) {
        return 1;
    }

    return strcmp(Dynstring.c_str(s1), s2);
}

/**
 * Interface to use when dealing with dynstrings.
 * Functions are in struct so we can use them in different files.
 */
const struct dynstring_interface_t Dynstring = {
        /*@{*/
        .ctor = Str_ctor,
        .ctor_empty = Str_ctor_empty,
        .len = Str_length,
        .c_str = Str_c_str,
        .append = Str_append,
        .dtor = Str_free,
        .cmp = Str_cmp,
        .cat = Str_cat,
        .dup = Str_dup,
        .clear = Str_clear,
        .trunc_to_len = Trunc_to_len,
        .cmp_c_str = Cmp_c_str,
};

#ifdef SELFTEST_dynstring
#include "tests/tests.h"
int main() {
    fprintf(stderr, "Selftests: %s\n", __FILE__);
    dynstring_t *string1 = Dynstring.ctor("");
    dynstring_t *string2 = Dynstring.ctor("hello");
    dynstring_t *string3 = Dynstring.ctor("cat");

    Dynstring.cat(string1, string2);
    Dynstring.clear(string1);

    printf("string1 = '%s'. Must be empty\n", Dynstring.c_str(string1));
    Dynstring.clear(string1);
    Dynstring.clear(string2);


    Dynstring.cat(string2, string3);
    Dynstring.cat(string1, string2);

    printf("string1 = '%s'. Must be cat\n", Dynstring.c_str(string1));
    Dynstring.clear(string1);
    Dynstring.clear(string2);


    Dynstring.cat(string1, string3);
    Dynstring.cat(string1, string1);

    printf("string1 = '%s'. Must be catcat\n", Dynstring.c_str(string1));


    Dynstring.trunc_to_len(string1, 0);
    printf("string1 = '%s'. Must be catcat\n", Dynstring.c_str(string1));

    Dynstring.dtor(string1);
    Dynstring.dtor(string2);
    Dynstring.dtor(string3);

    return 0;
}
#endif
