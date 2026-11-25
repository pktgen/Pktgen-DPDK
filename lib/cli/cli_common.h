/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2026>, Intel Corporation.
 */

#ifndef _CLI_COMMON_H_
#define _CLI_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

#include <cli_scrn.h>

/**
 * @file
 * CLI common output helpers.
 *
 * Provides cli_printf(), the primary console output routine used throughout
 * the CLI library, routed through the active cli_scrn file descriptor.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTE_ASSERT
#define RTE_ASSERT RTE_VERIFY
#endif

/**
 * printf-like routine to write formatted text to the CLI console.
 *
 * Output is written to this_scrn->fd_out and flushed immediately.
 *
 * @param fmt
 *   printf-compatible format string.
 * @param ...
 *   Variable arguments for @p fmt.
 * @return
 *   Number of characters written, as returned by vfprintf().
 */

static inline int __attribute__((format(printf, 1, 2)))
cli_printf(const char *fmt, ...)
{
    va_list vaList;
    int n;

    va_start(vaList, fmt);
    n = vfprintf(this_scrn->fd_out, fmt, vaList);
    va_end(vaList);

    fflush(this_scrn->fd_out);

    return n;
}

#ifdef __cplusplus
}
#endif

#endif /* _CLI_COMMON_H_ */
