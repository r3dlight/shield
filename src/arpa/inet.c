// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

/**
 * @file in.c
 *
 * standard POSIX implementation of endianess management, as specified in
 * POSIX.1-2001 and POSIX.1-2008 standard API
 */
#include <shield/arpa/inet.h>

#define BSWAP32(__bsx) \
    ((((__bsx) & 0xff000000u) >> 24) | (((__bsx) & 0x00ff0000u) >> 8) | (((__bsx) & 0x0000ff00u) << 8) | (((__bsx) & 0x000000ffu) << 24))

#define BSWAP16(__bs) \
    ((((__bs) & 0xff00u) >> 8) | (((__bs) & 0x00ff) << 8))


uint32_t htonl(uint32_t x)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#if defined(__has_builtin) && __has_builtin(__builtin_bswap32)
    return __builtin_bswap32(x);
#else
    return BSWAP32(x);
#endif/*! has builtin */
#else
    return x;
#endif
}

uint32_t ntohl(uint32_t x) __attribute__((alias("htonl")));

uint16_t htons(uint16_t x)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#if defined(__has_builtin) && __has_builtin(__builtin_bswap16)
    return __builtin_bswap16(x);
#else
    return BSWAP16(x);
#endif/*! has builtin */
#else
    return x;
#endif
}

uint16_t ntohs(uint16_t x) __attribute__((alias("htons")));
