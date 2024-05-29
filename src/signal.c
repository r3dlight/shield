#include <uapi.h>
#include <shield/signal.h>
#include <shield/errno.h>
#include <shield/string.h>
#include <shield/private/errno.h>
#include <shield/private/coreutils.h>

int sigpending(sigset_t *set)
{
    int res = -1;
    exchange_event_t * event = _memarea_get_svcexcange_event();
    Status sysres;
    uint32_t signal;

    if (unlikely(set == NULL)) {
        __shield_set_errno(EFAULT);
        goto end;
    }
    /* we may have more that one single signal pending. Checking while there are some pending sigs found */
    do {
        if (unlikely((sysres = sys_wait_for_event(EVENT_TYPE_SIGNAL, WFE_WAIT_NO)) != STATUS_OK)) {
            res = 0;
            goto end;
        }
        if (event->type != EVENT_TYPE_SIGNAL) {
            __shield_set_errno(EINVAL);
            goto end;
        }
        signal = *(uint32_t*)&event->data[0];
        if (signal > _SIGNUM) {
            __shield_set_errno(EINVAL);
            goto end;
        }
        set->__val[signal-1] = true;
    } while (sysres == STATUS_OK);
    res = 0;
end:
    return res;
}

int sigismember(const sigset_t *set, int signum)
{
    int res = -1;
    if (unlikely(set == NULL)) {
        __shield_set_errno(EFAULT);
        goto end;
    }
    if (unlikely(signum > _SIGNUM)) {
        __shield_set_errno(EINVAL);
        goto end;
    }
    if (set->__val[signum-1] == true) {
        res = 1;
    } else {
        res = 0;
    }
end:
    return res;
}

int sigemptyset(sigset_t *set)
{
    int res = -1;
    if (unlikely(set == NULL)) {
        __shield_set_errno(EFAULT);
        goto end;
    }
    memset(set->__val, 0, _SIGNUM * sizeof(bool));
    res = 0;
end:
    return res;
}

int sigfillset(sigset_t *set)
{
    int res = -1;
    if (unlikely(set == NULL)) {
        __shield_set_errno(EFAULT);
        goto end;
    }
    memset(&set->__val[0], 1, _SIGNUM * sizeof(bool));
    res = 0;
end:
    return res;
}

int sigaddset(sigset_t *set, int signum)
{
    int res = -1;
    if (unlikely(set == NULL)) {
        __shield_set_errno(EFAULT);
        goto end;
    }
    if (unlikely(signum > _SIGNUM)) {
        __shield_set_errno(EINVAL);
        goto end;
    }
    set->__val[signum-1] = 1;
    res = 0;
end:
    return res;
}

int sigdelset(sigset_t *set, int signum)
{
    int res = -1;
    if (unlikely(set == NULL)) {
        __shield_set_errno(EFAULT);
        goto end;
    }
    if (unlikely(signum > _SIGNUM)) {
        __shield_set_errno(EINVAL);
        goto end;
    }
    set->__val[signum-1] = 0;
    res = 0;
end:
    return res;
}

int kill(pid_t pid, int sig)
{
    int res = -1;

    if (unlikely(sys_send_signal(pid, sig) != STATUS_OK)) {
        /* do we differenciate ESRCH ? (invalid target) ? */
        __shield_set_errno(EINVAL);
        goto end;
    }
    res = 0;
end:
    return res;
}
