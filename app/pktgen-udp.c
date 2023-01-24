/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include "lua_config.h"

#include "pktgen.h"

#include "pktgen-udp.h"

/**
 *
 * pktgen_udp_hdr_ctor - UDP header constructor routine.
 *
 * DESCRIPTION
 * Construct the UDP header in a packer buffer.
 *
 * RETURNS: next header location
 *
 * SEE ALSO:
 */

void *
pktgen_udp_hdr_ctor(pkt_seq_t *pkt, void *hdr, int type)
{
    uint16_t tlen;

    if (type == RTE_ETHER_TYPE_IPV4) {
        struct rte_ipv4_hdr *ipv4 = hdr;
        struct rte_udp_hdr *udp   = (struct rte_udp_hdr *)&ipv4[1];

        /* Create the UDP header */
        ipv4->src_addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
        ipv4->dst_addr = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);

        ipv4->version_ihl   = (IPv4_VERSION << 4) | (sizeof(struct rte_ipv4_hdr) / 4);
        tlen                = pkt->pktSize - pkt->ether_hdr_size;
        ipv4->total_length  = htons(tlen);
        ipv4->next_proto_id = pkt->ipProto;

        tlen           = pkt->pktSize - (pkt->ether_hdr_size + sizeof(struct rte_ipv4_hdr));
        udp->dgram_len = htons(tlen);
        udp->src_port  = htons(pkt->sport);
        udp->dst_port  = htons(pkt->dport);

        if (pkt->dport == VXLAN_PORT_ID) {
            struct vxlan *vxlan = (struct vxlan *)&udp[1];

            vxlan->vni_flags = htons(pkt->vni_flags);
            vxlan->group_id  = htons(pkt->group_id);
            vxlan->vxlan_id  = htonl(pkt->vxlan_id) << 8;
        }

        udp->dgram_cksum = 0;
        udp->dgram_cksum = rte_ipv4_udptcp_cksum(ipv4, (const void *)udp);
        if (udp->dgram_cksum == 0)
            udp->dgram_cksum = 0xFFFF;
    } else {
        struct rte_ipv6_hdr *ipv6 = hdr;
        struct rte_udp_hdr *udp   = (struct rte_udp_hdr *)&ipv6[1];

        /* Create the pseudo header and TCP information */
        memset(ipv6->dst_addr, 0, sizeof(struct in6_addr));
        memset(ipv6->src_addr, 0, sizeof(struct in6_addr));
        rte_memcpy(ipv6->dst_addr, &pkt->ip_dst_addr.addr.ipv6.s6_addr, sizeof(struct in6_addr));
        rte_memcpy(ipv6->src_addr, &pkt->ip_src_addr.addr.ipv6.s6_addr, sizeof(struct in6_addr));

        tlen              = pkt->pktSize - (pkt->ether_hdr_size + sizeof(struct rte_ipv6_hdr));
        ipv6->payload_len = htons(tlen);
        ipv6->proto       = pkt->ipProto;

        udp->dgram_len = htons(tlen);
        udp->src_port  = htons(pkt->sport);
        udp->dst_port  = htons(pkt->dport);

        udp->dgram_cksum = 0;
        udp->dgram_cksum = rte_ipv6_udptcp_cksum(ipv6, (const void *)udp);
        if (udp->dgram_cksum == 0)
            udp->dgram_cksum = 0xFFFF;
    }

    /* Return the original pointer for IP ctor */
    return hdr;
}
