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

#include "pktgen.h"

#include "pktgen-ipv6.h"

/**************************************************************************//**
*
* pktgen_ipv6_ctor - IPv6 packet header constructor routine.
*
* DESCRIPTION
* Construct the IPv6 header constructor routine.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_ipv6_ctor(pkt_seq_t * pkt, ipv6Hdr_t * ip)
{
    uint32_t            addr;
    uint16_t			tlen;

    // IPv6 Header constructor
    memset(ip, 0, sizeof(ipv6Hdr_t));

    ip->ver_tc_fl       = htonl(IPv6_VERSION << 28);
    tlen           		= pkt->pktSize - (pkt->ether_hdr_size + sizeof(ipv6Hdr_t));

    ip->payload_length  = htons(tlen);
    ip->hop_limit       = 4;
    ip->next_header     = pkt->ipProto;

    addr                = htonl(pkt->ip_dst_addr);
    (void)rte_memcpy(&ip->daddr[8], &addr, sizeof(uint32_t));
    addr                = htonl(pkt->ip_src_addr);
    (void)rte_memcpy(&ip->saddr[8], &addr, sizeof(uint32_t));
}

/**************************************************************************//**
*
* pktgen_process_ping6 - Process a IPv6 ICMP echo request packet.
*
* DESCRIPTION
* Process a IPv6 ICMP echo request packet and send response if needed.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_process_ping6( __attribute__ ((unused)) struct rte_mbuf * m,
__attribute__ ((unused)) uint32_t pid, __attribute__ ((unused)) uint32_t vlan )
{
#if 0 /* Broken needs to be updated to do IPv6 packets */
    port_info_t     * info = &pktgen.info[pid];
    struct ether_hdr *eth = rte_pktmbuf_mtod(m, struct ether_hdr *);
    ipv6Hdr_t       * ip = (ipv6Hdr_t *)&eth[1];

	/* Adjust for a vlan header if present */
	if ( vlan )
		ip = (ipv6Hdr_t *)((char *)ip + sizeof(struct vlan_hdr));

    // Look for a ICMP echo requests, but only if enabled.
    if ( (rte_atomic32_read(&info->port_flags) & ICMP_ECHO_ENABLE_FLAG) &&
    		(ip->next_header == PG_IPPROTO_ICMPV6) ) {
#if !defined(RTE_ARCH_X86_64)
        icmpv4Hdr_t * icmp = (icmpv4Hdr_t *)((uint32_t)ip + sizeof(ipHdr_t));
#else
        icmpv4Hdr_t * icmp = (icmpv4Hdr_t *)((uint64_t)ip + sizeof(ipHdr_t));
#endif
        // We do not handle IP options, which will effect the IP header size.
        if ( cksum(icmp, (m->pkt.data_len - sizeof(struct ether_hdr) - sizeof(ipHdr_t)), 0) ) {
            rte_printf_status("ICMP checksum failed\n");
            goto leave:
        }

        if ( icmp->type == ICMP4_ECHO ) {
            // Toss all broadcast addresses and requests not for this port
            if ( (ip->dst == INADDR_BROADCAST) || (ip->dst != info->ip_src_addr) ) {
                char        buff[24];
                rte_printf_status("IP address %s != ",
                        inet_ntop4(buff, sizeof(buff), ip->dst, INADDR_BROADCAST));
                rte_printf_status("%s\n",
                        inet_ntop4(buff, sizeof(buff), htonl(info->ip_src_addr), INADDR_BROADCAST));
                goto leave;
            }

            info->echo_pkts++;

            icmp->type  = ICMP4_ECHO_REPLY;

            /* Recompute the ICMP checksum */
            icmp->cksum = 0;
            icmp->cksum = cksum(icmp, (m->pkt.data_len - sizeof(struct ether_hdr) - sizeof(ipHdr_t)), 0);

            // Swap the IP addresses.
            inetAddrSwap(&ip->src, &ip->dst);

            // Bump the ident value
            ip->ident   = htons(ntohs(ip->ident) + m->pkt.data_len);

            // Recompute the IP checksum
            ip->cksum   = 0;
            ip->cksum   = cksum(ip, sizeof(ipHdr_t), 0);

            // Swap the MAC addresses
            ethAddrSwap(&eth->d_addr, &eth->s_addr);

            pktgen_send_mbuf(m, pid, 0);

            pktgen_set_q_flags(info, 0, DO_TX_FLUSH);

            // No need to free mbuf as it was reused
            return;
        }
    }
leave:
#else
#endif
}
