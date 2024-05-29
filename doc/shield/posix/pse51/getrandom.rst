getrandom
"""""""""

**Name**

   getrandom - obtain a set of random bytes

**Synopsys**

   .. code-block:: c
      :caption: getrandom Synopsys

      #include <sys/random.h>

      ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);


**Description**

   The function getrandom() is a Linux-compliant implementation over Sentry kernel.

   the flags argument is kept for Linux compatibility, but is ignored in the shield implementation. If the current task is not allowed to access the kernel RNG, the entropy source used is the SSP initial
   value, which is derivated using the standard POSIX srand()/rand() API.


**Return value**

   getrandom() returns 0 for success, -1 for failure. In that last case, errno is set appropriately.

**Errors**

   EINVAL

      the buffer is NULL or its length is bigger than 65535.

**Conforming to**

   Linux-specific getrandom() implementation. This function is not POSIX

**Note**

   There is no way to differentiate, at POSIX level, rand() usage from KRNG usage when asking for entropy. The behavior is based on the CAP_SYS_RNG capability allowance, checked by the Sentry kernel.
