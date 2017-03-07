/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#ifdef __cplusplus
}
#endif

#endif /* _ETHER_H_ */
