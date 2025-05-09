// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef STRING_H
#define STRING_H

#include <inttypes.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \file this header declare libc compatible (API behavior) functions
 * meaning that if a library is using libc6 <string.h>, symbol resolution
 * using this very symbols instead will not arm the execution, as they behave
 * in a fully conform way to the POSIX.1-2001 reference implementation.
 */
#ifndef TEST_MODE
/* using alias directly */
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t len);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
int strcmp(const char *str1, const char *str2);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
#else
/* no aliasing */
size_t shield_strlen(const char *s);
size_t shield_strnlen(const char *s, size_t len)
char *shield_strcpy(char *dest, const char *src);
int shield_strcmp(const char *str1, const char *str2);

void *shield_memcpy(void *dest, const void *src, size_t n);
#endif

#if defined(__cplusplus)
}
#endif

#endif/*!STRING_H*/
