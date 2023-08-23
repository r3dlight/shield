
// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <stdatomic.h>
#include <stdint.h>
#include <shield/errno.h>
#include <pthread.h>

/**
 * max number of concurrent thread per task. Defines the number of instance of errno to handle
 */
#define MAX_THREAD_PER_TASK CONFIG_MAX_THREAD_PER_TASK

/*
 * This is the local ISO C, thread-safe, errno implementation.
 *
 * As we use a microkernel implementation, most of the errno set (for POSIX compatibility)
 * is made by the libshield itslef. Although, any errno access from out of the library itself
 * is kept read only (errno is a read-only variable for most of the userspace code).
 *
 * Like in POSIX systems, the current thread errno_v vector is initiated to zero by the
 * runtime .init funtion. 0 means 'errno has never been set'.
 */
static atomic_int shield_errno_v[MAX_THREAD_PER_TASK];

/**
 * the current thread errno instance is associated to the current thread id. The current thread
 * id is under the responsability of the ukernel/libc interaction, through the .init function
 * setting in the stack top content (stack local information set by the kernel at the initial
 * thread execution and as such, thread local). This identifier is accessible through a libc
 * private builtin only.
 * pthread_self() is a standard POSIX symbol for this. Its implementation may vary
 */
int __shield_errno_location(void) {
    uint8_t threadid = pthread_self();
    return shield_errno_v[threadid];
}

/**
 * \brief __libshield_set_errno set the errno value
 *
 * there is no check of val here, as valid error values are randomly ceated.
 * Although, while using canonical names only, any divergent value (faulted or
 * invalidely set) will be detected.
 */
void __shield_set_errno(int val) {
    atomic_store(&(shield_errno_v[pthread_self()]), val);
}
