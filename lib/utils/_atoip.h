/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2023> Intel Corporation.
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

#define RTE_IPADDR_V4      0x01
#define RTE_IPADDR_V6      0x02
#define RTE_IPADDR_NETWORK 0x04

#define RTE_INADDRSZ    4
#define RTE_IN6ADDRSZ   16
#define RTE_PREFIXMAX   128
#define RTE_V4PREFIXMAX 32

struct rte_ipaddr {
    uint8_t family;
    union {
        struct in_addr ipv4;
        struct in6_addr ipv6;
    };
    unsigned int prefixlen; /* in case of network only */
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
