// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <stdint.h>

/**
 * Canari variable, as defined in LLVM & GCC compiler documentation, in order to
 * be manipulated each time a new frame is added on stack
 * INFO: this file is not compiled in UT mode
 */
uint32_t __stack_chk_guard = 0;

/* INFO: maybe let meson handle this for this very file only */
#if __GNUC__
#pragma GCC push_options
#pragma GCC optimize("-fno-stack-protector")
#endif

/**
 * Standard entrypoint type
 */
typedef int (*thread_entrypoint_t)(void);

/**
 * When starting a thread, the thread identifer and the SSP seed is
 * passed by the Sentry kernel. The effective thread entrypoint symbol
 * address is passed as third argument.
 * This very stack-based thread identifier is used by the errno internals to
 * differentiate which thread-safe errno value to use.
 * The seed is used to set the compiler-handled SSP balue.
 */
void __start_thread(uint32_t const thread_id, uint32_t const seed, thread_entrypoint_t const entrypoint)
{
    int task_ret;
    /* here, the kernel alreay have copied data and zeroified bss section */
    /* set the current SSP to kernel-given seed (stack-passed) */
    __stack_chk_guard = seed;
    __libc_init(); /* initiate libc-relative ontext, if needed (globlals, etc.) */
    /* calling thread entrypoint. the main function being implemented out of this file, SSP is active */
    task_ret = entrypoint();
    /* End of thread, store exit value in kernel thread information */
#if CONFIG_WITH_SENTRY
    sys_exit(task_ret);
#else
# error "no supported backend"
#endif
}

#if __GNUC__
#pragma GCC pop_options
#endif
