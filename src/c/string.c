// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

/**
 * @file
 *
 * basic implementation of standard (yet unsecure) string manipulation.
 * These functions should not be used by hardened modules, although some OSS
 * require these symbols to exist.
 * In TEST_MODE, the POSIX symbols are not exported and prefixed with shield_
 * prefix, in order to help in the unit testing part.
 * in nominal build, prefixed symbols are local to this file and only aliases are
 * exported
 */

#include <stddef.h>
#include <shield/string.h>

/**
 * \brief standard (and thus unsecure) strlen implementation
 *
 * This implementation does respect the standard C API
 * conformity: POSIX.1-2001, POSIX.1-2008, C89, C99, C11, SVr4, 4.3BSD.
 *
 * INFO: strlen() is, by definition, unsecure (i.e. it does not handle input NULL pointer),
 * and is not able, in its standard behavior, to return other informations that the
 * number of chars of an effectively allocated input string.
 */
#ifndef TEST_MODE
static
#endif
size_t shield_strlen(const char *s)
{
    size_t result = 0;

    if (s == NULL) {
        /** TODO: panic to be called */
        goto err;
    }

    while (s[result] != '\0') {
        result++;
    }
err:
    return result;
}

#ifndef TEST_MODE
static
#endif
char *shield_strcpy(char *dest, const char *src)
{
    if (src == NULL || dest == NULL) {
        goto end;
    }
    /* INFO: no overlapping check here */
    size_t to_copy = shield_strlen(src);
    if (dest < src) {
        if (((size_t)src - (size_t)dest) < (to_copy + 1)) {
            /* overlapping here */
            goto end;
        }
    } else {
        if (((size_t)dest - (size_t)src) < (to_copy + 1)) {
            /* overlapping here */
            goto end;
        }
    }

    for (size_t i = 0; i < to_copy; ++i) {
        dest[i] = src[i];
    }
    dest[to_copy] = '\0';

end:
    return dest;
}

/**
 * \brief secure, constant-time implementation of strcmp
 *
 * This implementation does respect the standard C API
 * conformity: POSIX.1-2001, POSIX.1-2008, C89, C99, SVr4, 4.3BSD
 *
 * INFO: No double loop index is added as strcmp is not considered to be used in
 * **very** secure fault resistant code. Can be updated later.
 */
#ifndef TEST_MODE
static
#endif
int shield_strcmp(const char *str1, const char *str2)
{
    int result = -1;
    if (str1 == NULL || str2 == NULL) {
        goto err;
    }

    size_t len1 = shield_strlen(str1);
    size_t len2 = shield_strlen(str2);

    size_t max_len = (len1 > len2) ? len1 : len2;

    result = 0;
    for (size_t i = 0; i < max_len; i++) {
        unsigned char c1 = (i < len1) ? (unsigned char)str1[i] : 0;
        unsigned char c2 = (i < len2) ? (unsigned char)str2[i] : 0;

        result = (c1 > c2) - (c1 < c2);  /* Constant-time comparison on boolean calculation */

        if (result || c1 == '\0' || c2 == '\0')
            break;  // Reached end of one or both strings
    }
err:
    return result;
}

#ifndef TEST_MODE
/* if not in the test suite case, aliasing to POSIX symbols */
size_t strlen(const char *s) __attribute__((alias("shield_strlen")));
char *strcpy(char *dest, const char *src) __attribute__((alias("shield_strcpy")));
int strcmp(const char *str1, const char *str2) __attribute__((alias("shield_strcmp")));
#endif
