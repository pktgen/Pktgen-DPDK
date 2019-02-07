/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <rte_lua.h>
#include <rte_net.h>

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
pktgen_gre_hdr_ctor(port_info_t *info __rte_unused, pkt_seq_t *pkt,
		    greIp_t *gre)
{
	/* Zero out the header space */
	memset((char *)gre, 0, sizeof(greIp_t));

	/* Create the IP header */
	gre->ip.version_ihl = (IPv4_VERSION << 4) | (sizeof(struct ipv4_hdr) / 4);
	gre->ip.type_of_service = 0;
	gre->ip.total_length = htons(pkt->pktSize - pkt->ether_hdr_size);

	pktgen.ident += 27;	/* bump by a prime number */
	gre->ip.packet_id = htons(pktgen.ident);
	gre->ip.fragment_offset = 0;
	gre->ip.time_to_live = 64;
	gre->ip.next_proto_id= PG_IPPROTO_GRE;

	/* FIXME don't hardcode */
#define GRE_SRC_ADDR    (10 << 24) | (10 << 16) | (1 << 8) | 1
#define GRE_DST_ADDR    (10 << 24) | (10 << 16) | (1 << 8) | 2
	gre->ip.src_addr = htonl(GRE_SRC_ADDR);
	gre->ip.dst_addr = htonl(GRE_DST_ADDR);
#undef GRE_SRC_ADDR
#undef GRE_DST_ADDR

	gre->ip.hdr_checksum = rte_ipv4_cksum(&gre->ip);

	/* Create the GRE header */
	gre->gre.chk_present = 0;
	gre->gre.unused      = 0;
	gre->gre.key_present = 1;
	gre->gre.seq_present = 0;

	gre->gre.reserved0_0 = 0;
	gre->gre.reserved0_1 = 0;

	gre->gre.version     = 0;
	gre->gre.eth_type    = htons(ETHER_TYPE_IPv4);	/* FIXME get EtherType of the actual encapsulated packet instead of
							 * defaulting to IPv4 */

	int extra_count = 0;
	if (gre->gre.chk_present)
		/* The 16 MSBs of gre->gre.extra_fields[0] must be set to the IP (one's */
		/* complement) checksum of the GRE header and the payload packet. */
		/* Since the packet is still under construction at this moment, the */
		/* checksum cannot be calculated. We just record the presence of this */
		/* field, so the correct header length can be calculated. */
		++extra_count;

	if (gre->gre.key_present) {
		gre->gre.extra_fields[extra_count] = htonl(pkt->gre_key);
		++extra_count;
	}

	if (gre->gre.seq_present)
		/* gre->gre.extra_fields[extra_count] = htonl(<SEQ_NR>); */
		/* TODO implement GRE sequence numbers */
		++extra_count;

	/* 4 * (3 - extra_count) is the amount of bytes that are not used by */
	/* optional fields, but are included in sizeof(greIp_t). */
	pkt->ether_hdr_size += sizeof(greIp_t) - 4 * (3 - extra_count);
	return (char *)(gre + 1) - 4 * (3 - extra_count);
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
pktgen_gre_ether_hdr_ctor(port_info_t *info __rte_unused,
			  pkt_seq_t *pkt,
			  greEther_t *gre)
{
	/* Zero out the header space */
	memset((char *)gre, 0, sizeof(greEther_t));

	/* Create the IP header */
	gre->ip.version_ihl = (IPv4_VERSION << 4) | (sizeof(struct ipv4_hdr) / 4);
	gre->ip.type_of_service = 0;
	gre->ip.total_length = htons(pkt->pktSize - pkt->ether_hdr_size);

	pktgen.ident += 27;	/* bump by a prime number */
	gre->ip.packet_id = htons(pktgen.ident);
	gre->ip.fragment_offset = 0;
	gre->ip.time_to_live = 64;
	gre->ip.next_proto_id = PG_IPPROTO_GRE;

	/* FIXME don't hardcode */
#define GRE_SRC_ADDR    (10 << 24) | (10 << 16) | (1 << 8) | 1
#define GRE_DST_ADDR    (10 << 24) | (10 << 16) | (1 << 8) | 2
	gre->ip.src_addr = htonl(GRE_SRC_ADDR);
	gre->ip.dst_addr = htonl(GRE_DST_ADDR);
#undef GRE_SRC_ADDR
#undef GRE_DST_ADDR

	gre->ip.hdr_checksum = rte_ipv4_cksum(&gre->ip);

	/* Create the GRE header */
	gre->gre.chk_present = 0;
	gre->gre.unused      = 0;
	gre->gre.key_present = 1;
	gre->gre.seq_present = 0;

	gre->gre.reserved0_0 = 0;
	gre->gre.reserved0_1 = 0;

	gre->gre.version     = 0;
	gre->gre.eth_type    = htons(ETHER_TYPE_TRANSP_ETH_BR);

	int extra_count = 0;
	if (gre->gre.chk_present)
		/* The 16 MSBs of gre->gre.extra_fields[0] must be set to the IP (one's */
		/* complement) checksum of the GRE header and the payload packet. */
		/* Since the packet is still under construction at this moment, the */
		/* checksum cannot be calculated. We just record the presence of this */
		/* field, so the correct header length can be calculated. */
		++extra_count;

	if (gre->gre.key_present) {
		gre->gre.extra_fields[extra_count] = htonl(pkt->gre_key);
		++extra_count;
	}

	if (gre->gre.seq_present)
		/* gre->gre.extra_fields[extra_count] = htonl(<SEQ_NR>); */
		/* TODO implement GRE sequence numbers */
		++extra_count;

	/* Inner Ethernet header. Exact offset of start of ethernet header depends
	 * on the presence of optional fields in the GRE header. */
	struct ether_hdr *eth_hdr = (struct ether_hdr *)((char *)&gre->gre
							 + 2			/* Flags and version */
							 + 2			/* Protocol type */
							 + 4 * extra_count);	/* 4 bytes per optional field */
	ether_addr_copy(&pkt->eth_src_addr, &eth_hdr->s_addr);			/* FIXME get inner Ethernet parameters from user */
	ether_addr_copy(&pkt->eth_dst_addr, &eth_hdr->d_addr);			/* FIXME get inner Ethernet parameters from user */
	eth_hdr->ether_type = htons(ETHER_TYPE_IPv4);				/* FIXME get Ethernet type from actual encapsulated
										 * packet instead of hardcoding */

	/* 4 * (3 - extra_count) is the amount of bytes that are not used by */
	/* optional fields, but are included in sizeof(greIp_t). */
	pkt->ether_hdr_size += sizeof(greEther_t) - 4 * (3 - extra_count);
	return (char *)(gre + 1) - 4 * (3 - extra_count);
}
