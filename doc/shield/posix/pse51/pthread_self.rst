pthread_self
""""""""""""

**Name**

   pthread_self - return current thread identifier

**Synopsys**

   .. code-block:: c
      :caption: pthread_self Synopsys

      #include <pthread.h>

      pthread_t pthread_self(void);

**Description**

   pthread_self() return the current thread (i.e. job in Outpost environment) identifier. A given job of a given task may restart, depending on its configuration, making its kernel identifier (taskh_t opaque) evolves. This function always return the uptodate job identifier for the current task.

**Return value**

   This funcion always return the current job identifier.

**Errors**

   This function always succeeds.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008 (partial)

**Note**

   It is to note that there is not need for -pthread link in embedded Outpost Shield usage (POSIX incompatibility).
