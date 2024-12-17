// SPDX-FileCopyrightText: 2023-2024 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <shield/private/coreutils.h>
#include <shield/private/errno.h>
#include <errno.h>
#include "support/sentry.h"

/***** "Unsecure" ISO C random using a linear congruential generator (LGC) *****/
/* WARNING: this egenrator is *NOT* cryptographically secure!! */
/*
 * We use GLIBC's parameters:
 *    modulus m = 2**31
 *    multiplier a = 1103515245
 *    increment c = 12345
 *
 */
static unsigned int rand_seed = 1; /* seed default value is 1 */

/**
 * Initial seed affectation, based on SSP seed value (derivated)
 */
void __shield_rand_set_seed(uint32_t value)
{
    rand_seed = value;
}

/**
 * @brief GNU libC compliant POSIX random calculation
 */
int shield_rand(void)
{
        rand_seed = (rand_seed * 1103515245) + 12345;
        return (int)((unsigned int)(rand_seed/65536) % (RAND_MAX + 1));
}

/**
 * @brief POSIX compliant BSD srand()
 */
void shield_srand(unsigned int seed)
{
        rand_seed = seed;
        return;
}

/**
 * @brief GNU libC compliant POSIX random calculation
 */
int shield_rand_r(unsigned int *seedp)
{
        (*seedp) = ((*seedp) * 1103515245) + 12345;
        return (int)((unsigned int)((*seedp)/65536) % (RAND_MAX + 1));
}

ssize_t shield_getrandom(void *buf, size_t buflen, unsigned int flags)
{
    ssize_t copied = -1;
    if (unlikely(buf == NULL)) {
        goto end;
    }
    if (unlikely(buflen > 65535)) {
        __shield_set_errno(EINVAL);
        goto end;
    }
    copied = get_entropy(buf, buflen);
end:
    return copied;
}

#ifndef TEST_MODE
/* if not in the test suite case, aliasing to POSIX symbols */
int rand(void) __attribute__((alias("shield_rand")));
void srand(unsigned int seedp) __attribute__((alias("shield_srand")));
int rand_r(unsigned int *seedp) __attribute__((alias("shield_rand_r")));
ssize_t getrandom(void *buf, size_t buflen, unsigned int flags) __attribute__((alias("shield_getrandom")));
#endif
