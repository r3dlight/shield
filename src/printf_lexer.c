// SPDX-FileCopyrightText: 2023 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

/**
 * @file printf relative lexer implementation
 *
 * NOTE: this file is made to be fully generic (i.e. no dependency on kernel
 * specific headers) so that it can be used by the autotest app too, avoiding
 * code redundancy for a printf() implementation, which is POSIX-compliant.
 *
 * autotest printf() (see autotest print.c) and Sentry printk() are POSIX-based
 * and as such use the very same lexer methodology, which is lead by the standard
 * POSIX PSE51-1 stdio definition.
 *
 * WARN: do NOT add sentry specific headers or types here.
 */

#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

/*********************************************
 * Ring buffer and ring buffer utility functions
 */

// FIXME: must be Kconfig based
#define BUF_MAX 128

/*
 * .bss based, init to 0
 */
typedef struct debug_buffer {
    uint16_t offset;
    char buf[BUF_MAX];
} debug_buffer_t;

static debug_buffer_t dbgbuf;


uint16_t log_get_dbgbuf_offset(void)
{
    return dbgbuf.offset;
}

uint8_t* log_get_dbgbuf(void)
{
    return &dbgbuf.buf[0];
}

void dbgbuffer_flush(void)
{
    memset(&dbgbuf.buf[0], 0x0, BUF_MAX);
    dbgbuf.offset = 0;
}


/*
 * add a char in the ring buffer.
 *
 * INFO: by now, there is no bound check here. As a consequence, if
 * the ring buffer is full,
 *
 * WARNING: this function is the only one holding the ring buffer full
 * flag detection. As a consequence, any write access to the ring buffer
 * must be done through this function *exclusively*.
 */
static inline void dbgbuffer_write_char(const char c)
{
    /* if the buffer is full when we try to put char in it,
     * the char is discared, waiting for the ring buffer to be flushed.
     */
    if (dbgbuf.offset == BUF_MAX) {
        goto end;
    }
    dbgbuf.buf[dbgbuf.offset++] = c;
 end:
    return;
}

/*
 * Write a digit to the ring buffer.
 * This function convert a basic digit into a printable one.
 *
 * This function support usual bases such as binary
 *
 * INFO: this function can write digits in base up to hexadecimal one.
 * Bases bigger than hex are not supported.
 *
 */
static inline void dbgbuffer_write_digit(uint8_t digit)
{
    if (digit < 0xa) {
        digit += '0';
        dbgbuffer_write_char(digit);
    } else if (digit <= 0xf) {
        digit += 'a' - 0xa;
        dbgbuffer_write_char(digit);
    }
}

/*
 * copy a string to the ring buffer. This is an abstraction of the
 * dbgbuffer_write_char() function.
 *
 * This function is a helper function above dbgbuffer_write_char().
 */
 /*@
   requires \valid_read(str);
  */
static inline void dbgbuffer_write_string(char *str, uint32_t len)
{
    for (uint32_t i = 0; (i < len) && (str[i]); ++i) {
        dbgbuffer_write_char(str[i]);
    }
    return;
}


static uint8_t number[64];
/*
 * Write a number in the ring buffer.
 * This function is a helper function above dbgbuffer_write_char().
 */
static inline void dbgbuffer_write_u64(uint64_t value, const uint32_t base)
{
    /* we define a local storage to hold the digits list
     * in any possible base up to base 2 (64 bits) */

    uint8_t     index = 0;
    if ((base != 10) && (base != 8) && (base != 16)) {
        goto end;
    }

    for (; (value / base) != 0; value /= base) {
        number[index++] = value % base;
        if (index >= 64) {
            goto end;
        }
    }

    /* finishing with most significant unit */
    number[index] = value % base;

    /* now we can print out, starting with the most significant unit */
    for (; ; index--) {
        dbgbuffer_write_digit(number[index]);
        if (index == 0) {
            goto end;
        }
    }
end:
    return;
}

static inline void dbgbuffer_write_u32(uint32_t value, const uint32_t base)
{
    dbgbuffer_write_u64((uint64_t)value, base);
}

static inline void dbgbuffer_write_u16(uint16_t value, const uint32_t base)
{
    dbgbuffer_write_u64((uint64_t)value, base);
}


/*********************************************
 * other, not ring-buffer associated local utility functions
 */

/*
 * Return the number of digits of the given number, considering
 * the base in which the number is encoded.
 */
static uint8_t dbgbuffer_get_number_len(unsigned long value, uint8_t base)
{
    /* at least, if value is 0, its lenght is 1 digit */
    uint8_t len = 1;

    /* now we calculate the number of digits in the number */
    for (; (value / base) != 0; value /= base) {
        len++;
    }
    return len;
}

/**************************************************
 * printf lexer implementation
 *************************************************/

typedef enum {
    FS_NUM_DECIMAL,
    FS_NUM_HEX,
    FS_NUM_UCHAR,
    FS_NUM_SHORT,
    FS_NUM_LONG,
    FS_NUM_UNSIGNEDLONG,
    FS_NUM_LONGLONG,
    FS_NUM_UNSIGNEDLONGLONG,
    FS_NUM_UNSIGNED,
} fs_num_mode_t;

typedef struct {
    bool    attr_0len;
    bool    attr_size;
    uint8_t size;
    fs_num_mode_t numeric_mode;
    bool    started;
    uint8_t consumed;
    uint32_t strlen;
} fs_properties_t;


/*
 * Handle one format string (starting with '%' char).
 *
 * This function transform a format string into an effective content using given
 * va_list argument.
 *
 * The function updated the consumed argument with the number of char consumed
 * by the format string itself, and return 0 if the format string has been
 * correctly parsed, or 1 if the format string parsing failed.
 */
static uint8_t print_handle_format_string(const char *fmt, va_list *args,
                                          uint8_t * consumed,
                                          uint32_t * out_str_len)
{
    uint8_t status = 1; /* invalid */
    fs_properties_t fs_prop = {
        .attr_0len = false,
        .attr_size = false,
        .size = 0,
        .numeric_mode = FS_NUM_DECIMAL, /*default */
        .started = false,
        .consumed = 0,
        .strlen = 0
    };

    /*
     * Sanitation
     */
    if (!fmt || !consumed) {
        return 1;
    }

    /* Let parse the format string ... */
    do {
        /*
         * Handling '%' character
         */
        switch (fmt[fs_prop.consumed]) {
            case '%':
                {
                    if (fs_prop.started == false) {
                        /* starting string format parsing */
                        fs_prop.started = true;
                    } else if (fs_prop.consumed == 1) {
                        /* detecting '%' just after '%' */
                        dbgbuffer_write_char('%');
                        fs_prop.strlen++;
                        /* => end of format string */
                        goto end;
                    } else {
                        /* invalid: there is content before two '%' chars
                         * in the same format_string (e.g. %02%) */
                        goto err;
                    }
                    break;
                }
            case '0':
                {
                    /*
                     * Handling '0' character
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.attr_0len = true;
                    /* 0 must be completed with size content. We check it now */
                    while ((fmt[fs_prop.consumed + 1] >= '0') &&
                           (fmt[fs_prop.consumed + 1] <= '9')) {
                        /* getting back the size. Here only decimal values are handled */
                        fs_prop.size =
                            (fs_prop.size * 10) +
			    (fmt[fs_prop.consumed + 1] - '0');
                        fs_prop.consumed++;
                    }
                    /* if digits have been found after the 0len format string, attr_size is
                     * set to true
                     */
                    if (fs_prop.size != 0) {
                        fs_prop.attr_size = true;
                    }
                    break;
                }
            case 'd':
            case 'i':
                {
                    /*
                     * Handling integers
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.numeric_mode = FS_NUM_DECIMAL;
                    int     val = va_arg(*args, int);
                    uint8_t len = dbgbuffer_get_number_len(val, 10);

                    if (val < 0 ) {
                        dbgbuffer_write_char('-');
                        val = -val;
                        fs_prop.strlen++;
                    }
                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        /* we have to pad with 0 the number to reach
                         * the desired size */
                        for (uint32_t i = len; i < fs_prop.size; ++i) {
                            dbgbuffer_write_char('0');
                            fs_prop.strlen++;
                        }
                    }
                    /* now we can print the number in argument */
                    dbgbuffer_write_u32((uint32_t)val, 10);
                    fs_prop.strlen += len;
                    /* => end of format string */
                    goto end;
                }
            case 'l':
                {
                    /*
                     * Handling long and long long int
                     */
                    long    lval = 0;
                    long long llval = 0;
                    unsigned long    luval = 0;
                    unsigned long long lluval = 0;
                    uint8_t len = 0;

                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.numeric_mode = FS_NUM_LONG;
                    /* detecting long long */
                    if (fmt[fs_prop.consumed + 1] == 'l') {
                        fs_prop.numeric_mode = FS_NUM_LONGLONG;
                        fs_prop.consumed++;
                    }
                    /* detecting long (long) unsigned */
                    if (fmt[fs_prop.consumed + 1] == 'u') {
                        if (fs_prop.numeric_mode == FS_NUM_LONG) {
                            fs_prop.numeric_mode = FS_NUM_UNSIGNEDLONG;
                        } else {
                            fs_prop.numeric_mode = FS_NUM_UNSIGNEDLONGLONG;
                        }
                        fs_prop.consumed++;
                    }
                    switch (fs_prop.numeric_mode) {
                        case FS_NUM_LONG:
                            lval = va_arg(*args, long);
                            len = dbgbuffer_get_number_len(lval, 10);
                            break;
                        case FS_NUM_LONGLONG:
                            llval = va_arg(*args, long long);
                            len = dbgbuffer_get_number_len(llval, 10);
                            break;
                        case FS_NUM_UNSIGNEDLONG:
                            luval = va_arg(*args, unsigned long);
                            len = dbgbuffer_get_number_len(luval, 10);
                            break;
                        case FS_NUM_UNSIGNEDLONGLONG:
                            lluval = va_arg(*args, unsigned long long);
                            len = dbgbuffer_get_number_len(lluval, 10);
                            break;
                        default:
                            __builtin_unreachable();
                            break;
                    }
                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        /* we have to pad with 0 the number to reach
                         * the desired size */
                        for (uint32_t i = len; i < fs_prop.size; ++i) {
                            dbgbuffer_write_char('0');
                            fs_prop.strlen++;
                        }
                    }
                    /* now we can print the number in argument */
                    switch (fs_prop.numeric_mode) {
                        case FS_NUM_LONG:
                            dbgbuffer_write_u32(lval, 10);
                            break;
                        case FS_NUM_LONGLONG:
                            dbgbuffer_write_u64(llval, 10);
                            break;
                        case FS_NUM_UNSIGNEDLONG:
                            dbgbuffer_write_u32(luval, 10);
                            break;
                        case FS_NUM_UNSIGNEDLONGLONG:
                            dbgbuffer_write_u64(lluval, 10);
                            break;
                        default:
                            __builtin_unreachable();
                            break;
                    }
                    fs_prop.strlen += len;
                    /* => end of format string */
                    goto end;
                }
            case 'h':
                {
                    /*
                     * Handling long and long long int
                     */
                    short   s_val = 0;
                    unsigned char uc_val = 0;
                    uint8_t len = 0;

                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.numeric_mode = FS_NUM_SHORT;
                    /* detecting long long */
                    if (fmt[fs_prop.consumed + 1] == 'h') {
                        fs_prop.numeric_mode = FS_NUM_UCHAR;
                        fs_prop.consumed++;
                    }
                    if (fs_prop.numeric_mode == FS_NUM_SHORT) {
                        s_val = (short) va_arg(*args, int);

                        len = dbgbuffer_get_number_len(s_val, 10);
                    } else {
                        uc_val = (unsigned char) va_arg(*args, int);

                        len = dbgbuffer_get_number_len(uc_val, 10);
                    }
                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        /* we have to pad with 0 the number to reach
                         * the desired size */
                        for (uint32_t i = len; i < fs_prop.size; ++i) {
                            dbgbuffer_write_char('0');
                            fs_prop.strlen++;
                        }
                    }
                    /* now we can print the number in argument */
                    if (fs_prop.numeric_mode == FS_NUM_SHORT) {
                        dbgbuffer_write_u16((uint16_t)s_val, 10);
                    } else {
                        dbgbuffer_write_u16((uint16_t)uc_val, 10);
                    }
                    fs_prop.strlen += len;
                    /* => end of format string */
                    goto end;
                }
            case 'u':
                {
                    /*
                     * Handling unsigned
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.numeric_mode = FS_NUM_UNSIGNED;
                    uint32_t val = va_arg(*args, uint32_t);
                    uint8_t len = dbgbuffer_get_number_len(val, 10);

                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        /* we have to pad with 0 the number to reach
                         * the desired size */
                        for (uint32_t i = len; i < fs_prop.size; ++i) {
                            dbgbuffer_write_char('0');
                            fs_prop.strlen++;
                        }
                    }
                    /* now we can print the number in argument */
                    dbgbuffer_write_u32(val, 10);
                    fs_prop.strlen += len;
                    /* => end of format string */
                    goto end;
                }
            case 'p':
                {
                    /*
                     * Handling pointers. Include 0x prefix, as if using
                     * %#x format string in POSIX printf.
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    unsigned long val = va_arg(*args, unsigned long);
                    uint8_t len = dbgbuffer_get_number_len(val, 16);

                    dbgbuffer_write_string("0x", 2);
                    for (uint32_t i = len; i < fs_prop.size; ++i) {
                        dbgbuffer_write_char('0');
                        fs_prop.strlen++;
                    }
                    /* now we can print the number in argument */
                    dbgbuffer_write_u64(val, 16);
                    fs_prop.strlen += len;
                    /* => end of format string */
                    goto end;
                }

            case 'x':
                {
                    /*
                     * Handling hexadecimal
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.numeric_mode = FS_NUM_UNSIGNED;
                    uint32_t val = va_arg(*args, uint32_t);
                    uint8_t len = dbgbuffer_get_number_len(val, 16);

                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        /* we have to pad with 0 the number to reach
                         * the desired size */
                        for (uint32_t i = len; i < fs_prop.size; ++i) {
                            dbgbuffer_write_char('0');
                            fs_prop.strlen++;
                        }
                    }
                    /* now we can print the number in argument */
                    dbgbuffer_write_u32(val, 16);
                    fs_prop.strlen += len;
                    /* => end of format string */
                    goto end;
                }
            case 'o':
                {
                    /*
                     * Handling octal
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    fs_prop.numeric_mode = FS_NUM_UNSIGNED;
                    uint32_t val = va_arg(*args, uint32_t);
                    uint8_t len = dbgbuffer_get_number_len(val, 8);

                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        /* we have to pad with 0 the number to reach
                         * the desired size */
                        for (uint32_t i = len; i < fs_prop.size; ++i) {
                            dbgbuffer_write_char('0');
                            fs_prop.strlen++;
                        }
                    }
                    /* now we can print the number in argument */
                    dbgbuffer_write_u32(val, 8);
                    fs_prop.strlen += len;

                    /* => end of format string */
                    goto end;
                }
            case 's':
                {
                    /*
                     * Handling strings
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    /* no size or 0len attribute for strings */
                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        goto err;
                    }
                    char   *str = va_arg(*args, char *);
                    if (str == NULL) {
                        dbgbuffer_write_string("(null)", 6);
                        fs_prop.strlen += 6;
                    } else {
                        /* now we can print the number in argument. The strnlen is
                           more a best practice than a real protection as whatever the size is,
                           the buffer never write more than its own size */
                        dbgbuffer_write_string(str, strnlen(str, BUF_MAX));
                        fs_prop.strlen += strnlen(str, BUF_MAX);
                    }
                    /* => end of format string */
                    goto end;
                }
            case 'c':
                {
                    /*
                     * Handling chars
                     */
                    if (fs_prop.started == false) {
                        goto err;
                    }
                    /* no size or 0len attribute for strings */
                    if (fs_prop.attr_size && fs_prop.attr_0len) {
                        goto err;
                    }
                    unsigned char val = (unsigned char) va_arg(*args, int);

                    /* now we can print the number in argument */
                    dbgbuffer_write_char(val);

                    /* => end of format string */
                    goto end;
                }

                /* none of the above. Unsupported format */
            default:
                /* should not happend, unable to parse format string */
                goto err;

        }
        fs_prop.consumed++;
    } while (fmt[fs_prop.consumed]);
end:
    status = 0;
err:
    *out_str_len += fs_prop.strlen;
    *consumed = fs_prop.consumed + 1;   /* consumed is starting with 0 */
    return status;
}


/*
 * Print a given fmt string, considering variable arguments given in args.
 * This function *does not* flush the ring buffer, but only fullfill it.
 */
/*@
   requires \valid_read(fmt);
   requires \valid(sizew);
*/
uint8_t print_with_len(const char *fmt, va_list *args, size_t *sizew)
{
    uint8_t status = 1;
    int     i = 0;
    uint8_t consumed = 0;
    uint32_t out_str_s = 0;

    while (fmt[i]) {
        if (fmt[i] == '%') {
            status = print_handle_format_string(&(fmt[i]), args, &consumed, &out_str_s);
            if (status != 0) {
                /* the string format parsing has failed ! */
                goto err;
            }
            i += consumed;
            consumed = 0;
        } else {
            out_str_s++;
            dbgbuffer_write_char(fmt[i++]);
        }
    }
    status = 0;
err:
    *sizew = out_str_s;
    return status;
}
