/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <lua_config.h>
#include <rte_arp.h>

#include "pktgen-arp.h"

#include "pktgen.h"
#include "pktgen-cmds.h"
#include "pktgen-log.h"
#include "pktgen-txbuff.h"

void
arp_pkt_dump(struct rte_mbuf *m)
{
    struct rte_ether_hdr *eth;
    struct rte_arp_hdr *arp;
    char dst[64], src[64];
    char sip[64], tip[64];

    eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
    arp = rte_pktmbuf_mtod_offset(m, struct rte_arp_hdr *, sizeof(struct rte_ether_hdr));

    printf("\nARP Packet Dump\n");

    rte_ether_format_addr(dst, sizeof(dst), &eth->dst_addr);
    rte_ether_format_addr(src, sizeof(src), &eth->src_addr);
    printf("  Ethernet Header DST: %s, SRC: %s, Type: %04x\n", dst, src, ntohs(eth->ether_type));

    printf("  ARP Header Type: %04x, Proto: %04x, hlen: %d, plen: %d, opcode: %d\n",
           ntohs(arp->arp_hardware), ntohs(arp->arp_protocol), arp->arp_hlen, arp->arp_plen,
           ntohs(arp->arp_opcode));

    rte_ether_format_addr(dst, sizeof(dst), &arp->arp_data.arp_sha);
    rte_ether_format_addr(src, sizeof(src), &arp->arp_data.arp_tha);
    inet_ntop(AF_INET, &arp->arp_data.arp_sip, sip, sizeof(sip));
    inet_ntop(AF_INET, &arp->arp_data.arp_tip, tip, sizeof(tip));
    printf("  ARP Data Sender: %s-%s, Target: %s-%s\n", dst, sip, src, tip);
}

/**
 *
 * pktgen_send_arp - Send an ARP request packet.
 *
 * DESCRIPTION
 * Create and send an ARP request packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_send_arp(uint32_t pid, uint32_t type, uint8_t seq_idx)
{
    port_info_t *info = &pktgen.info[pid];
    pkt_seq_t *pkt;
    struct rte_mbuf *m;
    struct rte_ether_hdr *eth;
    struct rte_arp_hdr *arp;
    uint32_t addr;

    pkt = &info->seq_pkt[seq_idx];
    m   = rte_pktmbuf_alloc(info->special_mp);
    if (unlikely(m == NULL)) {
        pktgen_log_warning("No packet buffers found");
        return;
    }
    eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
    arp = (struct rte_arp_hdr *)&eth[1];

    /* src and dest addr */
    memset(&eth->dst_addr, 0xFF, 6);
    rte_ether_addr_copy(&pkt->eth_src_addr, &eth->src_addr);
    eth->ether_type = htons(RTE_ETHER_TYPE_ARP);

    memset(arp, 0, sizeof(struct rte_arp_hdr));

    rte_memcpy(&arp->arp_data.arp_sha, &pkt->eth_src_addr, 6);
    addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
    inetAddrCopy(&arp->arp_data.arp_sip, &addr);

    if (likely(type == GRATUITOUS_ARP)) {
        rte_memcpy(&arp->arp_data.arp_tha, &pkt->eth_src_addr, 6);
        addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
        inetAddrCopy(&arp->arp_data.arp_tip, &addr);
    } else {
        memset(&arp->arp_data.arp_tha, 0, 6);
        addr = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
        inetAddrCopy(&arp->arp_data.arp_tip, &addr);
    }

    /* Fill in the rest of the ARP packet header */
    arp->arp_hardware = htons(ETH_HW_TYPE);
    arp->arp_protocol = htons(RTE_ETHER_TYPE_IPV4);
    arp->arp_hlen     = 6;
    arp->arp_plen     = 4;
    arp->arp_opcode   = htons(ARP_REQUEST);

    m->pkt_len  = 60;
    m->data_len = 60;

    tx_buffer(info->q[0].txbuff, m);
}

/**
 *
 * pktgen_process_arp - Handle a ARP request input packet and send a response.
 *
 * DESCRIPTION
 * Handle a ARP request input packet and send a response if required.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_process_arp(struct rte_mbuf *m, uint32_t pid, uint32_t qid, uint32_t vlan)
{
    port_info_t *info         = &pktgen.info[pid];
    pkt_seq_t *pkt            = NULL;
    struct rte_ether_hdr *eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
    struct rte_arp_hdr *arp   = (struct rte_arp_hdr *)&eth[1];

    /* Adjust for a vlan header if present */
    if (vlan)
        arp = (struct rte_arp_hdr *)((char *)arp + sizeof(struct rte_vlan_hdr));

    /* Process all ARP requests if they are for us. */
    if (arp->arp_opcode == htons(ARP_REQUEST)) {
        int idx;

        if (arp->arp_data.arp_tip == arp->arp_data.arp_sip) { /* GARP Packet */
            idx = pktgen_find_matching_ipdst(info, arp->arp_data.arp_sip);

            /* Found a matching packet, replace the dst address */
            if (idx >= 0) {
                rte_memcpy(&pkt->eth_dst_addr, &arp->arp_data.arp_sha, 6);
                pktgen_clear_display();
            }
            return;
        }

        idx = pktgen_find_matching_ipsrc(info, arp->arp_data.arp_tip);

        /* ARP request not for this interface. */
        if (likely(idx >= 0)) {
            struct rte_mbuf *m1;

            pkt = &info->seq_pkt[idx];
            m1  = rte_pktmbuf_copy(m, info->special_mp, 0, UINT32_MAX);
            if (unlikely(m1 == NULL))
                return;
            eth = rte_pktmbuf_mtod(m1, struct rte_ether_hdr *);
            arp = (struct rte_arp_hdr *)&eth[1];

            /* Grab the source MAC address as the destination address for the port. */
            if (unlikely(pktgen.flags & MAC_FROM_ARP_FLAG)) {
                rte_memcpy(&pkt->eth_dst_addr, &arp->arp_data.arp_sha, 6);
                for (uint32_t i = 0; i < info->seqCnt; i++)
                    pktgen_packet_ctor(info, i, -1);
            }

            /* Swap the two MAC addresses */
            ethAddrSwap(&arp->arp_data.arp_sha, &arp->arp_data.arp_tha);

            /* Swap the two IP addresses */
            inetAddrSwap(&arp->arp_data.arp_tip, &arp->arp_data.arp_sip);

            /* Set the packet to ARP reply */
            arp->arp_opcode = htons(ARP_REPLY);

            /* Swap the MAC addresses */
            ethAddrSwap(&eth->dst_addr, &eth->src_addr);

            /* Copy in the MAC address for the reply. */
            rte_memcpy(&arp->arp_data.arp_sha, &pkt->eth_src_addr, 6);
            rte_memcpy(&eth->src_addr, &pkt->eth_src_addr, 6);

            m1->ol_flags = 0;

            tx_buffer(info->q[qid].txbuff, m1);
            tx_buffer_flush(info->q[qid].txbuff);
            return;
        }
    } else if (arp->arp_opcode == htons(ARP_REPLY)) {
        int idx;

        idx = pktgen_find_matching_ipsrc(info, arp->arp_data.arp_tip);

        /* ARP request not for this interface. */
        if (likely(idx >= 0)) {
            pkt = &info->seq_pkt[idx];

            /* Grab the real destination MAC address */
            if (pkt->ip_dst_addr.addr.ipv4.s_addr == ntohl(arp->arp_data.arp_sip))
                rte_memcpy(&pkt->eth_dst_addr, &arp->arp_data.arp_sha, 6);

            pktgen.flags |= PRINT_LABELS_FLAG;
        }
    }
}
