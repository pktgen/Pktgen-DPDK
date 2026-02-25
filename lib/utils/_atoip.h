/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2026> Intel Corporation.
 */

/**
 * @file
 *
 * String-related utility functions for IP addresses
 */

#ifndef __ATOIP_H_
#define __ATOIP_H_

#include <netinet/in.h>

#include <rte_string_fns.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTE_IPADDR_V4      0x01 /**< Flag: address is IPv4 */
#define RTE_IPADDR_V6      0x02 /**< Flag: address is IPv6 */
#define RTE_IPADDR_NETWORK 0x04 /**< Flag: parse as network/prefix notation */

#define RTE_INADDRSZ    4   /**< Size of an IPv4 address in bytes */
#define RTE_IN6ADDRSZ   16  /**< Size of an IPv6 address in bytes */
#define RTE_PREFIXMAX   128 /**< Maximum IPv6 prefix length in bits */
#define RTE_V4PREFIXMAX 32  /**< Maximum IPv4 prefix length in bits */

/** Holds a parsed IPv4 or IPv6 address, optionally with a prefix length. */
struct rte_ipaddr {
    uint8_t family; /**< Address family: AF_INET or AF_INET6 */
    union {
        struct in_addr ipv4;       /**< IPv4 address (when family == AF_INET) */
        struct rte_ipv6_addr ipv6; /**< IPv6 address (when family == AF_INET6) */
    };
    unsigned int prefixlen; /**< Network prefix length in bits (0 if not a network address) */
};

/**
 * Convert an IPv4/v6 address into a binary value.
 *
 * @param buf
 *   Location of string to convert
 * @param flags
 *   Set of flags for converting IPv4/v6 addresses and netmask.
 * @param res
 *   Location to put the results
 * @param ressize
 *   Length of res in bytes.
 * @return
 *   4 or 6 on OK, indicating an IPv4/v6 address, respectively, and -1 on error
 */
int _atoip(const char *buf, int flags, void *res, unsigned ressize);

#ifdef __cplusplus
}
#endif

#endif /* __ATOIP_H_ */
