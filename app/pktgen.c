/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
TXlib */

/* Created 2010 by Keith Wiles @ intel.com */

#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>

#include <pg_delay.h>
#include <rte_lcore.h>
#include <lua_config.h>
#include <rte_net.h>
#include <rte_arp.h>
#include <rte_cycles.h>
#include <rte_hexdump.h>

#include "pktgen.h"
#include "pktgen-gre.h"
#include "pktgen-tcp.h"
#include "pktgen-ipv4.h"
#include "pktgen-ipv6.h"
#include "pktgen-udp.h"
#include "pktgen-arp.h"
#include "pktgen-vlan.h"
#include "pktgen-cpu.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"
#include "pktgen-gtpu.h"
#include "pktgen-sys.h"

#include <pthread.h>
#include <sched.h>

/* Allocated the pktgen structure for global use */
pktgen_t pktgen;

double
next_poisson_time(double rateParameter)
{
    return -logf(1.0f - ((double)random()) / (double)(RAND_MAX)) / rateParameter;
}

#ifdef TX_DEBUG
/* dump a mbuf on console */
static void
pg_pktmbuf_dump(FILE *f, const struct rte_mbuf *m, unsigned dump_len)
{
    unsigned int len;
    unsigned int nb_segs;

    fprintf(f, "dump mbuf at %p, iova=%#" PRIx64 ", buf_len=%u, mempool '%s'\n", m,
            rte_mbuf_iova_get(m), m->buf_len, m->pool->name);
    fprintf(f, "  pkt_len=%u, ol_flags=%#" PRIx64 ", nb_segs=%u, port=%u", m->pkt_len, m->ol_flags,
            m->nb_segs, m->port);

    if (m->ol_flags & (RTE_MBUF_F_RX_QINQ | RTE_MBUF_F_TX_QINQ))
        fprintf(f, ", vlan_tci_outer=%u", m->vlan_tci_outer);

    if (m->ol_flags & (RTE_MBUF_F_RX_VLAN | RTE_MBUF_F_TX_VLAN))
        fprintf(f, ", vlan_tci=%u", m->vlan_tci);

    fprintf(f, ", ptype=%#" PRIx32 "\n", m->packet_type);

    nb_segs = m->nb_segs;

    while (m && nb_segs != 0) {
        __rte_mbuf_sanity_check(m, 0);

        fprintf(f, "  segment at %p, data=%p, len=%u, off=%u, refcnt=%u\n", m,
                rte_pktmbuf_mtod(m, void *), m->data_len, m->data_off, rte_mbuf_refcnt_read(m));

        len = dump_len;
        if (len > m->data_len)
            len = m->data_len;
        if (len != 0)
            rte_hexdump(f, NULL, rte_pktmbuf_mtod(m, void *), len);
        dump_len -= len;
        m = m->next;
        nb_segs--;
    }
}
#endif

/**
 *
 * wire_size - Calculate the wire size of the data in bits to be sent.
 *
 * DESCRIPTION
 * Calculate the number of bytes/bits in a burst of traffic.
 *
 * RETURNS: Number of bytes in a burst of packets.
 *
 * SEE ALSO:
 */
static uint64_t
pktgen_wire_size(port_info_t *pinfo)
{
    uint64_t i, size = 0;

    if (pktgen_tst_port_flags(pinfo, SEND_PCAP_PKTS)) {
        pcap_info_t *pcap = l2p_get_pcap(pinfo->pid);

        size = WIRE_SIZE(pcap->max_pkt_size, uint64_t);
    } else {
        if (unlikely(pinfo->seqCnt > 0)) {
            for (i = 0; i < pinfo->seqCnt; i++)
                size += WIRE_SIZE(pinfo->seq_pkt[i].pkt_size, uint64_t);
            size = size / pinfo->seqCnt; /* Calculate the average sized packet */
        } else
            size = WIRE_SIZE(pinfo->seq_pkt[SINGLE_PKT].pkt_size, uint64_t);
    }
    return (size * 8);
}

/**
 *
 * pktgen_packet_rate - Calculate the transmit rate.
 *
 * DESCRIPTION
 * Calculate the number of cycles to wait between sending bursts of traffic.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_packet_rate(port_info_t *port)
{
    uint64_t link_speed, wire_size, pps, cpb;

    wire_size = pktgen_wire_size(port);
    if (port->link.link_speed == 0) {
        port->tx_cycles = 0;
        port->tx_pps    = 0;
        return;
    }

    link_speed = (uint64_t)port->link.link_speed * Million;
    pps        = (((link_speed / wire_size) * ((port->tx_rate == 0) ? 1.0 : port->tx_rate)) / 100);
    pps        = ((pps > 0) ? pps : 1);
    cpb        = (rte_get_timer_hz() / pps) * (uint64_t)port->tx_burst; /* Cycles per Burst */

    port->tx_cycles = (uint64_t)l2p_get_txcnt(port->pid) * cpb;
    port->tx_pps    = pps;
}

/**
 *
 * pktgen_fill_pattern - Create the fill pattern in a packet buffer.
 *
 * DESCRIPTION
 * Create a fill pattern based on the arguments for the packet data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static __inline__ void
pktgen_fill_pattern(uint8_t *p, uint16_t len, uint32_t type, char *user)
{
    uint32_t i;

    switch (type) {
    case USER_FILL_PATTERN:
        memset(p, 0, len);
        for (i = 0; i < len; i++)
            p[i] = user[i & (USER_PATTERN_SIZE - 1)];
        break;

    case NO_FILL_PATTERN:
        break;

    case ZERO_FILL_PATTERN:
        memset(p, 0, len);
        break;

    default:
    case ABC_FILL_PATTERN: /* Byte wide ASCII pattern */
        for (i = 0; i < len; i++)
            p[i] = "abcdefghijklmnopqrstuvwxyz012345"[i & 0x1f];
        break;
    }
}

/**
 *
 * pktgen_find_matching_ipsrc - Find the matching IP source address
 *
 * DESCRIPTION
 * locate and return the pkt_seq_t pointer to the match IP address.
 *
 * RETURNS: index of sequence packets or -1 if no match found.
 *
 * SEE ALSO:
 */
int
pktgen_find_matching_ipsrc(port_info_t *pinfo, uint32_t addr)
{
    int i, ret = -1;
    uint32_t mask;

    addr = ntohl(addr);

    /* Search the sequence packets for a match */
    for (i = 0; i < pinfo->seqCnt; i++)
        if (addr == pinfo->seq_pkt[i].ip_src_addr.addr.ipv4.s_addr) {
            ret = i;
            break;
        }

    mask = size_to_mask(pinfo->seq_pkt[SINGLE_PKT].ip_src_addr.prefixlen);

    /* Now try to match the single packet address */
    if (ret == -1 ||
        (addr & mask) == (pinfo->seq_pkt[SINGLE_PKT].ip_dst_addr.addr.ipv4.s_addr * mask))
        ret = SINGLE_PKT;

    return ret;
}

/**
 *
 * pktgen_find_matching_ipdst - Find the matching IP destination address
 *
 * DESCRIPTION
 * locate and return the pkt_seq_t pointer to the match IP address.
 *
 * RETURNS: index of sequence packets or -1 if no match found.
 *
 * SEE ALSO:
 */
int
pktgen_find_matching_ipdst(port_info_t *pinfo, uint32_t addr)
{
    int i, ret = -1;

    addr = ntohl(addr);

    /* Search the sequence packets for a match */
    for (i = 0; i < pinfo->seqCnt; i++)
        if (addr == pinfo->seq_pkt[i].ip_dst_addr.addr.ipv4.s_addr) {
            ret = i;
            break;
        }

    /* Now try to match the single packet address */
    if (ret == -1 && addr == pinfo->seq_pkt[SINGLE_PKT].ip_dst_addr.addr.ipv4.s_addr)
        ret = SINGLE_PKT;

    /* Now try to match the range packet address */
    if (ret == -1 && addr == pinfo->seq_pkt[RANGE_PKT].ip_dst_addr.addr.ipv4.s_addr)
        ret = RANGE_PKT;

    return ret;
}

static __inline__ tstamp_t *
pktgen_tstamp_pointer(port_info_t *pinfo __rte_unused, char *p)
{
    int offset = 0;

    offset += sizeof(struct rte_ether_hdr);
    offset += sizeof(struct rte_ipv4_hdr);
    offset += sizeof(struct rte_udp_hdr);
    offset = (offset + sizeof(uint64_t)) & ~(sizeof(uint64_t) - 1);

    return (tstamp_t *)(p + offset);
}

static inline void
pktgen_tstamp_inject(port_info_t *pinfo, uint16_t qid)
{
    pkt_seq_t *pkt = &pinfo->seq_pkt[LATENCY_PKT];
    rte_mbuf_t *mbuf;
    l2p_port_t *port;
    uint16_t sent;

    uint64_t curr_ts = pktgen_get_time();
    latency_t *lat   = &pinfo->latency;

    if (curr_ts >= lat->latency_timo_cycles) {
        lat->latency_timo_cycles = curr_ts + lat->latency_rate_cycles;

        port = l2p_get_port(pinfo->pid);
        if (rte_mempool_get(port->special_mp, (void **)&mbuf) == 0) {
            uint16_t pktsize = pkt->pkt_size;
            uint16_t to_send;

            mbuf->pkt_len  = pktsize;
            mbuf->data_len = pktsize;

            /* IPv4 Header constructor */
            pktgen_packet_ctor(pinfo, LATENCY_PKT, -2);

            rte_memcpy(rte_pktmbuf_mtod(mbuf, uint8_t *), (uint8_t *)pkt->hdr, pktsize);

            to_send = 1;
            do {
                sent = rte_eth_tx_burst(pinfo->pid, qid, &mbuf, to_send);
                to_send -= sent;
            } while (to_send > 0);

            lat->num_latency_tx_pkts++;
        } else
            printf("*** No more latency buffers\n");
    }
}

#ifdef TX_DEBUG
static __inline__ void
pktgen_validate_pkt(rte_mbuf_t *mbuf)
{
    int dump         = 0;
    const char *msg  = NULL;
    uint16_t pkt_len = rte_pktmbuf_pkt_len(mbuf);

    if (pkt_len > RTE_ETHER_MAX_LEN) {
        dump = 1;
        msg  = "Packet too long";
    } else if (pkt_len < (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN)) {
        dump = 1;
        msg  = "Packet too short";
    }

    if (dump) {
        printf("*** %s (len %u) %s ***\n", msg, pkt_len, mbuf->pool->name);
        pg_pktmbuf_dump(stdout, mbuf, 64);
    }
    if (rte_pktmbuf_pkt_len(mbuf) > (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN))
        printf("%s: pkt_len %u Mempool '%s'\n", __func__, rte_pktmbuf_pkt_len(mbuf),
               mbuf->pool->name);
}
#endif

void
tx_send_packets(port_info_t *pinfo, uint16_t qid, struct rte_mbuf **pkts, uint16_t nb_pkts)
{
    if (nb_pkts) {
        uint16_t sent, to_send = nb_pkts;

        pinfo->queue_stats.q_opackets[qid] += nb_pkts;
        for (int i = 0; i < nb_pkts; i++) {
            pinfo->queue_stats.q_obytes[qid] += rte_pktmbuf_pkt_len(pkts[i]);
#ifdef TX_DEBUG
            pktgen_write_mbuf_to_pcap_file(pinfo->pcap_file, pkts[i]);
            pktgen_validate_pkt(pkts[i]);
#endif
        }

        do {
            sent = rte_eth_tx_burst(pinfo->pid, qid, pkts, to_send);
            to_send -= sent;
            pkts += sent;
        } while (to_send > 0);

        if (qid == 0 && pktgen_tst_port_flags(pinfo, SEND_LATENCY_PKTS))
            pktgen_tstamp_inject(pinfo, qid);
    }
}

static __inline__ void
pktgen_tstamp_check(port_info_t *pinfo, struct rte_mbuf **pkts, uint16_t nb_pkts)
{
    int lid = rte_lcore_id();
    int qid = l2p_get_rxqid(lid);
    int i;
    uint64_t cycles, jitter;
    latency_t *lat = &pinfo->latency;

    for (i = 0; i < nb_pkts; i++) {

        if (pktgen_tst_port_flags(pinfo, SEND_LATENCY_PKTS)) {
            tstamp_t *tstamp = pktgen_tstamp_pointer(pinfo, rte_pktmbuf_mtod(pkts[i], char *));

            if (tstamp->magic != TSTAMP_MAGIC)
                continue;

            cycles        = (pktgen_get_time() - tstamp->timestamp);
            tstamp->magic = 0UL; /* clear timestamp magic cookie */

            if (tstamp->index != lat->expect_index) {
                lat->expect_index = tstamp->index + 1;
                lat->num_skipped++;
                continue; /* Skip this latency packet */
            }
            lat->expect_index++;

            lat->num_latency_pkts++;

            if (pktgen_tst_port_flags(pinfo, SEND_LATENCY_PKTS)) {
                lat->running_cycles += cycles;

                if (lat->min_cycles == 0 || cycles < lat->min_cycles)
                    lat->min_cycles = cycles;
                if (lat->max_cycles == 0 || cycles > lat->max_cycles)
                    lat->max_cycles = cycles;

                jitter = (cycles > lat->prev_cycles) ? cycles - lat->prev_cycles
                                                     : lat->prev_cycles - cycles;
                if (jitter > lat->jitter_threshold_cycles)
                    lat->jitter_count++;

                lat->prev_cycles = cycles;
            }
            if (pktgen_tst_port_flags(pinfo, SAMPLING_LATENCIES)) {
                /* Record latency if it's time for sampling (seperately per lcore) */
                latsamp_stats_t *stats = &pinfo->latsamp_stats[qid];
                uint64_t now           = pktgen_get_time();

                stats->pkt_counter++;
                if (stats->next == 0 || now >= stats->next) {
                    if (stats->idx < stats->num_samples) {
                        stats->data[stats->idx] = (cycles * Billion) / rte_get_tsc_hz();
                        stats->idx++;
                    }

                    /* Calculate next sampling point */
                    if (pinfo->latsamp_type == LATSAMPLER_POISSON) {
                        double next_possion_time_ns = next_poisson_time(pinfo->latsamp_rate);

                        stats->next = now + next_possion_time_ns * (double)rte_get_tsc_hz();
                    } else        // LATSAMPLER_SIMPLE or LATSAMPLER_UNSPEC
                        stats->next = now + rte_get_tsc_hz() / pinfo->latsamp_rate;
                }
            }
        }
    }
}

/**
 *
 * pktgen_tx_flush - Flush Tx buffers from ring.
 *
 * DESCRIPTION
 * Flush TX buffers from ring.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static __inline__ void
pktgen_tx_flush(port_info_t *pinfo, uint16_t qid)
{
    rte_eth_tx_done_cleanup(pinfo->pid, qid, 0);
}

/**
 *
 * pktgen_exit_cleanup - Clean up the data and other items
 *
 * DESCRIPTION
 * Clean up the data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static __inline__ void
pktgen_exit_cleanup(uint8_t lid __rte_unused)
{
}

/**
 *
 * pktgen_packet_ctor - Construct a complete packet with all headers and data.
 *
 * DESCRIPTION
 * Construct a packet type based on the arguments passed with all headers.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_packet_ctor(port_info_t *pinfo, int32_t seq_idx, int32_t type)
{
    pkt_seq_t *pkt            = &pinfo->seq_pkt[seq_idx];
    uint16_t sport_entropy    = 0;
    struct rte_ether_hdr *eth = (struct rte_ether_hdr *)&pkt->hdr->eth;
    char *l3_hdr              = (char *)&eth[1]; /* Pointer to l3 hdr location for GRE header */
    uint16_t pktsz            = (pktgen.flags & JUMBO_PKTS_FLAG) ? RTE_ETHER_MAX_JUMBO_FRAME_LEN
                                                                 : RTE_ETHER_MAX_LEN;

    /* Fill in the pattern for data space. */
    pktgen_fill_pattern((uint8_t *)pkt->hdr, pktsz, pinfo->fill_pattern_type, pinfo->user_pattern);

    if (seq_idx == LATENCY_PKT) {
        latency_t *lat = &pinfo->latency;
        tstamp_t *tstamp;

        tstamp = pktgen_tstamp_pointer(pinfo, (char *)pkt->hdr);

        tstamp->magic     = TSTAMP_MAGIC;
        tstamp->timestamp = pktgen_get_time();
        tstamp->index     = lat->next_index++;

        if (lat->latency_entropy)
            sport_entropy = (uint16_t)(pkt->sport + (tstamp->index % lat->latency_entropy));
    }
    /* Add GRE header and adjust rte_ether_hdr pointer if requested */
    if (pktgen_tst_port_flags(pinfo, SEND_GRE_IPv4_HEADER))
        l3_hdr = pktgen_gre_hdr_ctor(pinfo, pkt, (greIp_t *)l3_hdr);
    else if (pktgen_tst_port_flags(pinfo, SEND_GRE_ETHER_HEADER))
        l3_hdr = pktgen_gre_ether_hdr_ctor(pinfo, pkt, (greEther_t *)l3_hdr);
    else
        l3_hdr = pktgen_ether_hdr_ctor(pinfo, pkt);

    if (likely(pkt->ethType == RTE_ETHER_TYPE_IPV4)) {
        if (likely(pkt->ipProto == PG_IPPROTO_TCP)) {
            if (pkt->dport != PG_IPPROTO_L4_GTPU_PORT) {
                /* Construct the TCP header */
                pktgen_tcp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV4);

                /* IPv4 Header constructor */
                pktgen_ipv4_ctor(pkt, l3_hdr);
            } else {
                /* Construct the GTP-U header */
                pktgen_gtpu_hdr_ctor(pkt, l3_hdr, pkt->ipProto, GTPu_VERSION | GTPu_PT_FLAG, 0, 0,
                                     0);

                /* Construct the TCP header */
                pktgen_tcp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV4);
                if (sport_entropy != 0) {
                    struct rte_ipv4_hdr *ipv4 = (struct rte_ipv4_hdr *)l3_hdr;
                    struct rte_tcp_hdr *tcp   = (struct rte_tcp_hdr *)&ipv4[1];

                    tcp->src_port = htons(sport_entropy & 0xFFFF);
                }

                /* IPv4 Header constructor */
                pktgen_ipv4_ctor(pkt, l3_hdr);
            }
        } else if (pkt->ipProto == PG_IPPROTO_UDP) {
            if (pktgen_tst_port_flags(pinfo, SEND_VXLAN_PACKETS)) {
                /* Construct the UDP header */
                pkt->dport = VXLAN_PORT_ID;
                pktgen_udp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV4);

                /* IPv4 Header constructor */
                pktgen_ipv4_ctor(pkt, l3_hdr);
            } else if (pkt->dport != PG_IPPROTO_L4_GTPU_PORT) {
                /* Construct the UDP header */
                pktgen_udp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV4);

                /* IPv4 Header constructor */
                pktgen_ipv4_ctor(pkt, l3_hdr);
            } else {
                /* Construct the GTP-U header */
                pktgen_gtpu_hdr_ctor(pkt, l3_hdr, pkt->ipProto, GTPu_VERSION | GTPu_PT_FLAG, 0, 0,
                                     0);

                /* Construct the UDP header */
                pktgen_udp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV4);
                if (sport_entropy != 0) {
                    struct rte_ipv4_hdr *ipv4 = (struct rte_ipv4_hdr *)l3_hdr;
                    struct rte_udp_hdr *udp   = (struct rte_udp_hdr *)&ipv4[1];

                    udp->src_port = htons(sport_entropy & 0xFFFF);
                }

                /* IPv4 Header constructor */
                pktgen_ipv4_ctor(pkt, l3_hdr);
            }
        } else if (pkt->ipProto == PG_IPPROTO_ICMP) {
            struct rte_ipv4_hdr *ipv4;
            struct rte_udp_hdr *udp;
            struct rte_icmp_hdr *icmp;
            uint16_t tlen;

            /* Start from Ethernet header */
            ipv4 = (struct rte_ipv4_hdr *)l3_hdr;
            udp  = (struct rte_udp_hdr *)&ipv4[1];

            /* Create the ICMP header */
            ipv4->src_addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
            ipv4->dst_addr = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);

            tlen = pkt->pkt_size - (pkt->ether_hdr_size + sizeof(struct rte_ipv4_hdr));
            ipv4->total_length  = htons(tlen);
            ipv4->next_proto_id = pkt->ipProto;

            icmp            = (struct rte_icmp_hdr *)&udp[1];
            icmp->icmp_code = 0;
            if ((type == -1) || (type == ICMP4_TIMESTAMP)) {
                union icmp_data *data = (union icmp_data *)&udp[1];

                icmp->icmp_type           = ICMP4_TIMESTAMP;
                data->timestamp.ident     = 0x1234;
                data->timestamp.seq       = 0x5678;
                data->timestamp.originate = 0x80004321;
                data->timestamp.receive   = 0;
                data->timestamp.transmit  = 0;
            } else if (type == ICMP4_ECHO) {
                union icmp_data *data = (union icmp_data *)&udp[1];

                icmp->icmp_type  = ICMP4_ECHO;
                data->echo.ident = 0x1234;
                data->echo.seq   = 0x5678;
                data->echo.data  = 0;
            }
            icmp->icmp_cksum = 0;
            /* ICMP4_TIMESTAMP_SIZE */
            tlen             = pkt->pkt_size - (pkt->ether_hdr_size + sizeof(struct rte_ipv4_hdr));
            icmp->icmp_cksum = rte_raw_cksum(icmp, tlen);
            if (icmp->icmp_cksum == 0)
                icmp->icmp_cksum = 0xFFFF;

            /* IPv4 Header constructor */
            pktgen_ipv4_ctor(pkt, l3_hdr);
        }
    } else if (pkt->ethType == RTE_ETHER_TYPE_IPV6) {
        if (pkt->ipProto == PG_IPPROTO_TCP) {
            /* Construct the TCP header */
            pktgen_tcp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV6);
            if (sport_entropy != 0) {
                struct rte_ipv6_hdr *ipv6 = (struct rte_ipv6_hdr *)l3_hdr;
                struct rte_tcp_hdr *tcp   = (struct rte_tcp_hdr *)&ipv6[1];

                tcp->src_port = htons(sport_entropy & 0xFFFF);
            }

            /* IPv6 Header constructor */
            pktgen_ipv6_ctor(pkt, l3_hdr);
        } else if (pkt->ipProto == PG_IPPROTO_UDP) {
            /* Construct the UDP header */
            pktgen_udp_hdr_ctor(pkt, l3_hdr, RTE_ETHER_TYPE_IPV6);
            if (sport_entropy != 0) {
                struct rte_ipv6_hdr *ipv6 = (struct rte_ipv6_hdr *)l3_hdr;
                struct rte_udp_hdr *udp   = (struct rte_udp_hdr *)&ipv6[1];

                udp->src_port = htons(sport_entropy & 0xFFFF);
            }

            /* IPv6 Header constructor */
            pktgen_ipv6_ctor(pkt, l3_hdr);
        }
    } else if (pkt->ethType == RTE_ETHER_TYPE_ARP) {
        /* Start from Ethernet header */
        struct rte_arp_hdr *arp = (struct rte_arp_hdr *)l3_hdr;

        arp->arp_hardware = htons(1);
        arp->arp_protocol = htons(RTE_ETHER_TYPE_IPV4);
        arp->arp_hlen     = RTE_ETHER_ADDR_LEN;
        arp->arp_plen     = 4;

        /* make request/reply operation selectable by user */
        arp->arp_opcode = htons(2);

        rte_ether_addr_copy(&pkt->eth_src_addr, &arp->arp_data.arp_sha);
        *((uint32_t *)&arp->arp_data.arp_sha) = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);

        rte_ether_addr_copy(&pkt->eth_dst_addr, &arp->arp_data.arp_tha);
        *((uint32_t *)((void *)&arp->arp_data + offsetof(struct rte_arp_ipv4, arp_tip))) =
            htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
    } else
        pktgen_log_error("Unknown EtherType 0x%04x", pkt->ethType);
}

/**
 *
 * pktgen_packet_type - Examine a packet and return the type of packet
 *
 * DESCRIPTION
 * Examine a packet and return the type of packet.
 * the packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static __inline__ pktType_e
pktgen_packet_type(struct rte_mbuf *m)
{
    pktType_e ret;
    struct rte_ether_hdr *eth;

    eth = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);

    ret = ntohs(eth->ether_type);

    return ret;
}

/**
 *
 * pktgen_packet_classify - Examine a packet and classify it for statistics
 *
 * DESCRIPTION
 * Examine a packet and determine its type along with counting statistics around
 * the packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_packet_classify(struct rte_mbuf *m, int pid, int qid)
{
    port_info_t *pinfo     = l2p_get_port_pinfo(pid);
    pkt_stats_t *pkt_stats = &pinfo->pkt_stats;
    pkt_sizes_t *pkt_sizes = &pinfo->pkt_sizes;
    uint16_t plen;
    pktType_e pType;

    pType = pktgen_packet_type(m);

    /* Count the type of packets found. */
    switch ((int)pType) {
    case RTE_ETHER_TYPE_ARP:
        pkt_stats->arp_pkts++;
        break;
    case RTE_ETHER_TYPE_IPV4:
        pkt_stats->ip_pkts++;
        break;
    case RTE_ETHER_TYPE_IPV6:
        pkt_stats->ipv6_pkts++;
        break;
    case RTE_ETHER_TYPE_VLAN:
        pkt_stats->vlan_pkts++;
        break;
    default:
        break;
    }

    if (unlikely(pktgen_tst_port_flags(pinfo, PROCESS_INPUT_PKTS))) {
        switch ((int)pType) {
        case RTE_ETHER_TYPE_ARP:
            pktgen_process_arp(m, pid, qid, 0);
            break;
        case RTE_ETHER_TYPE_IPV4:
            pktgen_process_ping4(m, pid, qid, 0);
            break;
        case RTE_ETHER_TYPE_IPV6:
            pktgen_process_ping6(m, pid, qid, 0);
            break;
        case RTE_ETHER_TYPE_VLAN:
            pktgen_process_vlan(m, pid, qid);
            break;
        case UNKNOWN_PACKET: /* FALL THRU */
        default:
            break;
        }
    }

    plen = rte_pktmbuf_pkt_len(m) + RTE_ETHER_CRC_LEN;

    /* Count the size of each packet. */
    if (plen < RTE_ETHER_MIN_LEN)
        pkt_sizes->runt++;
    else if (plen > RTE_ETHER_MAX_LEN)
        pkt_sizes->jumbo++;
    else if (plen == RTE_ETHER_MIN_LEN)
        pkt_sizes->_64++;
    else if ((plen >= (RTE_ETHER_MIN_LEN + 1)) && (plen <= 127))
        pkt_sizes->_65_127++;
    else if ((plen >= 128) && (plen <= 255))
        pkt_sizes->_128_255++;
    else if ((plen >= 256) && (plen <= 511))
        pkt_sizes->_256_511++;
    else if ((plen >= 512) && (plen <= 1023))
        pkt_sizes->_512_1023++;
    else if ((plen >= 1024) && (plen <= RTE_ETHER_MAX_LEN))
        pkt_sizes->_1024_1518++;
    else {
        pktgen_log_info("Unknown packet size: %u", plen);
        pinfo->pkt_sizes.unknown++;
    }

    uint8_t *p = rte_pktmbuf_mtod(m, uint8_t *);

    /* Process multicast and broadcast packets. */
    if (unlikely(p[0] & 1)) {
        if ((p[0] == 0xff) && (p[1] == 0xff))
            pkt_sizes->broadcast++;
        else
            pkt_sizes->multicast++;
    }
}

/**
 *
 * pktgen_packet_classify_buld - Classify a set of packets in one call.
 *
 * DESCRIPTION
 * Classify a list of packets and to improve classify performance.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
#define PREFETCH_OFFSET 3
static __inline__ void
pktgen_packet_classify_bulk(struct rte_mbuf **pkts, int nb_rx, int pid, int qid)
{
    int j, i;

    /* Prefetch first packets */
    for (j = 0; j < PREFETCH_OFFSET && j < nb_rx; j++)
        rte_prefetch0(rte_pktmbuf_mtod(pkts[j], void *));

    /* Prefetch and handle already prefetched packets */
    for (i = 0; i < (nb_rx - PREFETCH_OFFSET); i++) {
        rte_prefetch0(rte_pktmbuf_mtod(pkts[j], void *));
        j++;

        pktgen_packet_classify(pkts[i], pid, qid);
    }

    /* Handle remaining prefetched packets */
    for (; i < nb_rx; i++)
        pktgen_packet_classify(pkts[i], pid, qid);
}

/**
 *
 * pktgen_send_special - Send a special packet to the given port.
 *
 * DESCRIPTION
 * Create a special packet in the buffer provided.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_send_special(port_info_t *pinfo)
{
    if (!pktgen_tst_port_flags(pinfo, SEND_ARP_PING_REQUESTS))
        return;

    /* Send packets attached to the sequence packets. */
    for (uint32_t s = 0; s < pinfo->seqCnt; s++) {
        if (unlikely(pktgen_tst_port_flags(pinfo, SEND_GRATUITOUS_ARP)))
            pktgen_send_arp(pinfo->pid, GRATUITOUS_ARP, s);
        else if (unlikely(pktgen_tst_port_flags(pinfo, SEND_ARP_REQUEST)))
            pktgen_send_arp(pinfo->pid, 0, s);

        if (unlikely(pktgen_tst_port_flags(pinfo, SEND_PING4_REQUEST)))
            pktgen_send_ping4(pinfo->pid, s);
#ifdef INCLUDE_PING6
        if (unlikely(pktgen_tst_port_flags(pinfo, SEND_PING6_REQUEST)))
            pktgen_send_ping6(pinfo->pid, s);
#endif
    }

    /* Send the requests from the Single packet setup. */
    if (unlikely(pktgen_tst_port_flags(pinfo, SEND_GRATUITOUS_ARP)))
        pktgen_send_arp(pinfo->pid, GRATUITOUS_ARP, SINGLE_PKT);
    else if (unlikely(pktgen_tst_port_flags(pinfo, SEND_ARP_REQUEST)))
        pktgen_send_arp(pinfo->pid, 0, SINGLE_PKT);

    if (unlikely(pktgen_tst_port_flags(pinfo, SEND_PING4_REQUEST)))
        pktgen_send_ping4(pinfo->pid, SINGLE_PKT);
#ifdef INCLUDE_PING6
    if (unlikely(pktgen_tst_port_flags(pinfo, SEND_PING6_REQUEST)))
        pktgen_send_ping6(pinfo->pid, SINGLE_PKT);
#endif

    pktgen_clr_port_flags(pinfo, SEND_ARP_PING_REQUESTS);
}

struct pkt_setup_s {
    int32_t seq_idx;
    port_info_t *pinfo;
};

static __inline__ void
mempool_setup_cb(struct rte_mempool *mp __rte_unused, void *opaque, void *obj,
                 unsigned obj_idx __rte_unused)
{
    struct rte_mbuf *m    = (struct rte_mbuf *)obj;
    struct pkt_setup_s *s = (struct pkt_setup_s *)opaque;
    port_info_t *pinfo    = s->pinfo;
    int32_t idx, seq_idx = s->seq_idx;
    pkt_seq_t *pkt;

    idx = seq_idx;
    if (pktgen_tst_port_flags(pinfo, SEND_SEQ_PKTS)) {
        idx = pinfo->seqIdx;

        /* move to the next packet in the sequence. */
        if (unlikely(++pinfo->seqIdx >= pinfo->seqCnt))
            pinfo->seqIdx = 0;
    }
    pkt = &pinfo->seq_pkt[idx];

    if (idx == RANGE_PKT)
        pktgen_range_ctor(&pinfo->range, pkt);

    pktgen_packet_ctor(pinfo, idx, -1);

    rte_memcpy(rte_pktmbuf_mtod(m, uint8_t *), (uint8_t *)pkt->hdr, pkt->pkt_size);

    m->pkt_len  = pkt->pkt_size;
    m->data_len = pkt->pkt_size;

    switch (pkt->ethType) {
    case RTE_ETHER_TYPE_IPV4:
        if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM)
            pkt->ol_flags = RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_IPV4;
        break;

    case RTE_ETHER_TYPE_IPV6:
        pkt->ol_flags = RTE_MBUF_F_TX_IP_CKSUM | RTE_MBUF_F_TX_IPV6;
        break;

    case RTE_ETHER_TYPE_VLAN:
        if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_VLAN_INSERT) {
            /* TODO */
        }
        break;
    default:
        break;
    }

    switch (pkt->ipProto) {
    case PG_IPPROTO_UDP:
        if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_UDP_CKSUM)
            pkt->ol_flags |= RTE_MBUF_F_TX_UDP_CKSUM;
        break;
    case PG_IPPROTO_TCP:
        if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_TCP_CKSUM)
            pkt->ol_flags |= RTE_MBUF_F_TX_TCP_CKSUM;
        break;
    default:
        break;
    }
    m->ol_flags = pkt->ol_flags;
}

/**
 *
 * pktgen_setup_packets - Setup the default packets to be sent.
 *
 * DESCRIPTION
 * Construct the default set of packets for a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_setup_packets(uint16_t pid)
{
    struct rte_mempool *tx_mp = l2p_get_tx_mp(pid);
    l2p_port_t *port          = l2p_get_port(pid);
    port_info_t *pinfo        = l2p_get_port_pinfo(pid);

    if (unlikely(tx_mp == NULL))
        rte_exit(EXIT_FAILURE, "Invalid mempool for port %d\n", pid);

    if (port == NULL)
        rte_exit(EXIT_FAILURE, "Invalid l2p port for %d\n", pid);

    /* Make sure we are not updating the mempool from two different lcores */
    pthread_spin_lock(&port->lock);

    if (pktgen_tst_port_flags(pinfo, SETUP_TRANSMIT_PKTS)) {
        if (!pktgen_tst_port_flags(pinfo, SEND_PCAP_PKTS)) {
            struct pkt_setup_s s;
            int32_t idx = SINGLE_PKT;

            if (pktgen_tst_port_flags(pinfo, SEND_RANGE_PKTS)) {
                idx = RANGE_PKT;
            } else if (pktgen_tst_port_flags(pinfo, SEND_SEQ_PKTS))
                idx = FIRST_SEQ_PKT;
            else if (pktgen_tst_port_flags(pinfo, (SEND_SINGLE_PKTS | SEND_RANDOM_PKTS)))
                idx = SINGLE_PKT;

            s.pinfo   = pinfo;
            s.seq_idx = idx;
            rte_mempool_obj_iter(tx_mp, mempool_setup_cb, &s);
        }
        pktgen_clr_port_flags(pinfo, SETUP_TRANSMIT_PKTS);
    }
    pthread_spin_unlock(&port->lock);
}

/**
 *
 * pktgen_send_pkts - Send a set of packet buffers to a given port.
 *
 * DESCRIPTION
 * Transmit a set of packets mbufs to a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static __inline__ void
pktgen_send_pkts(port_info_t *pinfo, uint16_t qid, struct rte_mempool *mp)
{
    uint64_t txCnt;
    struct rte_mbuf *pkts[pinfo->tx_burst];

    if (!pktgen_tst_port_flags(pinfo, SEND_FOREVER)) {
        txCnt = pkt_atomic64_tx_count(&pinfo->current_tx_count, pinfo->tx_burst);
        if (txCnt == 0) {
            pktgen_clr_port_flags(pinfo, SENDING_PACKETS);
            return;
        }
        if (txCnt > pinfo->tx_burst)
            txCnt = pinfo->tx_burst;
    } else
        txCnt = pinfo->tx_burst;

    if (rte_mempool_get_bulk(mp, (void **)pkts, txCnt) == 0)
        tx_send_packets(pinfo, qid, pkts, txCnt);
}

/**
 *
 * pktgen_main_transmit - Determine the next packet format to transmit.
 *
 * DESCRIPTION
 * Determine the next packet format to transmit for a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_main_transmit(port_info_t *pinfo, uint16_t qid)
{
    struct rte_mempool *mp = NULL;

    /* Transmit ARP/Ping packets if needed */
    pktgen_send_special(pinfo);

    /* When not transmitting on this port then continue. */
    if (pktgen_tst_port_flags(pinfo, SENDING_PACKETS)) {
        mp = l2p_get_tx_mp(pinfo->pid);

        pktgen_setup_packets(pinfo->pid);

        if (pktgen_tst_port_flags(pinfo, SEND_PCAP_PKTS))
            mp = l2p_get_pcap_mp(pinfo->pid);

        pinfo->qcnt[qid]++; /* Count the number of times queue is sending */

        if (pktgen_tst_port_flags(pinfo, SEND_PCAP_PKTS))
            mp = l2p_get_pcap_mp(pinfo->pid);

        pktgen_send_pkts(pinfo, qid, mp);
    }
}

/**
 *
 * pktgen_main_receive - Main receive routine for packets of a port.
 *
 * DESCRIPTION
 * Handle the main receive set of packets on a given port plus handle all of the
 * input processing if required.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static __inline__ void
pktgen_main_receive(port_info_t *pinfo, uint16_t qid, struct rte_mbuf **pkts_burst,
                    uint16_t nb_pkts)
{
    uint16_t pid, nb_rx;

    if (unlikely(pktgen_tst_port_flags(pinfo, STOP_RECEIVING_PACKETS)))
        return;

    pid = pinfo->pid;

    /* Read packets from RX queues and free the mbufs */
    if (likely((nb_rx = rte_eth_rx_burst(pid, qid, pkts_burst, nb_pkts)) > 0)) {
        struct rte_eth_stats *qstats = &pinfo->queue_stats;

        qstats->q_ipackets[qid] += nb_rx;
        for (int i = 0; i < nb_rx; i++)
            qstats->q_ibytes[qid] += rte_pktmbuf_pkt_len(pkts_burst[i]);

        pktgen_tstamp_check(pinfo, pkts_burst, nb_rx);

        /* classify the packets for the counters */
        pktgen_packet_classify_bulk(pkts_burst, nb_rx, pid, qid);

        if (unlikely(pinfo->dump_count > 0))
            pktgen_packet_dump_bulk(pkts_burst, nb_rx, pid);

        if (unlikely(pktgen_tst_port_flags(pinfo, CAPTURE_PKTS))) {
            capture_t *capture = &pktgen.capture[rte_socket_id()];

            if (unlikely(capture->port == pid))
                pktgen_packet_capture_bulk(pkts_burst, nb_rx, capture);
        }

        rte_pktmbuf_free_bulk(pkts_burst, nb_rx);
    }
}

/**
 *
 * pktgen_main_rxtx_loop - Single thread loop for tx/rx packets
 *
 * DESCRIPTION
 * Handle sending and receiving packets from a given set of ports. This is the
 * main loop or thread started on a single core.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_main_rxtx_loop(void)
{
    port_info_t *pinfo;
    struct rte_mbuf *pkts_burst[MAX_PKT_RX_BURST];
    uint64_t curr_tsc, tx_next_cycle, tx_bond_cycle;
    uint16_t rx_qid, tx_qid, lid = rte_lcore_id();

    if (lid == rte_get_main_lcore()) {
        printf("Using %d initial lcore for Rx/Tx\n", lid);
        rte_exit(0, "using initial lcore for port");
    }

    pinfo = l2p_get_pinfo_by_lcore(lid);

    rx_qid = l2p_get_rxqid(lid);
    tx_qid = l2p_get_txqid(lid);

    printf("RX/TX lid %3d, pid %2d, qids %2d/%2d Mempool %-16s @ %p\n", lid, pinfo->pid, rx_qid,
           tx_qid, l2p_get_tx_mp(pinfo->pid)->name, l2p_get_tx_mp(pinfo->pid));

    curr_tsc      = pktgen_get_time();
    tx_next_cycle = curr_tsc;
    tx_bond_cycle = curr_tsc + (pktgen_get_timer_hz() / 10);

    while (pktgen.force_quit == 0) {
        /* Read Packets */
        pktgen_main_receive(pinfo, rx_qid, pkts_burst, pinfo->tx_burst);

        curr_tsc = pktgen_get_time();

        /* Determine when is the next time to send packets */
        if (curr_tsc >= tx_next_cycle) {
            tx_next_cycle = curr_tsc + pinfo->tx_cycles;

            if (unlikely(pinfo->tx_cycles == 0))
                continue;
            pktgen_main_transmit(pinfo, tx_qid);
        }
        if (curr_tsc >= tx_bond_cycle) {
            tx_bond_cycle = curr_tsc + (pktgen_get_timer_hz() / 10);
            if (pktgen_tst_port_flags(pinfo, BONDING_TX_PACKETS))
                rte_eth_tx_burst(pinfo->pid, tx_qid, NULL, 0);
        }
    }

    pktgen_log_debug("Exit %d", lid);

    pktgen_exit_cleanup(lid);
}

/**
 *
 * pktgen_main_tx_loop - Main transmit loop for a core, no receive packet handling
 *
 * DESCRIPTION
 * When Tx and Rx are split across two cores this routing handles the tx packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_main_tx_loop(void)
{
    port_info_t *pinfo;
    uint64_t curr_tsc, tx_next_cycle, tx_bond_cycle;
    uint16_t tx_qid, lid = rte_lcore_id();

    if (lid == rte_get_main_lcore()) {
        printf("Using %d initial lcore for Rx/Tx\n", lid);
        rte_exit(0, "Invalid initial lcore assigned to a port");
    }

    pinfo  = l2p_get_pinfo_by_lcore(lid);
    tx_qid = l2p_get_txqid(lid);

    printf("TX lid %3d, pid %2d, qid %2d, Mempool %-16s @ %p\n", lid, pinfo->pid, tx_qid,
           l2p_get_tx_mp(pinfo->pid)->name, l2p_get_tx_mp(pinfo->pid));

    curr_tsc      = pktgen_get_time();
    tx_next_cycle = curr_tsc;
    tx_bond_cycle = curr_tsc + pktgen_get_timer_hz() / 10;

    while (pktgen.force_quit == 0) {
        curr_tsc = pktgen_get_time();

        /* Determine when is the next time to send packets */
        if (curr_tsc >= tx_next_cycle) {
            tx_next_cycle = curr_tsc + pinfo->tx_cycles;

            if (unlikely(pinfo->tx_cycles == 0))
                continue;
            pktgen_main_transmit(pinfo, tx_qid);
        }
        if (curr_tsc >= tx_bond_cycle) {
            tx_bond_cycle = curr_tsc + pktgen_get_timer_hz() / 10;
            if (pktgen_tst_port_flags(pinfo, BONDING_TX_PACKETS))
                rte_eth_tx_burst(pinfo->pid, tx_qid, NULL, 0);
        }
    }

    pktgen_log_debug("Exit %d", lid);

    pktgen_exit_cleanup(lid);
}

/**
 *
 * pktgen_main_rx_loop - Handle only the rx packets for a set of ports.
 *
 * DESCRIPTION
 * When Tx and Rx processing is split between two ports this routine handles
 * only the receive packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_main_rx_loop(void)
{
    port_info_t *pinfo;
    struct rte_mbuf *pkts_burst[MAX_PKT_RX_BURST];
    uint16_t lid = rte_lcore_id(), rx_qid = l2p_get_rxqid(lid);

    if (lid == rte_get_main_lcore()) {
        printf("Using %d initial lcore for Rx/Tx\n", lid);
        rte_exit(0, "using initial lcore for ports");
    }

    pinfo  = l2p_get_pinfo_by_lcore(lid);
    rx_qid = l2p_get_rxqid(lid);

    printf("RX lid %3d, pid %2d, qid %2d, Mempool %-16s @ %p\n", lid, pinfo->pid, rx_qid,
           l2p_get_rx_mp(pinfo->pid)->name, l2p_get_rx_mp(pinfo->pid));

    while (pktgen.force_quit == 0)
        pktgen_main_receive(pinfo, rx_qid, pkts_burst, pinfo->rx_burst);

    pktgen_log_debug("Exit %d", lid);

    pktgen_exit_cleanup(lid);
}

/**
 *
 * pktgen_launch_one_lcore - Launch a single logical core thread.
 *
 * DESCRIPTION
 * Help launching a single thread on one logical core.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
int
pktgen_launch_one_lcore(void *arg __rte_unused)
{
    uint16_t pid, lid = rte_lcore_id();

    if ((pid = l2p_get_pid_by_lcore(lid)) >= RTE_MAX_ETHPORTS) {
        pktgen_log_info("*** Logical core %3d has no work, skipping launch", lid);
        return 0;
    }

    switch (l2p_get_type(lid)) {
    case LCORE_MODE_RX:
        pktgen_main_rx_loop();
        break;
    case LCORE_MODE_TX:
        pktgen_main_tx_loop();
        break;
    case LCORE_MODE_BOTH:
        pktgen_main_rxtx_loop();
        break;
    default:
        rte_exit(EXIT_FAILURE, "Invalid logical core mode %d\n", l2p_get_type(lid));
    }
    return 0;
}

static void
_page_display(void)
{
    static unsigned int counter = 0;

    pktgen_display_set_color("top.spinner");
    scrn_printf(1, 1, "%c", "-\\|/"[(counter++ & 3)]);
    pktgen_display_set_color(NULL);

    if (pktgen.flags & SYSTEM_PAGE_FLAG)
        pktgen_page_system();
    else if (pktgen.flags & PCAP_PAGE_FLAG)
        pktgen_page_pcap(pktgen.curr_port);
    else if (pktgen.flags & RANGE_PAGE_FLAG)
        pktgen_page_range();
    else if (pktgen.flags & CPU_PAGE_FLAG)
        pktgen_page_cpu();
    else if (pktgen.flags & SEQUENCE_PAGE_FLAG)
        pktgen_page_seq(pktgen.curr_port);
    else if (pktgen.flags & RND_BITFIELD_PAGE_FLAG) {
        port_info_t *pinfo = l2p_get_port_pinfo(pktgen.curr_port);
        pktgen_page_random_bitfields(pktgen.flags & PRINT_LABELS_FLAG, pktgen.curr_port,
                                     pinfo->rnd_bitfields);
    } else if (pktgen.flags & LOG_PAGE_FLAG)
        pktgen_page_log(pktgen.flags & PRINT_LABELS_FLAG);
    else if (pktgen.flags & LATENCY_PAGE_FLAG)
        pktgen_page_latency();
    else if (pktgen.flags & STATS_PAGE_FLAG)
        pktgen_page_queue_stats(pktgen.curr_port);
    else if (pktgen.flags & XSTATS_PAGE_FLAG)
        pktgen_page_xstats(pktgen.curr_port);
    else
        pktgen_page_stats();
}

/**
 *
 * pktgen_page_display - Display the correct page based on timer callback.
 *
 * DESCRIPTION
 * When timer is active update or display the correct page of data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_page_display(void)
{
    static unsigned int update_display = 1;

    /* Leave if the screen is paused */
    if (scrn_is_paused())
        return;

    scrn_save();

    if (pktgen.flags & UPDATE_DISPLAY_FLAG) {
        pktgen.flags &= ~UPDATE_DISPLAY_FLAG;
        update_display = 1;
    }

    update_display--;
    if (update_display == 0) {
        update_display = UPDATE_DISPLAY_TICK_INTERVAL;
        _page_display();

        if (pktgen.flags & PRINT_LABELS_FLAG)
            pktgen.flags &= ~PRINT_LABELS_FLAG;
    }

    scrn_restore();

    pktgen_print_packet_dump();
}

static void *
_timer_thread(void *arg)
{
    uint64_t process, page, prev;

    this_scrn = arg;

    pktgen.stats_timeout = pktgen.hz;
    pktgen.page_timeout  = UPDATE_DISPLAY_TICK_RATE;

    page = prev = pktgen_get_time();
    process     = page + pktgen.stats_timeout;
    page += pktgen.page_timeout;

    pktgen.timer_running = 1;

    while (pktgen.timer_running) {
        uint64_t curr;

        curr = pktgen_get_time();

        if (curr >= process) {
            process = curr + pktgen.stats_timeout;
            pktgen_process_stats();
            prev = curr;
        }

        if (curr >= page) {
            page = curr + pktgen.page_timeout;
            pktgen_page_display();
        }

        rte_pause();
    }
    return NULL;
}

/**
 *
 * pktgen_timer_setup - Set up the timer callback routines.
 *
 * DESCRIPTION
 * Setup the two timers to be used for display and calculating statistics.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_timer_setup(void)
{
    rte_cpuset_t cpuset_data;
    rte_cpuset_t *cpuset = &cpuset_data;
    pthread_t tid;

    CPU_ZERO(cpuset);

    pthread_create(&tid, NULL, _timer_thread, this_scrn);

    CPU_SET(rte_get_main_lcore(), cpuset);
    pthread_setaffinity_np(tid, sizeof(cpuset), cpuset);
}
