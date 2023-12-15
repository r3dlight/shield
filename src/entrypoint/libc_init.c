// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <shield/private/timer.h>

void __libc_init(void)
{
    timer_initialize();
    return;
}
