strlen
""""""

**Name**

   strlen, strnlen - calculate the length of a string

**Synopsys**

   .. code-block:: c

       #include <string.h>

       size_t strlen(const char *s);

       size_t strnlen(const char *s, size_t maxlen);

**Description**

   strlen() function calculates the length of a string pointed by s, excluding the '\0' termination character.
   strnlen() functions returns the number of character of s, upto maxlen. The result may then be shorter than the effective string size if maxlen is less than the number of bytes of s.

**Return value**

   strlen() returns the number of bytes found in the string.
   strnlen() returns at most maxlen, being the effective length or s or a truncation of the original size to maxlen.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008

**Note**

   strlen() should be avoided, replaced by strnlen(), so that unterminated strings do not lead to undefined behavior (including task memory fault).
