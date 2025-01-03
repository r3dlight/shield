// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#include <shield/stdio.h>
#include <stdatomic.h>

static atomic_bool asserted = ATOMIC_VAR_INIT(false);

/**
 * @brief assert handler.
 *
 * @param filename file that throw assert
 * @param lineno line number in file
 * @param func function name
 * @param cond condition evaluated to false in assertion
 *
 * This function is call by `assert` with the following arguments.
 * This function may trap after logging assert message.
 *
 * assert policy to be discussed later.
 */
__attribute__((noreturn))
void __assert_func (const char *filename, int lineno, const char *func, const char *cond)
{
    atomic_bool expected_false = ATOMIC_VAR_INIT(false);

    /*
     * XXX:
     *  If for any reason, there is a double fault or assert, trap immediately
     */
    if (atomic_compare_exchange_strong(&asserted, &expected_false, true)) {
        printf("assert %s failed %s:%d %s", cond, filename, lineno, func);
    }
    __builtin_trap();
}
