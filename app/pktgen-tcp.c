/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <lua_config.h>

#include "pktgen.h"

#include "pktgen-tcp.h"

/**
 *
 * pktgen_tcp_hdr_ctor - TCP header constructor routine.
 *
 * DESCRIPTION
 * Construct a TCP header in the packet buffer provided.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void *
pktgen_tcp_hdr_ctor(pkt_seq_t *pkt, void *hdr, int type, bool cksum_offload,
                    bool cksum_requires_phdr)
{
    uint16_t tlen;

    if (type == RTE_ETHER_TYPE_IPV4) {
        struct rte_ipv4_hdr *ipv4 = (struct rte_ipv4_hdr *)hdr;
        struct rte_tcp_hdr *tcp   = (struct rte_tcp_hdr *)&ipv4[1];

        /* Create the TCP header */
        ipv4->src_addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
        ipv4->dst_addr = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);

        ipv4->version_ihl   = (IPv4_VERSION << 4) | (sizeof(struct rte_ipv4_hdr) / 4);
        tlen                = pkt->pkt_size - pkt->ether_hdr_size;
        ipv4->total_length  = htons(tlen);
        ipv4->next_proto_id = pkt->ipProto;

        tcp->src_port = htons(pkt->sport);
        tcp->dst_port = htons(pkt->dport);
        tcp->sent_seq = htonl(pkt->tcp_seq);
        tcp->recv_ack = htonl(pkt->tcp_ack);
        tcp->data_off =
            ((sizeof(struct rte_tcp_hdr) / sizeof(uint32_t)) << 4); /* Offset in words */
        tcp->tcp_flags = pkt->tcp_flags;
        tcp->rx_win    = htons(DEFAULT_WND_SIZE);
        tcp->tcp_urp   = 0;

        tcp->cksum = 0;
        if (!cksum_offload)
            tcp->cksum = rte_ipv4_udptcp_cksum(ipv4, (const void *)tcp);
        else if (cksum_offload && cksum_requires_phdr)
            tcp->cksum = rte_ipv4_phdr_cksum(ipv4, 0);

    } else {
        struct rte_ipv6_hdr *ipv6 = (struct rte_ipv6_hdr *)hdr;
        struct rte_tcp_hdr *tcp   = (struct rte_tcp_hdr *)&ipv6[1];

        /* Create the pseudo header and TCP information */
        memset(&ipv6->dst_addr, 0, sizeof(struct rte_ipv6_addr));
        memset(&ipv6->src_addr, 0, sizeof(struct rte_ipv6_addr));
        rte_memcpy(&ipv6->dst_addr, &pkt->ip_dst_addr.addr.ipv6, sizeof(struct rte_ipv6_addr));
        rte_memcpy(&ipv6->src_addr, &pkt->ip_src_addr.addr.ipv6, sizeof(struct rte_ipv6_addr));

        tlen              = pkt->pkt_size - (pkt->ether_hdr_size + sizeof(struct rte_ipv6_hdr));
        ipv6->payload_len = htons(tlen);
        ipv6->proto       = pkt->ipProto;

        tcp->src_port = htons(pkt->sport);
        tcp->dst_port = htons(pkt->dport);
        tcp->sent_seq = htonl(pkt->tcp_seq);
        tcp->recv_ack = htonl(pkt->tcp_ack);
        tcp->data_off =
            ((sizeof(struct rte_tcp_hdr) / sizeof(uint32_t)) << 4); /* Offset in words */
        tcp->tcp_flags = pkt->tcp_flags;
        tcp->rx_win    = htons(DEFAULT_WND_SIZE);
        tcp->tcp_urp   = 0;

        tcp->cksum = 0;
        if (!cksum_offload)
            tcp->cksum = rte_ipv6_udptcp_cksum(ipv6, (const void *)tcp);
    }

    /* In this case we return the original value to allow IP ctor to work */
    return hdr;
}
