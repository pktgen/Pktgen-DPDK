/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *	 notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *	 notice, this list of conditions and the following disclaimer in
 *	 the documentation and/or other materials provided with the
 *	 distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *	 contributors may be used to endorse or promote products derived
 *	 from this software without specific prior written permission.
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
 *	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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

#ifndef _PKTGEN_SEQ_H_
#define _PKTGEN_SEQ_H_


#include <rte_ether.h>
#include <wr_inet.h>

#include "pktgen-constants.h"


typedef struct pkt_seq_s {
	// Packet type and information
	struct ether_addr	eth_dst_addr;			/**< Destination Ethernet address */
	struct ether_addr	eth_src_addr;			/**< Source Ethernet address */

	uint32_t			ip_src_addr;			/**< Source IPv4 address also used for IPv6 */
	uint32_t			ip_dst_addr;			/**< Destination IPv4 address */
	uint32_t			ip_mask;				/**< IPv4 Netmask value */

	uint16_t			sport;					/**< Source port value */
	uint16_t			dport;					/**< Destination port value */
	uint16_t			ethType;				/**< IPv4 or IPv6 */
	uint16_t			ipProto;				/**< TCP or UDP or ICMP */
	uint16_t			vlanid;					/**< VLAN ID value if used */
	uint16_t			ether_hdr_size;			/**< Size of Ethernet header in packet for VLAN ID */

	uint32_t			mpls_entry;				/**< MPLS entry if used */
	uint16_t			qinq_outerid;			/**< Outer VLAN ID if Q-in-Q */
	uint16_t			qinq_innerid;			/**< Inner VLAN ID if Q-in-Q */
	uint32_t			gre_key;				/**< GRE key if used */

	uint16_t			pktSize;				/**< Size of packet in bytes not counting FCS */
	uint16_t			tlen;					/**< Total length of packet data */
	/* 28 bytes + (2 * sizeof(struct ether_addr)) */
	pkt_hdr_t			hdr;					/**< Packet header data */
	uint8_t				pad[DEFAULT_BUFF_SIZE - (sizeof(pkt_hdr_t) + (sizeof(struct ether_addr)*2) + 28)];
} pkt_seq_t;


struct port_info_s;

extern void pktgen_send_seq_pkt(struct port_info_s * info, uint32_t seq_idx);

extern void pktgen_page_seq(uint32_t pid);

#endif	// _PKTGEN_SEQ_H_
