// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef STRING_H
#define STRING_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif
/**
 * \file this header declare libc compatible (API behavior) functions
 * meaning that if a library is using libc6 <string.h>, symbol resolution
 * using this very symbols instead will not arm the execution, as they behave
 * in a fully conform way to the POSIX.1-2001 reference implementation.
 */

size_t ssol_strlen(const char *s);

char *ssol_strcpy(char *dest, const char *src);

int ssol_strcmp(const char *str1, const char *str2);

#ifndef TEST_MODE
/* aliasing */
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *str1, const char *str2);
#endif

#if defined(__cplusplus)
}
#endif

#endif/*!STRING_H*/
