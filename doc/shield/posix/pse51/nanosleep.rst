nanosleep
"""""""""

**Name**

   nanosleep - high-resolution sleep

**Synopsys**

   .. code-block:: c
      :caption: nanosleep Synopsys

      #include <time.h>

      int nanosleep(const struct timespec *req, struct timespec *rem);

**Description**

   nanosleep() function suspends the execution of the calling thread until:

      * the time specified in req has elapsed
      * the calling thread receives an external event (signal, IPC or IRQ targetting the task)
      * the calling thread terminates

   if interrupted by a signal, nanosleep() return -1 and set errno to EINTR and write the remaining time into rem, if not NULL.

   The timespec structure is used to define intervals with nanosecond precision level:

   .. code-block:: c

      struct timespec {
         time_t tv_sec; /* seconds */
         long   tv_nsec; /* nanoseconds */
      };

   It is to note that the value of tv_nsec is limited due to the field size, the overall time to wait (or remaining time) being the addition of the seconds and nanoseconds fields.
   nanosleep() is made to ensure a more precise waiting time, in comparison with sleep(), which is limited to second precision.

**Return value**

   On successful sleeping, the nanosleep() function returns 0. If the call is interrupted by an external event, nanosleep() returns -1 and errno is set accordingly.

**Errors**

   EINVAL

      the req argument is NULL, or if req->tv_nsec is too big.

   EINTR

      the calling thread nanosleep() call has been interrupted by an external event targetting the task.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Note**

   The Shield implementation use an active wait if the requested time is shorter than the system tick period, or passive wait (calling the sys_sleep() syscall) otherwise.

**See also**

   sleep(3)
