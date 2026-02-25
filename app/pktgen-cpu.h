/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CPU_H_
#define _PKTGEN_CPU_H_

/**
 * @file
 *
 * CPU topology initialisation and display page for Pktgen.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise the CPU topology information for all detected lcores.
 *
 * Populates socket, core, and thread IDs for each lcore in the
 * global coreinfo table.
 */
void pktgen_cpu_init(void);

/**
 * Render the CPU topology display page to the console.
 */
void pktgen_page_cpu(void);

/**
 * Look up the lcore ID for a given socket/core/thread combination.
 *
 * @param s
 *   NUMA socket ID.
 * @param c
 *   Physical core ID within the socket.
 * @param t
 *   Hyper-thread (sibling thread) index within the core.
 * @return
 *   Logical core (lcore) ID on success, or 0 if not found.
 */
static inline uint16_t
sct(uint8_t s, uint8_t c, uint8_t t)
{
    coreinfo_t *ci;

    for (uint16_t i = 0; i < coreinfo_lcore_cnt(); i++) {
        ci = coreinfo_get(i);

        if (ci->socket_id == s && ci->core_id == c && ci->thread_id == t)
            return ci->lcore_id;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CPU_H_ */
