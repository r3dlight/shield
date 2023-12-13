// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef PTHREAD_H_
#define PTHREAD_H_

#include <inttypes.h>

/* thread identifier definition */
typedef uint32_t pthread_t;

pthread_t pthread_self(void);

#endif/*PTHREAD_H_*/
