strcpy
""""""

**Name**

   strcpy - copy a string

**Synopsys**

   .. code-block:: c

       #include <string.h>

       char *strcpy(char *dest, const char *src);

**Description**

   The strcpy() function copy the string pointed by src into dest, including the termination '\0' character.

**Return value**

   strcpy() returns a pointer to dest.

**Errors**

   EINVAL

      src and dest overlap, based on the src length basis.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Bugs**

   In the same way as the standard POSIX strcpy() implementation does, the Shield strcpy() is not able to determine that dest is large enough to support the copy of src. As a consequence, if dest is not large enough, everything may happen. Be carefull when using this function.

**Note**

  The shield strcpy() implementation check overlapping of src and dest, based on src length.
