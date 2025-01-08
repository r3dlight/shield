// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef POSIX_STDLIB_H
#define POSIX_STDLIB_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

int abs(int j);
long labs(long j);
long long llabs(long long j);
intmax_t imaxabs(intmax_t j);

unsigned long strtoul(const char *__restrict __n, char **__restrict __end_PTR, int __base);
long strtol(const char *__restrict __n, char **__restrict __end_PTR, int __base);

#if defined(__cplusplus)
}
#endif

#endif/*!POSIX_STDLIB_H*/
