printf
""""""

**Name**

    printf, snprintf - formatted output conversion

**Synopsys**


   .. code-block:: c
       #include <stdio.h>

       int printf(const char *format, ...);
       int snprintf(char *str, size_t size, const char *format, ...);

**Description**

    The  functions  in  the  printf()  family  produce  output  according  to a format as described below. The function printf() write to standard OS serial output, while snprintf() write at most *size* characters into the given (preallocated) *str* argument.


    Format of the format string

       The  format  string is a character string, beginning and ending in its initial shift state, if any.  The format string is composed of zero or more directives: ordinary characters (not %), which are copied unchanged to the output stream; and conversion  specifications,  each of which results in fetching zero or more subsequent arguments.  Each conversion specification is introduced by the character %, and ends with a conversion specifier.  In between there may be (in this order) zero or more flags, an optional  minimum field width, an optional precision and an optional length modifier.

       The  arguments  must correspond properly (after type promotion) with the conversion specifier. The arguments are used in the order given.

    Flag character

        0: zero-padded the given numerical value. The digit set just after the '0' character define the number width, including the 0-padding. For number having a length bigger than th declared max padded value, no 0-padding is made.


    Conversion specifier

        d,i: numerical, signed value (integer) is printed. The argument is of type int or equivalent

        o,x: numerical signed value in octal or hexadecimal form. hexadecimal form do not print the '0x' prefix. The argument must be an integer type or equivalent

        c: character, of standard C type char

        s: a null-terminated standard C string format, can be const

        p: a pointer. the address is printed in hexadecimal form with 0x prefix

        %: the '%' character itself. For e.g. "%%" print '%'

        
**Return value**

    Upon  successful  return,  these  functions return the number of characters printed (excluding the null byte used to end output to  strings).

**Errors**

   EINVAL

      the clockid is invalid or fp is NULL

   EPERM

      the clock measurement syscall failed

**Conforming to**

   Partial conformance (not all POSIX features implemented)

   POSIX.1-2001, POSIX.1-2008, C89 (printf), C99 (snprintf)

