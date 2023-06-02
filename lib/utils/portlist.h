/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2023> Intel Corporation.
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
#define INVALID_PORTLIST ((portlist_t)-1)

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
 * Parse a portmasl string into a mask or bitmap value.
 *
 * @param str
 *   String to parse
 * @param portlist
 *   Pointer to uint64_t value for returned bitmap
 * @return
 *   -1 on error or 0 on success.
 */
int portmask_parse(const char *str, portlist_t *portmask);

char *portlist_string(uint64_t portlist, char *buf, int len);
char *portlist_print(FILE *f, uint64_t portlist, char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __PORTLIST_H_ */
