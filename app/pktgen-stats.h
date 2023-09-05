/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
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

typedef struct pkt_stats_s {
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
    uint64_t rx_nombuf;    /**< Number of times we had not mbufs for Rx */
} pkt_stats_t;

typedef struct pkt_sizes_s {
    uint64_t _64;        /**< Number of 64 byte packets */
    uint64_t _65_127;    /**< Number of 65-127 byte packets */
    uint64_t _128_255;   /**< Number of 128-255 byte packets */
    uint64_t _256_511;   /**< Number of 256-511 byte packets */
    uint64_t _512_1023;  /**< Number of 512-1023 byte packets */
    uint64_t _1024_1518; /**< Number of 1024-1518 byte packets */
    uint64_t broadcast;  /**< Number of broadcast packets */
    uint64_t multicast;  /**< Number of multicast packets */
    uint64_t jumbo;      /**< Number of Jumbo frames */
    uint64_t runt;       /**< Number of Runt frames */
    uint64_t unknown;    /**< Number of unknown sizes */
} pkt_sizes_t;

struct port_info_s;

void pktgen_get_link_status(struct port_info_s *info, int pid, int wait);
void pktgen_process_stats(void);

void pktgen_page_stats(void);
void pktgen_page_phys_stats(uint16_t pid);
void pktgen_page_xstats(uint16_t pid);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_STATS_H_ */
