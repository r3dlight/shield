// SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

#ifndef __SORT_H
#define __SORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <string.h>
#include <shield/private/coreutils.h>

/** \addtogroup sort
 *  @{
 */

/**
 * @def sort swap function prototype
 *
 * If the caller need a specific swap function, it must respect the following
 * prototype.
 * @param a[out]: first cell to swap
 * @param b[out]: second cell to swap
 * @param size[in]: cell size in bytes
 */
typedef void (*swap_func_t)(void *a, void *b, size_t size);

/**
 * @def sort compare function prototype
 *
 * Always required. Specify the way to compare a and b
 *
 * @param a[in]: first cell to compare
 * @param b[in]: second cell to compare
 *
 * @returns:
 *   A positive value if a is bigger than b, a negative value if
 *   a is smaller than b, or 0 if they are equal
 */
typedef int (*cmp_func_t)(const void *a, const void *b);

/**
 * @brief generic swap function
 *
 * basically exchange two cells of same size
 */
static inline void generic_swap(void *a, void *b, size_t size)
{
    uint8_t buf[size];
    memcpy(&buf[0], b, size);
    memcpy(b, a, size);
    memcpy(a, &buf[0], size);
}

/**
 * @brief simple, yet generic, bubble sort for all kernel tables
 *
 * @param table[out]: the table to sort
 * @param len[in]: number of cells in the table
 * @param cell_size[in]: size of a cell in bytes
 * @param cmp[in]: comparison function, required
 * @param swp[in]: swap function. If NULL, fallback to generic_swap
 */
static inline int bubble_sort(void *table, size_t len, size_t cell_size, cmp_func_t cmp, swap_func_t swp)
{
    int status = -1;
    if (unlikely(table == NULL || cmp == NULL)) {
        goto end;
    }
    if (unlikely(len < 2 || cell_size == 0)) {
        status = 0;
        /* nothing to be done */
        goto end;
    }
    if (swp == NULL) {
        swp = generic_swap;
    }
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len - 1 - i; j++) {
            size_t ta = (size_t)table + (j*cell_size);
            size_t tb = (size_t)table + ((j+1)*cell_size);
            if (cmp((void*)ta, (void*)tb) > 0) {
                swp((void*)ta, (void*)tb, cell_size);
            }
        }
    }
    status = 0;
end:
    return status;
}


/** \addtogroup sort
 *  @}
 */

#ifdef __cplusplus
}
#endif

#endif/*!__SORT_H*/
