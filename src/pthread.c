#include <shield/pthread.h>

#if CONFIG_WITH_SENTRY
# include <support/sentry.h>
#endif

pthread_t pthread_self(void)
{
#if CONFIG_WITH_SENTRY
    return __libc_get_current_threadid();
#else
    return 0;
#endif
}
