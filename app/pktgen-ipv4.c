/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <arpa/inet.h>

#include <cli_scrn.h>
#include <lua_config.h>

#include "pktgen.h"
#include "pktgen-log.h"
#include "pktgen-ipv4.h"
#include "pktgen-txbuff.h"
#include "l2p.h"

/**
 *
 * pktgen_ipv4_ctor - Construct the IPv4 header for a packet
 *
 * DESCRIPTION
 * Constructor for the IPv4 header for a given packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_ipv4_ctor(pkt_seq_t *pkt, void *hdr)
{
    struct rte_ipv4_hdr *ip = hdr;
    uint16_t tlen;

    /* IPv4 Header constructor */
    tlen = pkt->pkt_size - pkt->ether_hdr_size;

    /* Zero out the header space */
    memset((char *)ip, 0, sizeof(struct rte_ipv4_hdr));

    ip->version_ihl = (IPv4_VERSION << 4) | (sizeof(struct rte_ipv4_hdr) / 4);

    ip->total_length    = htons(tlen);
    ip->time_to_live    = pkt->ttl;
    ip->type_of_service = pkt->tos;

    pktgen.ident += 27; /* bump by a prime number */
    ip->packet_id       = htons(pktgen.ident);
    ip->fragment_offset = 0;
    ip->next_proto_id   = pkt->ipProto;
    ip->src_addr        = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
    ip->dst_addr        = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
    ip->hdr_checksum    = 0;
    ip->hdr_checksum    = rte_ipv4_cksum((const struct rte_ipv4_hdr *)ip);
}

/**
 *
 * pktgen_send_ping4 - Create and send a Ping or ICMP echo packet.
 *
 * DESCRIPTION
 * Create a ICMP echo request packet and send the packet to a give port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_send_ping4(uint32_t pid, uint8_t seq_idx)
{
    port_info_t *pinfo = l2p_get_port_pinfo(pid);
    pkt_seq_t *ppkt    = &pinfo->seq_pkt[SPECIAL_PKT];
    pkt_seq_t *spkt    = &pinfo->seq_pkt[seq_idx];
    struct rte_mbuf *m;
    l2p_port_t *port;

    port = l2p_get_port(pid);
    if (rte_mempool_get(port->special_mp, (void **)&m)) {
        pktgen_log_warning("No packet buffers found");
        return;
    }
    *ppkt = *spkt; /* Copy the sequence setup to the ping setup. */
    pktgen_packet_ctor(pinfo, SPECIAL_PKT, ICMP4_ECHO);
    rte_memcpy(rte_pktmbuf_mtod(m, uint8_t *), (uint8_t *)ppkt->hdr, ppkt->pkt_size);

    m->pkt_len  = ppkt->pkt_size;
    m->data_len = ppkt->pkt_size;

    tx_send_packets(pinfo, l2p_get_txqid(rte_lcore_id()), &m, 1);
}

/**
 *
 * pktgen_process_ping4 - Process a input ICMP echo packet for IPv4.
 *
 * DESCRIPTION
 * Process a input packet for IPv4 ICMP echo request and send response if needed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_process_ping4(struct rte_mbuf *m, uint32_t pid, uint32_t qid, uint32_t vlan)
{
    port_info_t *pinfo        = l2p_get_port_pinfo(pid);
    struct rte_ether_hdr *eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
    struct rte_ipv4_hdr *ip   = (struct rte_ipv4_hdr *)&eth[1];
    char buff[24];

    /* Adjust for a vlan header if present */
    if (vlan)
        ip = (struct rte_ipv4_hdr *)((char *)ip + sizeof(struct rte_vlan_hdr));

    /* Look for a ICMP echo requests, but only if enabled. */
    if ((rte_atomic32_read(&pinfo->port_flags) & ICMP_ECHO_ENABLE_FLAG) &&
        (ip->next_proto_id == PG_IPPROTO_ICMP)) {
        struct rte_icmp_hdr *icmp =
            (struct rte_icmp_hdr *)((uintptr_t)ip + sizeof(struct rte_ipv4_hdr));
        uint16_t cksum = ~rte_raw_cksum(
            icmp, (m->data_len - sizeof(struct rte_ether_hdr) - sizeof(struct rte_ipv4_hdr)));
        /* We do not handle IP options, which will effect the IP header size. */
        if (unlikely(cksum != 0)) {
            pktgen_log_error("ICMP checksum failed");
            return;
        }

        if (unlikely(icmp->icmp_type == ICMP4_ECHO)) {
            int idx;

            if (ntohl(ip->dst_addr) == INADDR_BROADCAST) {
                pktgen_log_warning("IP address %s is a Broadcast",
                                   inet_ntop4(buff, sizeof(buff), ip->dst_addr, INADDR_BROADCAST));
                return;
            }

            /* Toss all broadcast addresses and requests not for this port */
            idx = pktgen_find_matching_ipsrc(pinfo, ip->dst_addr);

            /* ARP request not for this interface. */
            if (unlikely(idx == -1)) {
                pktgen_log_warning("IP address %s not found",
                                   inet_ntop4(buff, sizeof(buff), ip->dst_addr, INADDR_BROADCAST));
                return;
            }

            pinfo->pkt_stats.echo_pkts++;

            icmp->icmp_type = ICMP4_ECHO_REPLY;

            /* Recompute the ICMP checksum */
            icmp->icmp_cksum = 0;
            icmp->icmp_cksum = rte_raw_cksum(
                icmp, (m->data_len - sizeof(struct rte_ether_hdr) - sizeof(struct rte_ipv4_hdr)));

            /* Swap the IP addresses. */
            inetAddrSwap(&ip->src_addr, &ip->dst_addr);

            /* Bump the ident value */
            ip->packet_id = htons(ntohs(ip->packet_id) + m->data_len);

            /* Recompute the IP checksum */
            ip->hdr_checksum = 0;
            ip->hdr_checksum = ~rte_raw_cksum(ip, sizeof(struct rte_ipv4_hdr));

            /* Swap the MAC addresses */
            ethAddrSwap(&eth->dst_addr, &eth->src_addr);

            tx_send_packets(pinfo, qid, &m, 1);

            /* No need to free mbuf as it was reused. */
            return;
        } else if (unlikely(icmp->icmp_type == ICMP4_ECHO_REPLY))
            pinfo->pkt_stats.echo_pkts++;
    }
}
