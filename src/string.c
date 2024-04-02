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

#include <stdbool.h>
#include <shield/string.h>
#include <shield/errno.h>

#include <shield/private/errno.h>
#include <shield/private/coreutils.h>

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
size_t shield_strnlen(const char *s, size_t maxlen)
{
    size_t result = 0;

    if (s == NULL) {
        /** TODO: panic to be called */
        goto err;
    }

    while (s[result] != '\0' && result < maxlen) {
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

/**
 * TODO: way to allow concat
 * Here we only concat on place, whish is an UB by default.
 * This implementation is required by LVGL for some widgets.
 */
#ifndef TEST_MODE
static
#endif
char *shield_strcat(char *dest, const char* src)
{
    /* concat on place with trailing \0 */
    memcpy(&dest[strlen(dest)], src, strlen(src) + 1);
    return dest;
}

static inline void *_aligned_memcpy(void*dest, const void*src, size_t n)
{
    union memarea {
        void*     bin_p;
        uint32_t* word_p;
        uint8_t*  byte_p;
    };
    union memarea u_dest, u_src;
    u_dest.bin_p = dest;
    /*
     * INFO: the const is discarded to avoid duplicating the union type, yet src is NOT
     * modified at any time in this function
     */
    u_src.bin_p = (void*)src;
    size_t offset;
    /* world aligned copy first */
    for (offset = 0; offset < (n - (n % sizeof(uint32_t))); offset += sizeof(uint32_t)) {
        *u_dest.word_p = *u_src.word_p;
        ++u_src.word_p;
        ++u_dest.word_p;
    }
    /* handle residual */
    if (offset < n) {
        for (uint8_t i = 0; i < (n - offset); ++i) {
            *u_dest.byte_p = *u_src.byte_p;
            ++u_src.byte_p;
            ++u_dest.byte_p;
        }
    }
    /* TODO: add framaC ensures for src range content: 'new == 'old */
    return dest;
}

/**
 * We may calculate a PPCM mechanism to check if we can realign first, and then align-copy
 * (e.g. if both are unaligned in the same way), but it is probably a waste of time...
 */
static inline void *_unaligned_memcpy(void*dest, const void*src, size_t n)
{
    uint8_t *u8_dest = dest;
    const uint8_t *u8_src = src;
    size_t offset;
    /* byte copy, avoiding any multi-world writting */
    for (size_t offset = 0; offset < n; offset += sizeof(char)) {
        *u8_dest = *u8_src;
        ++u8_src;
        ++u8_dest;
    }
    return dest;
}

static bool _memarea_do_overlap(const void * mem_a_p, const void *mem_b_p, size_t n)
{
    bool result = true;
    /* do not use pointer algorithmic but N */
    size_t mem_a = (size_t)mem_a_p;
    size_t mem_b = (size_t)mem_b_p;
    /* using a clear to read algorithm, leaving optimisation to compile time */
    if ((mem_a > mem_b) && (mem_b + n > mem_a)) {
        goto end;
    }
    if ((mem_b > mem_a) && (mem_a + n > mem_b)) {
        goto end;
    }
    result = false;
end:
    return result;
}

void *shield_memcpy(void* dest, const void* src, size_t n)
{
    void* result = dest;
    if (unlikely((dest == NULL) || (src == NULL))) {
        __shield_set_errno(EINVAL);
        goto end;
    }
    if (unlikely(_memarea_do_overlap(dest, src, n))) {
        __shield_set_errno(EINVAL);
        goto end;
    }
    if (likely(_memarea_is_worldaligned(src) && _memarea_is_worldaligned(dest))) {
        result = _aligned_memcpy(dest, src, n);
    } else {
        result = _unaligned_memcpy(dest, src, n);
    }
end:
    return result;
}

#ifndef TEST_MODE
/* if not in the test suite case, aliasing to POSIX symbols */
size_t strlen(const char *s) __attribute__((alias("shield_strlen")));
size_t strnlen(const char *s, size_t len) __attribute__((alias("shield_strnlen")));
char *strcpy(char *dest, const char *src) __attribute__((alias("shield_strcpy")));
char *strcat(char *dest, const char *src) __attribute__((alias("shield_strcat")));
int strcmp(const char *str1, const char *str2) __attribute__((alias("shield_strcmp")));
void *memcpy(void* dest, const void* src, size_t n) __attribute__((alias("shield_memcpy")));
#endif
