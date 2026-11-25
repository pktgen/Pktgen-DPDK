/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_STATS_H_
#define _PKTGEN_STATS_H_

/**
 * @file
 *
 * Per-port statistics structures and display functions for Pktgen.
 *
 * Defines queue-level, size-histogram, extended, and aggregate port
 * statistics structs, and declares the functions that collect and display them.
 */

#include <rte_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_QUEUES_PER_PORT 32 /**< Maximum number of RX/TX queues per port */

/** Per-queue packet counters. */
typedef struct qstats_s {
    uint64_t q_ipackets; /**< Number of input packets */
    uint64_t q_opackets; /**< Number of output packets */
    uint64_t q_errors;   /**< Number of error packets */
} qstats_t;

/** Packet size histogram counters. */
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

/** Extended NIC statistics snapshot. */
typedef struct xstats_s {
    struct rte_eth_xstat_name *names; /**< Array of extended stat name strings */
    struct rte_eth_xstat *xstats;     /**< Current extended stat values */
    struct rte_eth_xstat *prev;       /**< Previous extended stat values (for rate calc) */
    int cnt;                          /**< Number of extended stats entries */
} xstats_t;

/** Aggregate per-port statistics (current, previous, rate, and base). */
typedef struct port_stats_s {
    struct rte_eth_stats curr; /**< current port statistics */
    struct rte_eth_stats prev; /**< previous port statistics */
    struct rte_eth_stats rate; /**< current packet rate statistics */
    struct rte_eth_stats base; /**< base port statistics for normalization */
    xstats_t xstats;           /**< Extended statistics */

    size_stats_t sizes; /**< Packet size counters */

    qstats_t qstats[MAX_QUEUES_PER_PORT];      /**< Hot-path: written only by worker lcores */
    qstats_t snap_qstats[MAX_QUEUES_PER_PORT]; /**< Snapshot: written only by timer thread */
    qstats_t prev_qstats[MAX_QUEUES_PER_PORT]; /**< Previous snapshot for rate calculation */
} port_stats_t;

struct port_info_s;

/** Query and update the Ethernet link status for a port. */
void pktgen_get_link_status(struct port_info_s *info);

/** Collect and compute per-port packet rates on a timer tick. */
void pktgen_process_stats(void);

/** Render the main statistics display page. */
void pktgen_page_stats(void);

/**
 * Render the per-queue statistics page for a port.
 *
 * @param pid  Port ID to display.
 */
void pktgen_page_qstats(uint16_t pid);

/**
 * Render the extended NIC statistics page for a port.
 *
 * @param pid  Port ID to display.
 */
void pktgen_page_xstats(uint16_t pid);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_STATS_H_ */
