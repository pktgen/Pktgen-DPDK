/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <rte_hexdump.h>

#include "pktgen.h"
#include "pktgen-ether.h"
#include "pktgen-seq.h"
#include "pktgen-port-cfg.h"

/**
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
pktgen_ether_hdr_ctor(port_info_t *pinfo, pkt_seq_t *pkt)
{
    struct rte_ether_hdr *eth;
    uint16_t vlan_id;

    eth = &pkt->hdr->eth;

    /* src and dest addr */
    rte_ether_addr_copy(&pkt->eth_src_addr, &eth->src_addr);
    rte_ether_addr_copy(&pkt->eth_dst_addr, &eth->dst_addr);

    if (pktgen_tst_port_flags(pinfo, SEND_VLAN_ID)) {
        /* vlan ethernet header */
        eth->ether_type = htons(RTE_ETHER_TYPE_VLAN);

        /* only set the TCI field for now; don't bother with PCP/DEI */
        struct rte_vlan_hdr *rte_vlan_hdr = (struct rte_vlan_hdr *)(eth + 1);
        vlan_id                           = (pkt->vlanid | (pkt->cos << 13));
        rte_vlan_hdr->vlan_tci            = htons(vlan_id);
        rte_vlan_hdr->eth_proto           = htons(pkt->ethType);

        /* adjust header size for VLAN tag */
        pkt->ether_hdr_size = sizeof(struct rte_ether_hdr) + sizeof(struct rte_vlan_hdr);

        return (char *)(rte_vlan_hdr + 1);
    } else if (pktgen_tst_port_flags(pinfo, SEND_MPLS_LABEL)) {
        /* MPLS unicast ethernet header */
        eth->ether_type = htons(ETHER_TYPE_MPLS_UNICAST);

        mplsHdr_t *mpls_hdr = (mplsHdr_t *)(eth + 1);

        /* Only a single MPLS label is supported at the moment. Make sure the
         * BoS flag is set. */
        uint32_t mpls_label = pkt->mpls_entry;
        MPLS_SET_BOS(mpls_label);

        mpls_hdr->label = htonl(mpls_label);

        /* Adjust header size for MPLS label */
        pkt->ether_hdr_size = sizeof(struct rte_ether_hdr) + sizeof(mplsHdr_t);

        return (char *)(mpls_hdr + 1);
    } else if (pktgen_tst_port_flags(pinfo, SEND_Q_IN_Q_IDS)) {
        /* Q-in-Q ethernet header */
        eth->ether_type = htons(ETHER_TYPE_Q_IN_Q);

        qinqHdr_t *qinq_hdr = (qinqHdr_t *)(eth + 1);

        /* only set the TCI field for now; don't bother with PCP/DEI */
        qinq_hdr->qinq_tci = htons(pkt->qinq_outerid);

        qinq_hdr->vlan_tpid = htons(RTE_ETHER_TYPE_VLAN);
        qinq_hdr->vlan_tci  = htons(pkt->qinq_innerid);

        qinq_hdr->eth_proto = htons(pkt->ethType);

        /* Adjust header size for Q-in-Q header */
        pkt->ether_hdr_size = sizeof(struct rte_ether_hdr) + sizeof(qinqHdr_t);

        return (char *)(qinq_hdr + 1);
    } else {
        /* normal ethernet header */
        eth->ether_type     = htons(pkt->ethType);
        pkt->ether_hdr_size = sizeof(struct rte_ether_hdr);
    }

#ifdef TX_DEBUG_PKT
    if (eth->dst_addr.addr_bytes[0] & 1)
        rte_hexdump(stdout, "Ether", eth, sizeof(struct rte_ether_hdr));
#endif

    return (char *)(eth + 1);
}
