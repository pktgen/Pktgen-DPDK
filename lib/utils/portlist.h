/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
 */

/**
 * @file
 *
 * String-related utility function for parsing port mask.
 */

#ifndef __PORTLIST_H_
#define __PORTLIST_H_

#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include <rte_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t portlist_t;
#define INVALID_PORTLIST ((portlist_t) - 1)

/**
 * Parse a portlist string into a mask or bitmap value.
 *
 * @param str
 *   String to parse
 * @param nb_ports
 *   Max number of ports to set in the portlist
 * @param portlist
 *   Pointer to uint64_t value for returned bitmap
 * @return
 *   -1 on error or 0 on success.
 */
int portlist_parse(const char *str, int nb_ports, portlist_t *portlist);

/**
 * Parse a portmask string into a mask or bitmap value.
 *
 * @param str
 *   String to parse
 * @param portmask
 *   Pointer to uint64_t value for returned bitmap
 * @return
 *   -1 on error or 0 on success.
 */
int portmask_parse(const char *str, portlist_t *portmask);

/**
 * Convert a portlist bitmap to a human-readable string (e.g. "0,1,3-5").
 *
 * @param portlist
 *   64-bit bitmap of port indices to serialise.
 * @param buf
 *   Destination buffer for the result string.
 * @param len
 *   Size of @p buf in bytes.
 * @return
 *   Pointer to @p buf on success, or NULL on error.
 */
char *portlist_string(uint64_t portlist, char *buf, int len);

/**
 * Convert a portlist bitmap to a string and print it to file @p f.
 *
 * @param f
 *   Output file (e.g. stdout).
 * @param portlist
 *   64-bit bitmap of port indices to serialise.
 * @param buf
 *   Scratch buffer used to build the string.
 * @param len
 *   Size of @p buf in bytes.
 * @return
 *   Pointer to @p buf on success, or NULL on error.
 */
char *portlist_print(FILE *f, uint64_t portlist, char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __PORTLIST_H_ */
