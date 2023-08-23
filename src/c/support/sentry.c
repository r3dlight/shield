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
