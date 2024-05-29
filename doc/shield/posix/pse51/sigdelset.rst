sigdelset
"""""""""

**Name**

   sigemptyset, sigfillset, sigaddset, sigdelset, sigismember - POSIX signal set manipulation API

**Synopsys**

   .. code-block:: c

      #include <signal.h>

      int sigemptyset(sigset_t *set);

      int sigfillset(sigset_t *set);

      int sigaddset(sigset_t *set, int signum);

      int sigdelset(sigset_t *set, int signum);

      int sigismember(const sigset_t *set, int signum);

**Description**

   All these functions are made in order to manipulate POSIX signal sets.

   sigemptyset() clear the given set, making it empty.

   sigfillset() fullfill the given set, making it set for all signals.

   sigaddset() add a given signal to the set.

   sigdelset() remove a signal from the set.

   sigismember() tests wether the signal signum exists in the set.

   sigaddset(), sigdelset() and sigismember() consider that the given set has already been initialized with one of sigemptyset() or sigfillset(). If not, the result is unpredictable.

**Return value**

   sigemptyset(), sigfillset(), sigaddset(), and sigdelset() return 0 on success, or -1 on failure, with errno set accordingly.

   sigismember() return 1 if signum exists in the set, or 0 otherwise.

**Errors**

   EINVAL

      set is NULL, or, for functions that have the signum argument, signum is not a valid signal.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Note**

   The supported signal list is limited by the Sentry kernel. See kill(3) for the complete listing of signals.

**See also**

   sigpending(3), kill(3)
