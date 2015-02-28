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

#include "pktgen-arp.h"

#include "pktgen.h"
#include "pktgen-cmds.h"
#include "pktgen-log.h"


/**************************************************************************//**
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
pktgen_send_arp( uint32_t pid, uint32_t type, uint8_t seq_idx )
{
    port_info_t       * info = &pktgen.info[pid];
    pkt_seq_t         * pkt;
    struct rte_mbuf   * m ;
    struct ether_hdr  * eth;
    arpPkt_t          * arp;
    uint32_t            addr;
    uint8_t				qid = 0;

    pkt = &info->seq_pkt[seq_idx];
    m   = rte_pktmbuf_alloc(info->q[qid].special_mp);
    if ( unlikely(m == NULL) ) {
        pktgen_log_warning("No packet buffers found");
        return;
    }
    eth = rte_pktmbuf_mtod(m, struct ether_hdr *);
    arp = (arpPkt_t *)&eth[1];

    /* src and dest addr */
    memset(&eth->d_addr, 0xFF, 6);
    ether_addr_copy(&pkt->eth_src_addr, &eth->s_addr);
    eth->ether_type = htons(ETHER_TYPE_ARP);

    memset(arp, 0, sizeof(arpPkt_t));

    rte_memcpy( &arp->sha, &pkt->eth_src_addr, 6 );
    addr = htonl(pkt->ip_src_addr);
    inetAddrCopy(&arp->spa, &addr);

    if ( likely(type == GRATUITOUS_ARP) ) {
        rte_memcpy( &arp->tha, &pkt->eth_src_addr, 6 );
        addr = htonl(pkt->ip_src_addr);
        inetAddrCopy(&arp->tpa, &addr);
    } else {
        memset( &arp->tha, 0, 6 );
        addr = htonl(pkt->ip_dst_addr);
        inetAddrCopy(&arp->tpa, &addr);
    }

    /* Fill in the rest of the ARP packet header */
    arp->hrd    = htons(ETH_HW_TYPE);
    arp->pro    = htons(ETHER_TYPE_IPv4);
    arp->hln    = 6;
    arp->pln    = 4;
    arp->op     = htons(ARP_REQUEST);

    m->pkt_len  = 60;
    m->data_len = 60;

    pktgen_send_mbuf(m, pid, qid);

    pktgen_set_q_flags(info, qid, DO_TX_FLUSH);
}

/**************************************************************************//**
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
pktgen_process_arp( struct rte_mbuf * m, uint32_t pid, uint32_t vlan )
{
    port_info_t   * info = &pktgen.info[pid];
    pkt_seq_t     * pkt;
    struct ether_hdr *eth = rte_pktmbuf_mtod(m, struct ether_hdr *);
    arpPkt_t      * arp = (arpPkt_t *)&eth[1];

	/* Adjust for a vlan header if present */
	if ( vlan )
		arp = (arpPkt_t *)((char *)arp + sizeof(struct vlan_hdr));

    // Process all ARP requests if they are for us.
    if ( arp->op == htons(ARP_REQUEST) ) {
		if ((rte_atomic32_read(&info->port_flags) & PROCESS_GARP_PKTS) &&
 			(arp->tpa._32 == arp->spa._32) ) {		/* Must be a GARP packet */

			pkt = pktgen_find_matching_ipdst(info, arp->spa._32);

			/* Found a matching packet, replace the dst address */
			if ( pkt ) {
				rte_memcpy(&pkt->eth_dst_addr, &arp->sha, 6);
				pktgen_set_q_flags(info, wr_get_txque(pktgen.l2p, rte_lcore_id(), pid), DO_TX_CLEANUP);
				pktgen_redisplay(0);
			}
			return;
		}

		pkt = pktgen_find_matching_ipsrc(info, arp->tpa._32);

		/* ARP request not for this interface. */
		if ( likely(pkt != NULL) ) {
			/* Grab the source MAC address as the destination address for the port. */
			if ( unlikely(pktgen.flags & MAC_FROM_ARP_FLAG) ) {
				uint32_t    i;

				rte_memcpy(&pkt->eth_dst_addr, &arp->sha, 6);
				for (i = 0; i < info->seqCnt; i++)
					pktgen_packet_ctor(info, i, -1);
			}

			// Swap the two MAC addresses
			ethAddrSwap(&arp->sha, &arp->tha);

			// Swap the two IP addresses
			inetAddrSwap(&arp->tpa._32, &arp->spa._32);

			// Set the packet to ARP reply
			arp->op = htons(ARP_REPLY);

			// Swap the MAC addresses
			ethAddrSwap(&eth->d_addr, &eth->s_addr);

			// Copy in the MAC address for the reply.
			rte_memcpy(&arp->sha, &pkt->eth_src_addr, 6);
			rte_memcpy(&eth->s_addr, &pkt->eth_src_addr, 6);

			pktgen_send_mbuf(m, pid, 0);

			// Flush all of the packets in the queue.
			pktgen_set_q_flags(info, 0, DO_TX_FLUSH);

			// No need to free mbuf as it was reused
			return;
		}
	} else if ( arp->op == htons(ARP_REPLY) ) {
		pkt = pktgen_find_matching_ipsrc(info, arp->tpa._32);

		// ARP request not for this interface.
		if ( likely(pkt != NULL) ) {
			// Grab the real destination MAC address
			if ( pkt->ip_dst_addr == ntohl(arp->spa._32) )
				rte_memcpy(&pkt->eth_dst_addr, &arp->sha, 6);

			pktgen.flags |= PRINT_LABELS_FLAG;
		}
	}
}
