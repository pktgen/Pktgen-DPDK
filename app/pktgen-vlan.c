/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include "lua_config.h"

#include "pktgen.h"
#include "pktgen-arp.h"
#include "pktgen-ipv4.h"
#include "pktgen-ipv6.h"
#include "pktgen-vlan.h"

/**
 *
 * pktgen_process_vlan - Process a VLAN packet
 *
 * DESCRIPTION
 * Process a input VLAN packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_process_vlan(struct rte_mbuf *m, uint32_t pid, uint32_t qid)
{
    pktType_e pType;
    struct rte_ether_hdr *eth;
    struct rte_vlan_hdr *rte_vlan_hdr;
    port_info_t *info = l2p_get_port_pinfo(pid);

    eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);

    /* Now dealing with the inner header */
    rte_vlan_hdr = (struct rte_vlan_hdr *)(eth + 1);

    pType = ntohs(rte_vlan_hdr->eth_proto);

    /* No support for nested tunnel */
    switch ((int)pType) {
    case RTE_ETHER_TYPE_ARP:
        info->pkt_stats.arp_pkts++;
        pktgen_process_arp(m, pid, qid, 1);
        break;
    case RTE_ETHER_TYPE_IPV4:
        info->pkt_stats.ip_pkts++;
        pktgen_process_ping4(m, pid, qid, 1);
        break;
    case RTE_ETHER_TYPE_IPV6:
        info->pkt_stats.ipv6_pkts++;
        pktgen_process_ping6(m, pid, qid, 1);
        break;
    case UNKNOWN_PACKET: /* FALL THRU */
    default:
        break;
    }
}
