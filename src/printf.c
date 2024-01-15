
#include <shield/string.h>
#include <shield/stdio.h>
#include <uapi.h>
#include "printf_lexer.h"



/**
 * log_lexer delivered printf POSIX compliant implementation
 */
uint8_t print_with_len(const char *fmt, va_list *args, size_t *sizew);

uint16_t log_get_dbgbuf_offset(void);
uint8_t* log_get_dbgbuf(void);
void dbgbuffer_flush(void);


static inline void dbgbuffer_display(void)
{
    uint16_t len = log_get_dbgbuf_offset();
    if (unlikely(copy_from_user(log_get_dbgbuf(), len) != STATUS_OK)) {
        /* should not happen */
        /*@ assert(false); */
        goto err;
    }
    sys_log(len);
err:
    return;
}

/*************************************************************
 * libstream exported API implementation: POSIX compilant API
 ************************************************************/

/*
 * Linux-like printk() API (no kernel tagging by now)
 */
__attribute__ ((format (printf, 1, 2))) int shield_printf(const char *fmt, ...)
{
    va_list args;
    size_t  len;
    int res = -1;

    dbgbuffer_flush();
    if (fmt == NULL) {
        goto err;
    }
    /*
     * if there is some asyncrhonous printf to pass to the kernel, do it
     * before execute the current printf command
     */
    va_start(args, fmt);
    if (print_with_len(fmt, &args, &len) == 0) {
        res = (int)len;
    }
    va_end(args);
    if (res == -1) {
        dbgbuffer_flush();
        goto err;
    }
    /* display to debug output */
    dbgbuffer_display();
err:
    dbgbuffer_flush();
    return res;
}

#ifndef TEST_MODE
__attribute__ ((format (printf, 1, 2))) int printf(const char *fmt, ...) __attribute__((alias("shield_printf")));
#endif
