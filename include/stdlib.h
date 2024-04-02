// SPDX-FileCopyrightText: 2023-2024 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef SHIELD_STDLIB_H_
#define SHIELD_STDLIB_H_

/**
 * 31bits settable max random value, allowing the
 * POSIX int type (32bits length at least)
 */
#define	RAND_MAX	((1 << 30) - 1)

int rand(void);

int rand_r(unsigned int *seedp);

void srand(unsigned int seed);


#endif/*!SHIELD_STDLIB_H_*/
