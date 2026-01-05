/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2025-2025 Intel Corporation
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

#define HMAP_LOG(args...)   RTE_LOG(INFO, USER2, ##args)
#define HMAP_ERR(args...)   RTE_LOG(ERR, USER2, ##args)
#define HMAP_WARN(args...)  RTE_LOG(WARNING, USER2, ##args)
#define HMAP_DEBUG(args...) RTE_LOG(DEBUG, USER2, ##args)

/**
 * Generates a log message regardless of log level.
 *
 * @param f
 *   The fmt string, as in printf(3), followed by the variable arguments
 *   required by the format.
 * @param args
 *   Variable arguments depend on Application.
 * @return
 *   - The number of characters printed on success.
 *   - A negative value on error.
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
