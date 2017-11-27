/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ETHER_H_
#define _ETHER_H_

/**
 * @file
 *
 * Ethernet Helpers in RTE
 */

#ifdef __cplusplus
extern "C" {
#endif
#include <rte_ether.h>
#if 0
/**
 * Convert a string Ethernet MAC address to the binary form
 *
 * @param a
 *   String containing the MAC address in two forms
 *      XX:XX:XX:XX:XX:XX or XXXX:XXXX:XXX
 * @param e
 *   pointer to a struct ether_addr to place the return value. If the value
 *   is null then use a static location instead.
 * @return
 *   Pointer to the struct ether_addr structure;
 */
static inline struct ether_addr *
rte_ether_aton(const char *a, struct ether_addr *e)
{
    int i;
    char *end;
    unsigned long o[ETHER_ADDR_LEN];
    static struct ether_addr ether_addr;

    if (!e)
        e = &ether_addr;

    i = 0;
    do {
        errno = 0;
        o[i] = strtoul(a, &end, 16);
        if (errno != 0 || end == a || (end[0] != ':' && end[0] != 0))
            return NULL;
        a = end + 1;
    } while (++i != sizeof (o) / sizeof (o[0]) && end[0] != 0);

    /* Junk at the end of line */
    if (end[0] != 0)
        return NULL;

    /* Support the format XX:XX:XX:XX:XX:XX */
    if (i == ETHER_ADDR_LEN) {
        while (i-- != 0) {
            if (o[i] > UINT8_MAX)
                return NULL;
            e->addr_bytes[i] = (uint8_t)o[i];
        }
    /* Support the format XXXX:XXXX:XXXX */
    } else if (i == ETHER_ADDR_LEN / 2) {
        while (i-- != 0) {
            if (o[i] > UINT16_MAX)
                return NULL;
            e->addr_bytes[i * 2] = (uint8_t)(o[i] >> 8);
            e->addr_bytes[i * 2 + 1] = (uint8_t)(o[i] & 0xff);
        }
    /* unknown format */
    } else
        return NULL;

    return e;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ETHER_H_ */
