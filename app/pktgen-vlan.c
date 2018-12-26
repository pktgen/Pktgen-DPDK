/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include "rte_lua.h"

#include "pktgen.h"
#include "pktgen-arp.h"
#include "pktgen-ipv4.h"
#include "pktgen-ipv6.h"
#include "pktgen-vlan.h"

/**************************************************************************//**
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
pktgen_process_vlan(struct rte_mbuf *m, uint32_t pid)
{
	pktType_e pType;
	struct ether_hdr *eth;
	struct vlan_hdr  *vlan_hdr;
	port_info_t      *info = &pktgen.info[pid];

	eth = rte_pktmbuf_mtod(m, struct ether_hdr *);

	/* Now dealing with the inner header */
	vlan_hdr = (struct vlan_hdr *)(eth + 1);

	pType = ntohs(vlan_hdr->eth_proto);

	/* No support for nested tunnel */
	switch ((int)pType) {
	case ETHER_TYPE_ARP:
		info->stats.arp_pkts++;
		pktgen_process_arp(m, pid, 1);
		break;
	case ETHER_TYPE_IPv4:
		info->stats.ip_pkts++;
		pktgen_process_ping4(m, pid, 1);
		break;
	case ETHER_TYPE_IPv6:
		info->stats.ipv6_pkts++;
		pktgen_process_ping6(m, pid, 1);
		break;
	case UNKNOWN_PACKET:	/* FALL THRU */
	default:
		break;
	}
}
