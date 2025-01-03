// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t htonl(uint32_t x);

uint32_t ntohl(uint32_t x);

uint16_t htons(uint16_t x);

uint16_t ntohs(uint16_t x);

#if defined(__cplusplus)
}
#endif

#endif/*!ARPA_INET_H*/
