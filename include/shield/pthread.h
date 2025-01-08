// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef PTHREAD_H_
#define PTHREAD_H_

#include <inttypes.h>

/* thread identifier definition */
typedef uint32_t pthread_t;

pthread_t pthread_self(void);

#endif/*PTHREAD_H_*/
