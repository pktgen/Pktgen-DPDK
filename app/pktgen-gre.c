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

#include "pktgen-gre.h"
#include "pktgen.h"

/**************************************************************************//**
*
* pktgen_gre_hdr_ctor - IPv4/GRE header construction routine.
*
* DESCRIPTION
* Construct an IPv4/GRE header in a packet buffer.
*
* RETURNS: Pointer to memory after the GRE header.
*
* SEE ALSO:
*/

char *
pktgen_gre_hdr_ctor(__attribute__ ((unused)) port_info_t * info, pkt_seq_t * pkt, greIp_t * gre)
{
	// Zero out the header space
	memset((char *)gre, 0, sizeof(greIp_t));

	// Create the IP header
	gre->ip.vl		 = (IPv4_VERSION << 4) | (sizeof(ipHdr_t) /4);
	gre->ip.tos		 = 0;
	gre->ip.tlen	 = htons(pkt->pktSize - pkt->ether_hdr_size);

	pktgen.ident	+= 27; 		// bump by a prime number
	gre->ip.ident	 = htons(pktgen.ident);
	gre->ip.ffrag	 = 0;
	gre->ip.ttl		 = 64;
	gre->ip.proto	 = PG_IPPROTO_GRE;

	// FIXME don't hardcode
#define GRE_SRC_ADDR	(10 << 24) | (10 << 16) | (1 << 8) | 1
#define GRE_DST_ADDR	(10 << 24) | (10 << 16) | (1 << 8) | 2
	gre->ip.src		 = htonl(GRE_SRC_ADDR);
	gre->ip.dst		 = htonl(GRE_DST_ADDR);
#undef GRE_SRC_ADDR
#undef GRE_DST_ADDR

	gre->ip.cksum	 = cksum(gre, sizeof(ipHdr_t), 0);

	// Create the GRE header
	gre->gre.chk_present = 0;
	gre->gre.unused      = 0;
	gre->gre.key_present = 1;
	gre->gre.seq_present = 0;

	gre->gre.reserved0_0 = 0;
	gre->gre.reserved0_1 = 0;

	gre->gre.version     = 0;
	gre->gre.eth_type    = htons(ETHER_TYPE_IPv4); 	// FIXME get EtherType of the actual encapsulated packet instead of defaulting to IPv4

	int extra_count = 0;
	if (gre->gre.chk_present) {
		// The 16 MSBs of gre->gre.extra_fields[0] must be set to the IP (one's
		// complement) checksum of the GRE header and the payload packet.
		// Since the packet is still under construction at this moment, the
		// checksum cannot be calculated. We just record the presence of this
		// field, so the correct header length can be calculated.
		++extra_count;
	}

	if (gre->gre.key_present) {
		gre->gre.extra_fields[extra_count] = htonl(pkt->gre_key);
		++extra_count;
	}

	if (gre->gre.seq_present) {
		// gre->gre.extra_fields[extra_count] = htonl(<SEQ_NR>);
		// TODO implement GRE sequence numbers
		++extra_count;
	}

	// 4 * (3 - extra_count) is the amount of bytes that are not used by
	// optional fields, but are included in sizeof(greIp_t).
	pkt->ether_hdr_size += sizeof(greIp_t) - 4 * (3 - extra_count);
	return ((char *)(gre + 1) - 4 * (3 - extra_count));
}


/**************************************************************************//**
*
* pktgen_gre_ether_hdr_ctor - GRE/Ethernet header construction routine.
*
* DESCRIPTION
* Construct a GRE/Ethernet header in a packet buffer.
*
* RETURNS: Pointer to memory after the GRE header.
*
* SEE ALSO:
*/

char *
pktgen_gre_ether_hdr_ctor(__attribute__ ((unused)) port_info_t * info, pkt_seq_t * pkt, greEther_t * gre)
{
	// Zero out the header space
	memset((char *)gre, 0, sizeof(greEther_t));

	// Create the IP header
	gre->ip.vl		 = (IPv4_VERSION << 4) | (sizeof(ipHdr_t) /4);
	gre->ip.tos		 = 0;
	gre->ip.tlen	 = htons(pkt->pktSize - pkt->ether_hdr_size);

	pktgen.ident	+= 27; 		// bump by a prime number
	gre->ip.ident	 = htons(pktgen.ident);
	gre->ip.ffrag	 = 0;
	gre->ip.ttl		 = 64;
	gre->ip.proto	 = PG_IPPROTO_GRE;

	// FIXME don't hardcode
#define GRE_SRC_ADDR	(10 << 24) | (10 << 16) | (1 << 8) | 1
#define GRE_DST_ADDR	(10 << 24) | (10 << 16) | (1 << 8) | 2
	gre->ip.src		 = htonl(GRE_SRC_ADDR);
	gre->ip.dst		 = htonl(GRE_DST_ADDR);
#undef GRE_SRC_ADDR
#undef GRE_DST_ADDR

	gre->ip.cksum	 = cksum(gre, sizeof(ipHdr_t), 0);

	// Create the GRE header
	gre->gre.chk_present = 0;
	gre->gre.unused      = 0;
	gre->gre.key_present = 1;
	gre->gre.seq_present = 0;

	gre->gre.reserved0_0 = 0;
	gre->gre.reserved0_1 = 0;

	gre->gre.version     = 0;
	gre->gre.eth_type    = htons(ETHER_TYPE_TRANSP_ETH_BR);

	int extra_count = 0;
	if (gre->gre.chk_present) {
		// The 16 MSBs of gre->gre.extra_fields[0] must be set to the IP (one's
		// complement) checksum of the GRE header and the payload packet.
		// Since the packet is still under construction at this moment, the
		// checksum cannot be calculated. We just record the presence of this
		// field, so the correct header length can be calculated.
		++extra_count;
	}

	if (gre->gre.key_present) {
		gre->gre.extra_fields[extra_count] = htonl(pkt->gre_key);
		++extra_count;
	}

	if (gre->gre.seq_present) {
		// gre->gre.extra_fields[extra_count] = htonl(<SEQ_NR>);
		// TODO implement GRE sequence numbers
		++extra_count;
	}

	/* Inner Ethernet header. Exact offset of start of ethernet header depends
	 * on the presence of optional fields in the GRE header. */
	struct ether_hdr *eth_hdr = (struct ether_hdr *)((char *)&gre->gre
			+ 2						// Flags and version
			+ 2						// Protocol type
			+ 4 * extra_count);		// 4 bytes per optional field
	ether_addr_copy(&pkt->eth_src_addr, &eth_hdr->s_addr);	// FIXME get inner Ethernet parameters from user
	ether_addr_copy(&pkt->eth_dst_addr, &eth_hdr->d_addr);	// FIXME get inner Ethernet parameters from user
	eth_hdr->ether_type = htons(ETHER_TYPE_IPv4);			// FIXME get Ethernet type from actual encapsulated packet instead of hardcoding

	// 4 * (3 - extra_count) is the amount of bytes that are not used by
	// optional fields, but are included in sizeof(greIp_t).
	pkt->ether_hdr_size += sizeof(greEther_t) - 4 * (3 - extra_count);
	return ((char *)(gre + 1) - 4 * (3 - extra_count));
}
