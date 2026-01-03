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

typedef struct ext_stats_s {
    uint64_t arp_pkts;     /**< Number of ARP packets received */
    uint64_t echo_pkts;    /**< Number of ICMP echo requests received */
    uint64_t ip_pkts;      /**< Number of IPv4 packets received */
    uint64_t ipv6_pkts;    /**< Number of IPv6 packets received */
    uint64_t vlan_pkts;    /**< Number of VLAN packets received */
    uint64_t dropped_pkts; /**< Hyperscan dropped packets */
    uint64_t unknown_pkts; /**< Number of Unknown packets */
    uint64_t tx_failed;    /**< Transmits that failed to send */
    uint64_t imissed;      /**< Number of RX missed packets */
    uint64_t ibadcrc;      /**< Number of RX bad crc packets */
    uint64_t ibadlen;      /**< Number of RX bad length packets */
    uint64_t rx_nombuf;    /**< Number of times we could not get any mbufs */
    uint64_t max_ipackets; /**< Maximum input packets per second */
    uint64_t max_opackets; /**< Maximum output packets per second */
} ext_stats_t;

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

typedef struct port_stats_s {
    struct rte_eth_stats curr; /**< current port statistics */
    struct rte_eth_stats prev; /**< previous port statistics */
    struct rte_eth_stats rate; /**< current packet rate statistics */
    struct rte_eth_stats base; /**< base port statistics for normalization */
    ext_stats_t ext;           /**< Extended statistics */
    size_stats_t sizes;        /**< Packet sizes statistics */

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
