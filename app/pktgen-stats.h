/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_STATS_H_
#define _PKTGEN_STATS_H_

#include <rte_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_QUEUES_PER_PORT 32        // RTE_MAX_QUEUES_PER_PORT

typedef struct qstats_s {
    uint64_t q_ipackets; /**< Number of input packets */
    uint64_t q_opackets; /**< Number of output packets */
    uint64_t q_errors;   /**< Number of error packets */
} qstats_t;

typedef struct size_stats_s {
    uint64_t _64;        /**< Number of 64 byte packets */
    uint64_t _65_127;    /**< Number of 65-127 byte packets */
    uint64_t _128_255;   /**< Number of 128-255 byte packets */
    uint64_t _256_511;   /**< Number of 256-511 byte packets */
    uint64_t _512_1023;  /**< Number of 512-1023 byte packets */
    uint64_t _1024_1522; /**< Number of 1024-1522 byte packets */
    uint64_t broadcast;  /**< Number of broadcast packets */
    uint64_t multicast;  /**< Number of multicast packets */
    uint64_t jumbo;      /**< Number of Jumbo frames */
    uint64_t runt;       /**< Number of Runt frames */
    uint64_t unknown;    /**< Number of unknown sizes */
} size_stats_t;

typedef struct xstats_s {
    struct rte_eth_xstat_name *names;
    struct rte_eth_xstat *xstats;
    struct rte_eth_xstat *prev;
    int cnt;
} xstats_t;

typedef struct port_stats_s {
    struct rte_eth_stats curr; /**< current port statistics */
    struct rte_eth_stats prev; /**< previous port statistics */
    struct rte_eth_stats rate; /**< current packet rate statistics */
    struct rte_eth_stats base; /**< base port statistics for normalization */
    xstats_t xstats;           /**< Extended statistics */

    size_stats_t sizes; /**< Packet size counters */

    qstats_t qstats[MAX_QUEUES_PER_PORT];      /**< Current queue stats */
    qstats_t prev_qstats[MAX_QUEUES_PER_PORT]; /**< Previous queue stats to determine rate */
} port_stats_t;

struct port_info_s;

void pktgen_get_link_status(struct port_info_s *info);
void pktgen_process_stats(void);
void pktgen_page_stats(void);
void pktgen_page_qstats(uint16_t pid);
void pktgen_page_xstats(uint16_t pid);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_STATS_H_ */
