// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <shield/private/coreutils.h>
#include <uapi.h>
#include "sentry.h"

/**
 * return the current thread identifier
 */
uint8_t __libc_get_current_threadid(void)
{
  return 0;
  /*
   * FIXME: arch specific: based on PSP address & CONFIG_STACK_SIZE, get back
   * the stack root address, and return the initial frame first argument,
   * corresponding to the current thread identifier
   */
}

/* sentry syscalls upper layer */

static inline uint32_t sentry_get_entropy(void)
{
  uint32_t entropy;
  /* if the task do not have SYS_RANDOM permission, falling back to
   * POSIX random using SSP seed as initial seed value */
  if (__sys_get_random() != STATUS_OK) {
    /* get back for SSP backed GNU libC compliant BSD rand() */
    entropy = (uint32_t)rand();
  } else {
    copy_from_kernel((uint8_t*)&entropy, sizeof(uint32_t));
  }
  return entropy;
}

/**
 * @brief get entropy from kernel TRNG if possible, or SSP-derivated otherwise
 */
int get_entropy(unsigned char *in, uint16_t len)
{
  uint32_t entropy;
  size_t offset = 0;
  int res = -1;

  if (unlikely(in == NULL)) {
    goto end;
  }
  if (unlikely(_memarea_is_worldaligned(in) == false)) {
    /* realign first */
    entropy = sentry_get_entropy();
    for (offset = 0; (offset = (__WORDSIZE - ((size_t)&in[0]) % __WORDSIZE)); ++offset) {
      in[offset] = (uint8_t)(entropy & 0xff);
      entropy >>= 8;
    }
  }
  /* in[offset] is world align. copy with __WORDSIZE chunks */
  for (; offset < len; offset += sizeof(uint32_t)) {
    entropy = sentry_get_entropy();
    if ((unlikely((len - offset) < sizeof(uint32_t)))) {
      /* residual */
      memcpy(&in[offset], &entropy, (len - offset));
    } else {
      /* chunk */
      memcpy(&in[offset], &entropy, sizeof(uint32_t));
    }
  }
  res = len;
end:
  return res;
}
