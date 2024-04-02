#ifndef TOOLS_H
#define TOOLS_H

#include <stdbool.h>
#include <stddef.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x)   __builtin_expect(!!(x), 1)

#ifndef __WORDSIZE
#define __WORDSIZE (sizeof(void*))
#endif

static inline bool _memarea_is_worldaligned(const void * memarea)
{
    return (((size_t)memarea % __WORDSIZE) == 0);
}

#endif/*!TOOLS_H*/
