#include <pthread.h>

pthread_t pthread_self(void)
{
#if CONFIG_WITH_SENTRY
    return __libc_get_current_threadid();
#else
    return 0;
#endif
}
