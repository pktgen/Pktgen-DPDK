/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include "pktgen-ether.h"
#include "pktgen-seq.h"
#include "pktgen-port-cfg.h"

/**************************************************************************//**
 *
 * pktgen_ether_hdr_ctor - Ethernet header constructor routine.
 *
 * DESCRIPTION
 * Construct the ethernet header for a given packet buffer.
 *
 * RETURNS: Pointer to memory after the ethernet header.
 *
 * SEE ALSO:
 */

char *
pktgen_ether_hdr_ctor(port_info_t *info, pkt_seq_t *pkt, struct ether_hdr *eth)
{
	uint32_t flags;
	uint16_t vlan_id;
	/* src and dest addr */
	ether_addr_copy(&pkt->eth_src_addr, &eth->s_addr);
	ether_addr_copy(&pkt->eth_dst_addr, &eth->d_addr);

	flags = rte_atomic32_read(&info->port_flags);
	if (flags & SEND_VLAN_ID) {
		/* vlan ethernet header */
		eth->ether_type = htons(ETHER_TYPE_VLAN);

		/* only set the TCI field for now; don't bother with PCP/DEI */
		struct vlan_hdr *vlan_hdr = (struct vlan_hdr *)(eth + 1);
		vlan_id = (pkt->vlanid | (pkt->cos << 13));
		vlan_hdr->vlan_tci = htons(vlan_id);
		vlan_hdr->eth_proto = htons(pkt->ethType);

		/* adjust header size for VLAN tag */
		pkt->ether_hdr_size = sizeof(struct ether_hdr) +
			sizeof(struct vlan_hdr);

		return (char *)(vlan_hdr + 1);
	} else if (flags & SEND_MPLS_LABEL) {
		/* MPLS unicast ethernet header */
		eth->ether_type = htons(ETHER_TYPE_MPLS_UNICAST);

		mplsHdr_t *mpls_hdr = (mplsHdr_t *)(eth + 1);

		/* Only a single MPLS label is supported at the moment. Make sure the
		 * BoS flag is set. */
		uint32_t mpls_label = pkt->mpls_entry;
		MPLS_SET_BOS(mpls_label);

		mpls_hdr->label = htonl(mpls_label);

		/* Adjust header size for MPLS label */
		pkt->ether_hdr_size = sizeof(struct ether_hdr) +
			sizeof(mplsHdr_t);

		return (char *)(mpls_hdr + 1);
	} else if (flags & SEND_Q_IN_Q_IDS) {
		/* Q-in-Q ethernet header */
		eth->ether_type = htons(ETHER_TYPE_Q_IN_Q);

		qinqHdr_t *qinq_hdr = (qinqHdr_t *)(eth + 1);

		/* only set the TCI field for now; don't bother with PCP/DEI */
		qinq_hdr->qinq_tci = htons(pkt->qinq_outerid);

		qinq_hdr->vlan_tpid = htons(ETHER_TYPE_VLAN);
		qinq_hdr->vlan_tci = htons(pkt->qinq_innerid);

		qinq_hdr->eth_proto = htons(pkt->ethType);

		/* Adjust header size for Q-in-Q header */
		pkt->ether_hdr_size = sizeof(struct ether_hdr) +
			sizeof(qinqHdr_t);

		return (char *)(qinq_hdr + 1);
	} else {
		/* normal ethernet header */
		eth->ether_type = htons(pkt->ethType);
		pkt->ether_hdr_size = sizeof(struct ether_hdr);
	}

	return (char *)(eth + 1);
}
