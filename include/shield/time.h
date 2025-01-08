// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef POSIX_TIME_H
#define POSIX_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <shield/signal.h>

/**
 * @def time_t definition, using target unsigned long
 *
 * NOTE: Libshield do not support time64 use case, considered out of embedded scope
 */
typedef unsigned long time_t;

typedef enum clockid {
    CLOCK_MONOTONIC, /* monolithic clock, lonely supported by now */
    CLOCK_REALTIME,
    CLOCK_REALTIME_ALARM,
    CLOCK_BOOTTIME,
    CLOCK_BOOTTIME_ALARM,
} clockid_t;

/*!
 * @def standard timer_t type (timer identifier) for embedded.
 * Correspond to current cycle
 */
typedef uint64_t timer_t;


/**
 * @def POSIX compliant timespec structure definition
 *
 * @param tv_sec: number of seconds in the MONOLITIC clock
 * @param tv_nsec: number of nano-seconds to complete tv_sec for nsec precision
 */
struct timespec {
    time_t tv_sec;        /* seconds */
    long   tv_nsec;       /* nanoseconds */
};

/**
 * @def POSIX compliant itimerspec structure definition
 */
struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};


/*
 * POSIX-1 2001 and POSIX-1 2008 compliant nanosleep() implementation
 * TODO: errno is not set as not yet supported by libstd
 */
int nanosleep(const struct timespec *req, struct timespec *rem);

/*
 * INFO: to work properly, timer API request the calling task to have full time measurement access (upto cycle level).
 * Create a new, unique, timer identifier associated to the specified clock id.
 * The timer behavior (polling, thread execution...) is specified in the sigevent structure.
 * This function does not activate the timer.
 */
int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);

/*
 * Activate or reconfigure timer (if already set) according to new_value. If old_value exists, previously set
 * configuration is returned in old_value.
 * If new_value->it_value is set to 0, the timer is unset
 * If new_value->it_interval is not set to 0, the timer is periodic, based on new_value->it_value values for interval duration
 * If new_value->it_interval is set to 0 as previously configured timer was periodic, the timer is no more periodic
 */
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

/*
 * Poll given timer timerid. Return the residual time in curr_value.
 * - If timer is not set, cur_value fields are set to 0
 * - If timer is set, curr_value->it_value are set according to clockid
 * - If timer is set and is periodic, the timer period is also added to curr_value->it_interval
 */
int timer_gettime(timer_t timerid, struct itimerspec *curr_value);

/*
 * Get current time for clock identifier clockid in timespec sp (POSIX API)
 */
int clock_gettime(clockid_t clockid, struct timespec *tp);

#ifdef __cplusplus
}
#endif

#endif/*!POSIX_TIME_H*/
