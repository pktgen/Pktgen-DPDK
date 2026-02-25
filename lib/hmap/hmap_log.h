/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2025-2026 Intel Corporation
 */

#pragma once

/**
 * @file
 *
 * HMAP Logs API
 *
 * This file provides a log API to HMAP applications.
 */

#include <stdio.h>         // for NULL
#include <stdarg.h>        // for va_list
#include <stdint.h>        // for uint32_t
#include <rte_common.h>
#include <rte_log.h>
#include <rte_branch_prediction.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Log an INFO-level message to the USER2 facility. */
#define HMAP_LOG(args...) RTE_LOG(INFO, USER2, ##args)
/** Log an ERR-level message to the USER2 facility. */
#define HMAP_ERR(args...) RTE_LOG(ERR, USER2, ##args)
/** Log a WARNING-level message to the USER2 facility. */
#define HMAP_WARN(args...) RTE_LOG(WARNING, USER2, ##args)
/** Log a DEBUG-level message to the USER2 facility. */
#define HMAP_DEBUG(args...) RTE_LOG(DEBUG, USER2, ##args)

/**
 * Log a message at a caller-specified level to the USER2 facility.
 *
 * @param l
 *   DPDK log level (e.g. INFO, ERR, DEBUG) passed to RTE_LOG.
 * @param args
 *   Format string followed by variable arguments, as in printf(3).
 */
#define HMAP_PRINT(l, args...) RTE_LOG(l, USER2, ##args)

/**
 * Generate an Error log message and return value
 *
 * Same as HMAP_LOG(ERR,...) define, but returns -1 to enable this style of coding.
 *   if (val == error) {
 *       HMAP_ERR("Error: Failed\n");
 *       return -1;
 *   }
 * Returning _val  to the calling function.
 */
#define HMAP_ERR_RET_VAL(_val, ...)       \
    do {                                  \
        RTE_LOG(ERR, USER2, __VA_ARGS__); \
        return _val;                      \
    } while ((0))

/**
 * Generate an Error log message and return
 *
 * Same as HMAP_LOG(ERR,...) define, but returns to enable this style of coding.
 *   if (val == error) {
 *       HMAP_ERR("Error: Failed\n");
 *       return;
 *   }
 * Returning to the calling function.
 */
#define HMAP_RET(...) HMAP_ERR_RET_VAL(, __VA_ARGS__)

/**
 * Generate an Error log message and return -1
 *
 * Same as HMAP_LOG(ERR,...) define, but returns -1 to enable this style of coding.
 *   if (val == error) {
 *       HMAP_ERR("Error: Failed\n");
 *       return -1;
 *   }
 * Returning a -1 to the calling function.
 */
#define HMAP_ERR_RET(...) HMAP_ERR_RET_VAL(-1, __VA_ARGS__)

/**
 * Generate an Error log message and return NULL
 *
 * Same as HMAP_LOG(ERR,...) define, but returns NULL to enable this style of coding.
 *   if (val == error) {
 *       HMAP_ERR("Error: Failed\n");
 *       return NULL;
 *   }
 * Returning a NULL to the calling function.
 */
#define HMAP_NULL_RET(...) HMAP_ERR_RET_VAL(NULL, __VA_ARGS__)

/**
 * Generate a Error log message and goto label
 *
 * Same as HMAP_LOG(ERR,...) define, but goes to a label to enable this style of coding.
 *   if (error condition) {
 *       HMAP_ERR("Error: Failed\n");
 *       goto lbl;
 *   }
 */
#define HMAP_ERR_GOTO(lbl, ...)           \
    do {                                  \
        RTE_LOG(ERR, USER2, __VA_ARGS__); \
        goto lbl;                         \
    } while ((0))

#ifdef __cplusplus
}
#endif
