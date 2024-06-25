strcmp
""""""

**Name**

**Synopsys**

   .. code-block:: c

      #include <string.h>

      int strcmp(const char *s1, const char *s2);

**Description**

   The strcmp() funvtion compare two strings s1 and s2 and returns an integer indicating the result of the comparison with the following rules:

      * 0 if s1 et s2 are equal
      * a negative value if s1 is less than s2
      * a positive value if s1 is greater than s2

      In the special case of NULl arguments: if both strings are NULL, they are considered as equal. If only s1 is NULL, the result is negative. If only s2 is NULL, the result is positive.

**Return value**

   strcmp() function returns the value corresponding to the above rule.

**Errors**

   strcmp() never fails.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008, C89, C99, 4.3BSD

**Note**

   This implementation uses a secure, constant-time, implementation of the comparison.
