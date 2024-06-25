sigpending
""""""""""

**Name**

   sigpending - test for pending signals

**Synopsys**

   .. code-block:: c
      :caption: sigpending Synopsys

      #include <signal.h>

      int sigpending(sigset_t *set);

**Description**

   sigpending() function test that there is one or more signal(s) waiting in the job's signal input queue. The mask of pending signal is returned in set.

   The given signal set is considered as initialized with sigemptyset(), so that only currently detected signals is set in the signal set.

**Return value**

   sigpending() return 0 on success, or -1 on error. In the latter case, errno is set accordingly.

**Errors**

   EFAULT

      the signal set is NULL.

   EINVAL

      the signal queue read failed.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Note**

   The supported signal list is limited by the Sentry kernel. See kill(3) for the complete listing of signals.

**See also**

   sigaddset(3), sigdelset(3), sigemptyset(3), sigfillset(3), sigismember(3), kill(3)
