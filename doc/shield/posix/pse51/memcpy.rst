memcpy
""""""

**Name**

   memcpy - copy a memory area

**Synopsys**

   .. code-block:: c
      :caption: memcpy Synopsys

      #include <string.h>

      void *memcpy(void *dest, const void *src, size_t n);


**Description**

   The memcpy() function copies n bytes from memory area src to memory area dest. The memory area must not overlap, but can be unaligned.

**Return value**

   memcpy() returns a pointer to dest.

**Errors**

   EINVAL

      src or dest is NULL or src and dest overlap

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Note**

   memcpy() is usually a compiler builtin, meaning that this implementation overseed compiler's one while the symbol implementation is linked.
