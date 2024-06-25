clock_gettime
"""""""""""""

**Name**

   clock_gettime() - clock and time function

**Synopsys**

   .. code-block:: c
      :caption: clock_gettime Synopsys

      #include <time.h>

      int clock_gettime(clockid_t clockid, struct timespec *tp);


**Description**

   The function clock_gettime() retreive the time of the specified clockid into tp timespec structure pointed by tp. The clockid is the identifier of the particular clock on which the access is requested.

   POSIX define multiple clockid, but the Shield libc implementation only support CLOCK_REALTIME

   CLOCK_REALTIME

      A stable, system-wide clock that measure the real time since bootup. This clock is affected by discontinuous jumps that are the consequence of power-management, if the core enters sleep mode. This clock is strictly monotonic.

**Return value**

   clock_gettime() returns 0 for success, -1 for failure. In that last case, errno is set appropriately.

**Errors**

   EINVAL

      the clockid is invalid or fp is NULL

   EPERM

      the clock measurement syscall failed

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Note**

   The libshield implementation of the clock_gettime() API only support the CLOCK_REALTIME clockid, as by now, only the hardware core timestamp counter is accessible.
   The EPERM error is not POSIX, the clock measurement access failure may be the consequence of a Sentry kernel failure or deny.
