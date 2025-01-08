// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef __SENTRY_H_
#define __SENTRY_H_

#include <inttypes.h>

uint8_t __libc_get_current_threadid(void);

int get_entropy(unsigned char *in, uint16_t len);

#endif/*__SENTRY_H_*/
