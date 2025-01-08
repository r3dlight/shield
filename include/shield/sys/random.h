// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef SHIELD_SYS_RANDOM_H_
#define SHIELD_SYS_RANDOM_H_

#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Linux compatible getrandom API
 *
 * This API is not POSIX but Linux specific
 */
ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);

#endif/*!SHIELD_SYS_RANDOM_H_*/
