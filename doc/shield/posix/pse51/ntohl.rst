ntohl
"""""

**Name**

   htonl, ntohl, htons,ntohs - convert values between host and network byte order

**Synopsys**

   .. code-block:: c

      #include <arpa/inet.h>

      uint32_t htonl(uint32_t hostlong);

      uint16_t htons(uint16_t hostshort);

      uint32_t ntohl(uint32_t netlong);

      uint16_t ntohs(uint16_t netshort);


**Description**

   The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.

   The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.

   The ntohl() function converts the unsigned integer netlong from network byte order to host byte order.

   The ntohs() function converts the unsigned short integer netshort from network byte order to host byte order.

   On the armv7m architecture, the default byte order is Least Significant Byte first, but be configured using a Core register. The network byte order, as used on the Internet, is always Most Significant Byte first.

**Return value**

   htonl() and htons() return the network byte order encoded value of the function argument, using the same type (uint32_t or uint16_t). The hton(ls) function autodetect the argument byte encoding of the current host. ntohl() and ntohs() return the host-encoded value of the argument, considered as in network byte order encoding.

**Conforming to**

   POSIX.1-2001, POSIX.1-2008
