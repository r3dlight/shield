// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef SHIELD_STDIO_H
#define SHIELD_STDIO_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

__attribute__ ((format (printf, 1, 2))) int printf(const char *fmt, ...);
__attribute__ ((format (printf, 3, 4))) int snprintf(char *dest, size_t len, const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif/*!SHIELD_STDIO_H*/
