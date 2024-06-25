memset
""""""

**Name**

memset - fill a memory area with a constant byte value

**Synopsys**

   .. code-block:: c
      :caption: memset Synopsys

      #include <string.h>

      void *memset(void *s, int c, size_t n);


**Description**

   The memset() function fill the n first bytes of s with the constant byte c. s can be unaligned.

**Return value**

   memset() returns a pointer to memory area s.

**Errors**

   EINVAL

      s is NULL

**Conforming to**

   POSIX.1-2001, POSIX.1-2008, C89, C99, 4.3BSD

**Note**

   memset() is usually a compiler builtin, meaning that this implementation overseed compiler's one while the symbol implementation is linked.
