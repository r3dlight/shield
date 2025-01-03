// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#include <shield/private/timer.h>

void __libc_init(void)
{
    timer_initialize();
    return;
}
