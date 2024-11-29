// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef POSIX_STDLIB_H
#define POSIX_STDLIB_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

inline int abs(int j) { return __builtin_abs(j); }
inline long labs(long j) { return __builtin_labs(j); }
inline long long llabs(long long j) { return __builtin_llabs(j); }
inline intmax_t imaxabs(intmax_t j) { return __builtin_imaxabs(j); }

unsigned long strtoul(const char *__restrict __n, char **__restrict __end_PTR, int __base);
long strtol(const char *__restrict __n, char **__restrict __end_PTR, int __base);

#if defined(__cplusplus)
}
#endif

#endif/*!POSIX_STDLIB_H*/
