// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef POSIX_STDLIB_H
#include POSIX_STDLIB_H

#if defined(__cplusplus)
extern "C" {
#endif

unsigned long strtoul(const char *__restrict __n, char **__restrict __end_PTR, int __base);
long strtol(const char *__restrict __n, char **__restrict __end_PTR, int __base);

#if defined(__cplusplus)
}
#endif

#endif/*!POSIX_STDLIB_H*/
