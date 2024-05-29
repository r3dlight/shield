// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#ifndef POSIX_SIGNAL_H
#define POSIX_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_WITH_SENTRY
#include <types.h>
#include <uapi.h>

/** NOTE: this value should be kernel delivered */
#define _SIGNUM SIGNAL_USR2
typedef taskh_t pid_t;

enum posix_sigs {
  SIGABORT = SIGNAL_ABORT,
  SIGALARM = SIGNAL_ALARM,
  SIGBUS = SIGNAL_BUS,
  SIGCONT = SIGNAL_CONT,
  SIGILL = SIGNAL_ILL,
  SIGPIPE = SIGNAL_PIPE,
  SIGPOLL = SIGNAL_POLL,
  SIGTERM = SIGNAL_TERM,
  SIGTRAP = SIGNAL_TRAP,
  SIGUSR1 = SIGNAL_USR1,
  SIGUSR2 = SIGNAL_USR2
};
#endif

/* Future for multithreads compatible sigev structure */
#define __SIGEV_MAX_SIZE	64
#if __WORDSIZE == 64
# define __SIGEV_PAD_SIZE	((__SIGEV_MAX_SIZE / sizeof (int)) - 4)
#else
# define __SIGEV_PAD_SIZE	((__SIGEV_MAX_SIZE / sizeof (int)) - 3)
#endif

enum
{
  SIGEV_SIGNAL = 0,		/* Notify via POSIX signal */
  SIGEV_NONE,			    /* Pure userspace handling, timer polling only */
  SIGEV_THREAD 			  /* execute given handler at timer terminaison */
};

union sigval
{
  int sival_int;
  void *sival_ptr;
};
typedef union sigval __sigval_t;

typedef void (*sigev_notify_function_t)(__sigval_t sig);

/*
 * Simplified, yet POSIX sigevent_t. No support for pid_t
 */
typedef struct sigevent {
    sigev_notify_function_t sigev_notify_function;
    __sigval_t sigev_value;
    int sigev_signo;
    int sigev_notify;
} sigevent_t;

/*
 * sigset_t definition
 */
typedef struct
{
  bool __val[_SIGNUM];
} __sigset_t;

typedef __sigset_t sigset_t;

int sigpending(sigset_t *set);

int sigismember(const sigset_t *set, int signum);

int sigemptyset(sigset_t *set);

int sigfillset(sigset_t *set);

int sigaddset(sigset_t *set, int signum);

int sigdelset(sigset_t *set, int signum);

int kill(pid_t pid, int sig);

#ifdef __cplusplus
}
#endif

#endif/*!POSIX_SIGNAL_H*/
