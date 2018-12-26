/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
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
 * RTE Command line interface
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTE_ASSERT
#define RTE_ASSERT	RTE_VERIFY
#endif

/**
 * CLI printf like routine to write on the console.
 *
 * @note Uses thread variable this_cli.
 *
 * @param va_args
 *   va_args for the rest of the printf ouput.
 * @return
 *   N/A
 */

static inline int
__attribute__((format(printf, 1, 2)))
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
