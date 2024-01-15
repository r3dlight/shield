// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: LicenseRef-LEDGER

#include <stdbool.h>
#include <shield/string.h>
#include <shield/signal.h>
#include <shield/time.h>
#include <shield/errno.h>
#include <shield/private/sort.h>
#include <shield/private/errno.h>
#include <uapi.h>

#define TIME_DEBUG 0

/**
 * @def number of nanosecs in one miliseconds
 *
 * We use a signed type here, as POSIX tv_nsec is typed long in POSIX-PSE51
 * the tv_nsec is always kept under a second valud (i.e. 1.000.000.000) as
 * it hold the complement to tv_sec field. As such, the sign bit is never
 * reached.
 */
#define MILI_IN_NSECS 1000000L

/**
 * @def number of nanosecs in one seconds
 *
 * We use a signed type here, as POSIX tv_nsec is typed long in POSIX-PSE51
 * the tv_nsec is always kept under a second valud (i.e. 1.000.000.000) as
 * it hold the complement to tv_sec field. As such, the sign bit is never
 * reached.
 */
#define SEC_IN_NSECS 1000000000L

/* macros for module and multiply when separating nsec and sec fields */
#define MILI_IN_SEC 1000UL
#define MICRO_IN_SEC 1000000UL
#define MICRO_IN_MSEC MILI_IN_SEC
#define MICRO_IN_NSEC MILI_IN_SEC
#define NANO_IN_MSEC MICRO_IN_SEC

/*
 * max timers in the list (including postponed ones) *-/
 */
#define STD_POSIX_TIMER_MAXNUM 5

typedef struct timer_info {
    /** Timer identifier (uint64_t), set at timer creation time
        differ from 'key', upgraded with next timer trigger timetstamp
        MUST be aligned on 8 bytes to avoid strd usage fault
    */
    timer_t         id;
    /** duration in ms (at least for initial, it periodic) */
    uint32_t        duration_ms;
    sigev_notify_function_t sigev_notify_function;
    __sigval_t      sigev_value;
    /** notify mode */
    int             sigev_notify;
    /** period (interval) specification, if periodic == true */
    struct timespec period;
    /** timer is active (timer_settime() has been executed at least once)
     * with non-null it_value content */
    bool            set;
    /** timer has been postponed by another timer_settime().
        A new node has been created. for this node, the timer_handler()
        should not call the notify function. */
    bool            postponed;
    /** when setting a timer with it_interval, the timer is executed
        periodicaly until a new timer_settime() reconfigure it. */
    bool            periodic;
    /* is this entry valid ? */
    bool            valid;
} timer_info_t;

/**
 * timers subsystem context
 */
typedef struct timers_context {
    timer_info_t timers[STD_POSIX_TIMER_MAXNUM];
    timer_info_t active_timers[STD_POSIX_TIMER_MAXNUM];
    uint8_t num_timers;
    uint8_t num_active_timers;
} timers_context_t;

_Alignas(uint64_t) timers_context_t timer_ctx;


/******************************************************************
 * Local static utility functions, that interact with list backend
 */

/**
 * @brief find the first timer in a timer list based on its identifier
 */
static inline timer_info_t *__timer_find(timer_info_t * const timer_list, const timer_t key)
{
    timer_info_t *timer = NULL;
    for (uint8_t i = 0; i < STD_POSIX_TIMER_MAXNUM; ++i) {
        if ((timer_list[i].valid == true) && (timer_list[i].id == key)) {
            timer = &timer_list[i];
        }
        break;
    }
    return timer;
}

/**
 * @brief find the first postponed timer in a timer list based on its identifier
 */
static inline timer_info_t *__timer_find_not_postponed(timer_info_t * const timer_list, const timer_t key)
{
    timer_info_t *timer = NULL;
    for (uint8_t i = 0; i < STD_POSIX_TIMER_MAXNUM; ++i) {
        if ((timer_list[i].valid == true) &&
            (timer_list[i].id == key) &&
            (timer_list[i].postponed == false)) {
            timer = &timer_list[i];
        }
        break;
    }
    return timer;
}

/**
 * @brief find a free timer cell in a timer list and return it
 */
static inline timer_info_t *__timer_find_freenode(timer_info_t * const timer_list)
{
    timer_info_t *timer = NULL;
    for (uint8_t i = 0; i < STD_POSIX_TIMER_MAXNUM; ++i) {
        if (timer_list[i].valid == false) {
            timer = &timer_list[i];
        }
        break;
    }
    return timer;
}

/**
 * @brief get back current time in microseconds
 */
static inline int __timer_get_time_us(uint64_t *time)
{
    int errcode = 0;
    if (sys_get_cycle(PRECISION_MICROSECONDS) != STATUS_OK) {
        errcode = -1;
        __shield_set_errno(EPERM);
        goto err;
    }
    if (unlikely(copy_to_user((uint8_t*)time, sizeof(uint64_t)) != STATUS_OK)) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
err:
    return errcode;
}

/**
 * @brief get back current time in miliseconds
 */
static inline int __timer_get_time_ms(uint64_t *time)
{
    int errcode = 0;
    if (sys_get_cycle(PRECISION_MILLISECONDS) != STATUS_OK) {
        errcode = -1;
        __shield_set_errno(EPERM);
        goto err;
    }
    if (unlikely(copy_to_user((uint8_t*)time, sizeof(uint64_t)) != STATUS_OK)) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
err:
    return errcode;
}

/**
 * @brief get back current time in nanoseconds
 */
static inline int __timer_get_time_ns(uint64_t *time)
{
    int errcode = 0;
    if (sys_get_cycle(PRECISION_NANOSECONDS) != STATUS_OK) {
        errcode = -1;
        __shield_set_errno(EPERM);
        goto err;
    }
    if (unlikely(copy_to_user((uint8_t*)time, sizeof(uint64_t)) != STATUS_OK)) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
err:
    return errcode;
}

/**
 * comparion function to be used by the sort function, so that
 * active timers list is always ordered based on elapsed time. As
 * a consequence, they are ordered synchronously with successive kernel
 * alarm
 */
int timer_compare(const void *a, const void *b)
{
    int res = 0;
    uint64_t now_us;
    uint64_t eta_us_a;
    uint64_t eta_us_b;
    __timer_get_time_us(&now_us);
    const timer_info_t *ta = (const timer_info_t*)a;
    const timer_info_t *tb = (const timer_info_t*)b;
    eta_us_a = (ta->id + (ta->duration_ms*1000)) - now_us;
    eta_us_b = (tb->id + (tb->duration_ms*1000)) - now_us;
    if (ta->valid == false) {
        /* invalid are pushed at the end: force swapping */
        res = 1;
        goto end;
    }
    if (tb->valid == false) {
        /* right one is invalid, no swap */
        res = -1;
        goto end;
    }
    if (eta_us_a > eta_us_b) {
        /* a elapsed time is bigger than b, swap */
        res = 1;
    } else {
        res = -1;
    }
end:
    return res;
}

/*
 * Create a new timer node using the given key as timer identifier
 *
 * the timer identifier is timestamp based with microsecond level precision to avoid
 * timer_t identifier collisision.
 *
 * a created timer is never set by default (see POSIX PSE51-1)
 */
static int __timer_create_node(struct sigevent *sevp, timer_t key, bool periodic)
{
    int errcode = 0;

    timer_info_t* timer;
    if (unlikely((timer = __timer_find_freenode(timer_ctx.timers)) == NULL)) {
        errcode = -1;
        __shield_set_errno(ENOMEM);
        goto err;
    }

    timer->sigev_notify_function = sevp->sigev_notify_function;
    timer->sigev_value = sevp->sigev_value;
    timer->sigev_notify = sevp->sigev_notify;
    timer->id = key;
    timer->set = false;
    timer->postponed = false;
    timer->periodic = periodic;
    timer->duration_ms = 0;

    timer_ctx.num_timers++;
err:
    return errcode;
}


/*
 * set a timer (created or already set) in the timer context storage
 * and active the alarm
 */
static int __timer_setnode(timer_t id,
                           const struct timespec *ts,
                           bool periodic,
                           const struct timespec *interval_ts,
                           struct itimerspec *old)
{
    int errcode = 0;
    Status ret;
    timer_info_t *timer = NULL;
    timer_info_t *active_timer = NULL;
    uint32_t period_ms = 0;
    /* foreach node, get back its id....
     * As we have locked the timer lock, we can read the list manualy here */
    period_ms = (ts->tv_sec * MILI_IN_SEC);
    period_ms += (ts->tv_nsec / NANO_IN_MSEC);

    /* searching for node, starting with unset timers.... */
    if (unlikely(timer_ctx.num_timers == 0) ||
        unlikely((timer = __timer_find(timer_ctx.timers, id)) == NULL)) {
        /* no unset timer found, fallback to already set timers */
        if (unlikely((timer = __timer_find(timer_ctx.active_timers, id)) == NULL)) {
            errcode = -1;
            __shield_set_errno(EINVAL);
            goto err;
        }

        /* when timer already set and 'old' is non-null, set the previously
         * configured values to it */
        if (old) {
            old->it_value.tv_sec = timer->duration_ms / 1000;
            old->it_value.tv_nsec = (timer->duration_ms  % 1000) * 1000000;
            if (timer->periodic == false) {
                old->it_interval.tv_sec = 0;
                old->it_interval.tv_nsec = 0;
            } else {
                old->it_interval.tv_sec = timer->period.tv_sec;
                old->it_interval.tv_nsec = timer->period.tv_nsec;
            }
        }
        /* for all nodes having the same id (including that one), mark them as 'postponed'. */
        timer->postponed = true;
        while ((timer = __timer_find_not_postponed(timer_ctx.active_timers, id)) != NULL) {
            timer->postponed = true;
            timer->periodic = false;
        }
        if (period_ms == 0) {
            timer_info_t *unset_timer;
            /* we require to set a node with NULL value, meaning cleaning a timer,
             * not setting a new one.
             * Previous instances has been postponed, we can leave now... */
            /* this timer being postpone but without newly created one, the handler will not
             * move it back to the unset timers list. We do it now */
            timer->postponed = true;
            timer->periodic = false;
            if (unlikely((unset_timer = __timer_find_freenode(timer_ctx.timers)) == NULL)) {
                errcode = -1;
                __shield_set_errno(ENOMEM);
                goto err;
            }
            memcpy(unset_timer, timer, sizeof(timer_info_t));
            errcode = 0;
            goto end;
        }
        /* get back free entry in active timers table to add new entry */
        active_timer = NULL;
        if (unlikely((active_timer = __timer_find_freenode(timer_ctx.active_timers)) == NULL)) {
            errcode = -1;
            __shield_set_errno(ENOMEM);
            goto err;
        }
        active_timer->sigev_notify_function = timer->sigev_notify_function;
        active_timer->sigev_value = timer->sigev_value;
        active_timer->sigev_notify = timer->sigev_notify;
        active_timer->id = timer->id;
        active_timer->set = true;
        active_timer->postponed = false;
        if (periodic == true) {
            active_timer->periodic = true;
        }
        active_timer->duration_ms = period_ms;
        /* finished, we can call kernel alarm */
    } else {
        /* timer found in unset timers */
        if (period_ms == 0) {
            /* unsetting an unset timer has no meaning... */
            goto err;
        }
        /* for unset timers, if old, exists, set it with zeros */
        if (old != NULL) {
            memset(old, 0x0, sizeof(struct itimerspec));
        }
        /* simple case: move timer from created to active timers list */
        if (unlikely((active_timer = __timer_find_freenode(timer_ctx.active_timers)) == NULL)) {
            errcode = -1;
            __shield_set_errno(ENOMEM);
            goto err;
        }
        memcpy(active_timer, timer, sizeof(timer_info_t));
        timer_ctx.num_active_timers++;
        timer_ctx.num_timers--;
        /* remove timers from create but not set timers list */
        timer->valid = false;
        active_timer->set = true;
        active_timer->duration_ms = period_ms;

        if (periodic == true) {
            active_timer->periodic = true;
            /* two cases: interval == initial period (interval_ts == NULL)
               or interval != from initial period (interval_ts exists) */
            if (interval_ts) {
                active_timer->period.tv_sec = interval_ts->tv_sec;
                active_timer->period.tv_nsec = interval_ts->tv_nsec;
            }
        }
    }
    /* ok whatever the case here, active_node hold the newly created timer */
    /* reorder active timers based on id (time based) */
    bubble_sort(timer_ctx.active_timers, STD_POSIX_TIMER_MAXNUM, sizeof(timer_info_t), timer_compare, NULL);

    /* call sigalarm() */
    switch (sys_alarm(period_ms)) {
        case STATUS_OK:
            goto err;
        case STATUS_DENIED:
            /* TODO: remove node */
            errcode = -1;
            __shield_set_errno(EPERM);
            goto err;
            break;
        default:
            /* TODO: remove node */
            errcode = -1;
            __shield_set_errno(EAGAIN);
            goto err;
            break;
    }

end:
err:
    return errcode;
}


/* timer handler that is effectively called by the kernel */
int timer_handler(void)
{
    uint64_t key;
    int ret;
    int errcode;

    /* the timer associated to the current handle is ALWAYS the first cell */
    timer_info_t *timer = &timer_ctx.active_timers[0];
    if (timer->postponed == true) {
        timer->valid = false;
        /* the current timer node has been postponed. Another timer node exists (or has existed)
         * and has been/will be executed in order to execute the associated callback. By now,
         * just do nothing except reordering.
         */
    } else {
        switch (timer->sigev_notify) {
            case SIGEV_THREAD:
                timer->sigev_notify_function(timer->sigev_value);
                break;
            default:
                break;
        }
        /* upper thread execution is requested. The callback **must** be set as it has been checked
         * at creation time. */
        if (timer->periodic == false) {
            timer_info_t *inactive_timer;
            timer->valid = false;
            /* move timer to created but not set */
            if (unlikely((inactive_timer = __timer_find_freenode(timer_ctx.timers)) == NULL)) {
                errcode = -1;
                __shield_set_errno(ENOMEM);
                goto err;
            }
            memcpy(inactive_timer, timer, sizeof(timer_info_t));
            timer_ctx.num_timers++;
            timer_ctx.num_active_timers--;
        } else {
            /* update id to current timestamp (used to define timer static point)*/
            if (unlikely(__timer_get_time_ns(&timer->id) != STATUS_OK)) {
                errcode = -1;
                __shield_set_errno(EPERM);
                goto err;
            }
        }
    }
    errcode = 0;
err:
    bubble_sort(timer_ctx.active_timers, STD_POSIX_TIMER_MAXNUM, sizeof(timer_info_t), timer_compare, NULL);
    return errcode;
}

/**************************************************************************
 * Exported functions part 1; timers
 */


/* zeroify function, called at task preinit.*/
void timer_initialize(void)
{
    memset(&timer_ctx, 0x0, sizeof(timers_context_t));
    //dangling_timers = 0;
}



/**
 * @brief Create a timer (timer is not activated here).
 *
 * POSIX PSE51-1 compliant
 */
int shield_timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
    int errcode = 0;
    uint8_t ret;

    /* by now, CLOCK_REALTIME not supported */
    if (clockid == CLOCK_REALTIME) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
    /* other input params sanitation */
    if (clockid > CLOCK_MONOTONIC && clockid <= CLOCK_BOOTTIME_ALARM) {
        errcode = -1;
        __shield_set_errno(ENOTSUP);
        goto err;
    }
    if (clockid > CLOCK_BOOTTIME_ALARM || sevp == NULL || timerid == NULL) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
    /* by now, SIGEV_SIGNAL is not supported */
    if (sevp->sigev_notify == SIGEV_SIGNAL) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
    /* other input params sanitation */
    if (sevp->sigev_notify != SIGEV_THREAD && sevp->sigev_notify != SIGEV_NONE) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
    /* SIGEV_THREAD case: check notify function */
    if (sevp->sigev_notify == SIGEV_THREAD && sevp->sigev_notify_function == NULL) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
    /* timer identifier is the current cycle id. To avoid collision in case ot SMP
     * system, we use a semaphore to lock a shared ressource between getting current
     * timestamp, then unlock the semaphore. Doing this, even in SMP systems, concurrent
     * timer_create() will have separated timer_id.
     */
    if (unlikely(__timer_get_time_ns(timerid) != STATUS_OK)) {
        errcode = -1;
        __shield_set_errno(EPERM);
        goto err;
    }
    errcode = __timer_create_node(sevp, *timerid, false);
err:
    return errcode;
}

/*
 * Activate timer
 *
 * At settime():
 * if the node set at create() time is not yet active:
 * - it is get back (using timerid==key)
 * - upgraded by setting the key properly with the new itimespec informations
 * - re-added correctly in the list accordingly
 * if the node set at create() time is already active (a previous create() has been done)
 * - all potential existing nodes are flagued as 'postponed' (the handler will not call the upper layer)
 * - a new node is created using the itimespec information and set in the list correctly (it may be **before** a previously postoned
 *   timer if the time is reduced
 *
 * The alarm request is sent to the kernel.
 */
int shield_timer_settime(timer_t timerid, int flags __attribute__((unused)), const struct itimerspec *new_value, struct itimerspec *old_value __attribute__((unused)))
{
    int errcode = 0;
    const struct timespec *ts;
    bool interval = false;
    bool cleaning = false;

    /* Sanitize first. old_value is allowed to be NULL */
    if (new_value == NULL) {
        errcode = -1;
        __shield_set_errno(EFAULT);
        goto err;
    }
    /* select type of setting (value or interval) */
    /*TODO: FIX1: if it_value fields are 0 -> unset timer */
    ts = &new_value->it_value;
    if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0) {
        /* simply clean previously set timer */
        cleaning = true;
    }
    /* if both interval and value are non-null, this is a periodic timer */
    if ((new_value->it_interval.tv_sec != 0 || new_value->it_interval.tv_nsec != 0) &&
        (new_value->it_value.tv_sec != 0 || new_value->it_value.tv_nsec != 0)) {
        /* an periodic interval is requested after first trigger (set by it_value)*/
        interval = true;
    }
    /* when not unsetting a timer, timer specs must be large enough */
    if (cleaning == false) {
        if (ts->tv_sec == 0 && ts->tv_nsec < MILI_IN_NSECS) {
            /* too short timer step */
            errcode = -1;
            __shield_set_errno(EINVAL);
            goto err;
        }
        if (ts->tv_nsec >= SEC_IN_NSECS) {
            /* nsec bigger than 1 sec (POSIX compliance) */
            errcode = -1;
            __shield_set_errno(EINVAL);
            goto err;
        }
        if (interval == true &&
            new_value->it_interval.tv_sec == 0 &&
            new_value->it_interval.tv_nsec < MILI_IN_NSECS)
        {
            /* too short interval */
            errcode = -1;
            __shield_set_errno(EINVAL);
            goto err;
        }
    }
    errcode = __timer_setnode(timerid, &new_value->it_value, interval, &new_value->it_interval, old_value);
err:
    return errcode;
}

int shield_timer_gettime(timer_t timerid, struct itimerspec *curr_value)
{
    uint64_t now_us;
    uint64_t eta_us;
    int errcode;
    timer_info_t *timer = NULL;
    uint8_t ret;
    /* Sanitize first */
    if (curr_value == NULL) {
        errcode = -1;
        __shield_set_errno(EFAULT);
        goto err;
    }

    if (timer_ctx.num_active_timers == 0) {
        /* no timer set */
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }

    if (unlikely((timer = __timer_find(timer_ctx.active_timers, timerid)) == NULL)) {
        if (unlikely((timer = __timer_find(timer_ctx.timers, timerid)) == NULL)) {
            errcode = -1;
            __shield_set_errno(EINVAL);
            goto err;
        }
    }
    /*@ assert \valid(timer);*/

    if (timer->set == false) {
        /* unset timer */
        curr_value->it_value.tv_sec = 0;
        curr_value->it_value.tv_nsec = 0;
        errcode = 0;
        goto err;
    }

    /* calculate remaining time for current timer */
    __timer_get_time_us(&now_us);
    eta_us = (timer->id + (timer->duration_ms*1000)) - now_us;
    /* timer id is the time (in us) when the timer has been set */
    //memcpy(&timer_us, &info->id, sizeof(uint64_t));
    if (timer->periodic == true) {
        curr_value->it_interval.tv_nsec = (timer->duration_ms % 1000) * MILI_IN_NSECS;
        curr_value->it_interval.tv_sec = timer->duration_ms / MILI_IN_SEC;
    }
    curr_value->it_value.tv_nsec = (eta_us % MILI_IN_NSECS) * MILI_IN_SEC;
    curr_value->it_value.tv_sec = eta_us / MILI_IN_NSECS;
    errcode = 0;
err:
    return errcode;
}

/**************************************************************************
 * Exported functions part 1; clock
 */

int shield_clock_gettime(clockid_t clockid, struct timespec *tp)
{
    int errcode = 0;
    uint64_t time;
    /* sanitation */
    if (tp == NULL) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }
    /* by now, no support for RTC clock. For boards with RTC clocks, add a config
     * option to allow CLOCK_REALTIME */
    if (clockid != CLOCK_MONOTONIC) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }

    if (likely(__timer_get_time_us(&time) == STATUS_OK)) {
        tp->tv_nsec = (time % MICRO_IN_SEC) * MICRO_IN_NSEC;
        tp->tv_sec = (time / MICRO_IN_SEC);
        goto end;
    }
    /* EPERM is not a POSIX defined return value, but time measurement is controled on EwoK */
    errcode = -1;
    __shield_set_errno(EPERM);
err:
end:
    return errcode;
}

int shield_nanosleep(const struct timespec *req, struct timespec *rem)
{
    int errcode = 0;
    if (unlikely(req == NULL)) {
        errcode = -1;
        __shield_set_errno(EINVAL);
        goto err;
    }

    if (unlikely(req->tv_sec > 0)) {
        /* nanosleeping.... for a **very** long time ? */
        enum Status status;
        struct SleepDuration sd;

        sd.arbitrary_ms = req->tv_nsec / MILI_IN_NSECS;
        /* int overflow check first */
        if (unlikely(req->tv_sec >= ((0xffffffffUL) - sd.arbitrary_ms))) {
            errcode = -1;
            __shield_set_errno(EINVAL);
            goto err;
        }
        sd.arbitrary_ms += req->tv_sec;
        sd.tag = SLEEP_DURATION_ARBITRARY_MS;
        status = sys_sleep(sd, SLEEP_MODE_SHALLOW);
        if (unlikely(status != STATUS_OK)) {
            errcode = -1;
            __shield_set_errno(EINTR);
            goto err;
        }
        /* TODO add rem content if duration is lesser in long sleep mode */
    } else {
        /* active wait here, the scheduler may preempt the thread though */
        uint32_t sleep_time_ms = req->tv_nsec / MILI_IN_NSECS;
        uint64_t start;
        uint64_t curr;
        /* should never fail with us precision */
        __timer_get_time_us(&start);
        do {
            __timer_get_time_us(&curr);
        } while ((curr - start) < (req->tv_nsec / MICRO_IN_NSEC));
    }
err:
    return errcode;
}

#ifndef TEST_MODE
int clock_gettime(clockid_t clockid, struct timespec *tp) __attribute__((alias("shield_clock_gettime")));
int timer_gettime(timer_t timerid, struct itimerspec *curr_value) __attribute__((alias("shield_timer_gettime")));
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value) __attribute__((alias("shield_timer_settime")));
int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid) __attribute__((alias("shield_timer_create")));
int nanosleep(const struct timespec *req, struct timespec *rem) __attribute__((alias("shield_nanosleep")));
#endif/*!TEST_MODE*/
