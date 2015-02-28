/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include "pktgen-ipv4.h"

#include <arpa/inet.h>

#include "pktgen-log.h"
#include "pktgen.h"


/**************************************************************************//**
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
pktgen_ipv4_ctor(pkt_seq_t * pkt, ipHdr_t * ip)
{
	uint16_t	tlen;

    // IPv4 Header constructor
    tlen                = pkt->pktSize - pkt->ether_hdr_size;

    // Zero out the header space
    memset((char *)ip, 0, sizeof(ipHdr_t));

    ip->vl              = (IPv4_VERSION << 4) | (sizeof(ipHdr_t) /4);

    ip->tlen            = htons(tlen);
    ip->ttl             = 4;
    ip->tos             = 0;

    pktgen.ident        += 27;          // bump by a prime number
    ip->ident           = htons(pktgen.ident);
    ip->ffrag           = 0;
    ip->proto           = pkt->ipProto;
    ip->src             = htonl(pkt->ip_src_addr);
    ip->dst             = htonl(pkt->ip_dst_addr);
    ip->cksum           = cksum(ip, sizeof(ipHdr_t), 0);
}

/**************************************************************************//**
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
pktgen_send_ping4( uint32_t pid, uint8_t seq_idx )
{
    port_info_t       * info = &pktgen.info[pid];
    pkt_seq_t         * ppkt = &info->seq_pkt[PING_PKT];
    pkt_seq_t         * spkt = &info->seq_pkt[seq_idx];
    struct rte_mbuf   * m ;
    uint8_t				qid = 0;

    m   = rte_pktmbuf_alloc(info->q[qid].special_mp);
    if ( unlikely(m == NULL) ) {
        pktgen_log_warning("No packet buffers found");
        return;
    }
	*ppkt = *spkt;		// Copy the sequence setup to the ping setup.
    pktgen_packet_ctor(info, PING_PKT, ICMP4_ECHO);
	rte_memcpy((uint8_t *)m->buf_addr + m->data_off, (uint8_t *)&ppkt->hdr, ppkt->pktSize);

    m->pkt_len  = ppkt->pktSize;
    m->data_len = ppkt->pktSize;

    pktgen_send_mbuf(m, pid, qid);

    pktgen_set_q_flags(info, qid, DO_TX_FLUSH);
}

/**************************************************************************//**
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
pktgen_process_ping4( struct rte_mbuf * m, uint32_t pid, uint32_t vlan )
{
    port_info_t   * info = &pktgen.info[pid];
    pkt_seq_t     * pkt;
    struct ether_hdr *eth = rte_pktmbuf_mtod(m, struct ether_hdr *);
    ipHdr_t       * ip = (ipHdr_t *)&eth[1];
    char            buff[24];

	/* Adjust for a vlan header if present */
	if ( vlan )
		ip = (ipHdr_t *)((char *)ip + sizeof(struct vlan_hdr));

    // Look for a ICMP echo requests, but only if enabled.
    if ( (rte_atomic32_read(&info->port_flags) & ICMP_ECHO_ENABLE_FLAG) &&
    		(ip->proto == PG_IPPROTO_ICMP) ) {
#if !defined(RTE_ARCH_X86_64)
        icmpv4Hdr_t * icmp = (icmpv4Hdr_t *)((uint32_t)ip + sizeof(ipHdr_t));
#else
        icmpv4Hdr_t * icmp = (icmpv4Hdr_t *)((uint64_t)ip + sizeof(ipHdr_t));
#endif

        // We do not handle IP options, which will effect the IP header size.
        if ( unlikely(cksum(icmp, (m->data_len - sizeof(struct ether_hdr) - sizeof(ipHdr_t)), 0)) ) {
            pktgen_log_error("ICMP checksum failed");
            return;
        }

        if ( unlikely(icmp->type == ICMP4_ECHO) ) {
            if ( ntohl(ip->dst) == INADDR_BROADCAST ) {
                pktgen_log_warning("IP address %s is a Broadcast",
                        inet_ntop4(buff, sizeof(buff), ip->dst, INADDR_BROADCAST));
                return;
            }

            // Toss all broadcast addresses and requests not for this port
            pkt = pktgen_find_matching_ipsrc(info, ip->dst);

            // ARP request not for this interface.
            if ( unlikely(pkt == NULL) ) {
                pktgen_log_warning("IP address %s not found",
                        inet_ntop4(buff, sizeof(buff), ip->dst, INADDR_BROADCAST));
                return;
            }

            info->stats.echo_pkts++;

            icmp->type  = ICMP4_ECHO_REPLY;

            /* Recompute the ICMP checksum */
            icmp->cksum = 0;
            icmp->cksum = cksum(icmp, (m->data_len - sizeof(struct ether_hdr) - sizeof(ipHdr_t)), 0);

            // Swap the IP addresses.
            inetAddrSwap(&ip->src, &ip->dst);

            // Bump the ident value
            ip->ident   = htons(ntohs(ip->ident) + m->data_len);

            // Recompute the IP checksum
            ip->cksum   = 0;
            ip->cksum   = cksum(ip, sizeof(ipHdr_t), 0);

            // Swap the MAC addresses
            ethAddrSwap(&eth->d_addr, &eth->s_addr);

            pktgen_send_mbuf(m, pid, 0);

            pktgen_set_q_flags(info, 0, DO_TX_FLUSH);

            // No need to free mbuf as it was reused.
            return;
        } else if ( unlikely(icmp->type == ICMP4_ECHO_REPLY) ) {
            info->stats.echo_pkts++;
        }
    }
}

