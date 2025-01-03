// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#include <shield/pthread.h>

#if CONFIG_WITH_SENTRY
# include <support/sentry.h>
#endif

pthread_t pthread_self(void)
{
#if CONFIG_WITH_SENTRY
    return __libc_get_current_threadid();
#else
    return 0;
#endif
}
