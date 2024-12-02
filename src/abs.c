// SPDX-FileCopyrightText: 2024 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <shield/stdlib.h>

int abs(int j)
{
    return __builtin_abs(j);
}

long labs(long j)
{
    return __builtin_labs(j);
}

long long llabs(long long j)
{
    return __builtin_llabs(j);
}

intmax_t imaxabs(intmax_t j)
{
    return __builtin_imaxabs(j);
}
