/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/**
 * @file
 *
 * String-related utility function for parsing port mask.
 */

#ifndef _RTE_PORTLIST_H_
#define _RTE_PORTLIST_H_

#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include <rte_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t portlist_t;

/**
 * Parse a portlist string into a mask or bitmap value.
 *
 * @param str
 *   String to parse
 * @param portlist
 *   Pointer to uint64_t value for returned bitmap
 * @return
 *   -1 on error or 0 on success.
 */
int __rte_experimental rte_parse_portlist(const char *str, portlist_t *portlist);

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
int __rte_experimental rte_parse_portmask(const char *str, portlist_t *portmask);

char * __rte_experimental rte_portlist_string(uint64_t portlist, char *buf, int len);
char * __rte_experimental rte_print_portlist(FILE *f, uint64_t portlist,
	char *buf, int len);



#ifdef __cplusplus
}
#endif

#endif /* _RTE_PORTLIST_H_ */
