kill
""""

**Name**

   kill - sens signal to another task

**Synopsys**

   .. code-block:: c
      :caption: kill Synopsys

      #include <signal.h>

      int kill(pid_t pid, int sig);


**Description**

   The kill() function emit the signal sig toward the target task handle identified by pid.

   if pid is a valid task handle on the system, the signal is emitted. If pid is 0, there is no transmission (POSIX incompatilibity). pid equality to -1 is also not supported (POSIX incompatibility) as the pid argument is, on Outpost, an unsigned value. In the same way, sig=0 signal emission check is not supported.

   A task is always allowed to emit a signal to another task of the same domain. Althgough, only one signal at a time can be emitted to the very same target. In that case, the kill() call fails.

**Return value**

   On success, 0 is returned. On error, -1 is returned and errno is set accordingly.

**Errors**

   EINVAL

      invalid input signal or invalid targetted pid (task handle on Sentry). The same error is emitted if the target pid signal entry is busy.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008 (partial)

**Note**

   On Outpost, the following POSIX signals are supported:

   SIGABORT, SIGALARM, SIGBUS, SIGCONT, SIGILL, SIGPIPE, SIGPOLL, SIGTERM, SIGTRAP, SIGUSR1, SIGUSR2
