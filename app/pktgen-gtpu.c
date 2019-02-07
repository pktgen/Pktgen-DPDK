/*-
 * Copyright (c) <2015-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2015 by abhinandan.gujjar@intel.com */

#include <cli_scrn.h>
#include "rte_lua.h"

#include "pktgen.h"

#include "pktgen-gtpu.h"

/**************************************************************************//**
 *
 * pktgen_gtpu_udp_hdr_ctor - GTP-U header constructor routine for TCP/UDP.
 *
 * DESCRIPTION
 * Construct the GTP-U header in a packer buffer.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_gtpu_hdr_ctor(pkt_seq_t *pkt, void *hdr, uint16_t ipProto,
		uint8_t flags, uint16_t seq_no, uint8_t npdu_no,
		uint8_t next_ext_hdr_type)
{
	unsigned int l4HdrSize;
	unsigned int options = 0;
	gtpuHdr_t *gtppHdr;
	void *p;

	if (ipProto == PG_IPPROTO_UDP)
		l4HdrSize = sizeof(struct udp_hdr);
	else
		l4HdrSize = sizeof(struct tcp_hdr);

	gtppHdr = (gtpuHdr_t *)RTE_PTR_ADD(hdr, sizeof(struct ipv4_hdr) + l4HdrSize);

	/* Zero out the header space */
	memset((char *)gtppHdr, 0, sizeof(gtpuHdr_t));

	/* Version: It is a 3-bit field. For GTPv1, this has a value of 1. */
	gtppHdr->version_flags = flags;

	/* Message Type: an 8-bit field that indicates the type of GTP message.
	 * Different types of messages are defined in 3GPP TS 29.060 section 7.1
	 */
	gtppHdr->msg_type = 0xff;

	/* Tunnel endpoint identifier (TEID)
	 * A 32-bit(4-octet) field used to multiplex different connections in the
	 * same GTP tunnel.
	 */
	gtppHdr->teid = htonl(pkt->gtpu_teid);

	p = (void *)gtppHdr;

	if (gtppHdr->version_flags & GTPu_S_FLAG) {
		/* Sequence number an (optional) 16-bit field.
		 * This field exists if any of the E, S, or PN bits are on. The field
		 * must be interpreted only if the S bit is on.
		 */
		*(uint16_t *)p = seq_no;
		p = RTE_PTR_ADD(p, 2);
		gtppHdr->tot_len += 2;
		options += 2;
	}

	if (gtppHdr->version_flags & GTPu_PN_FLAG) {
		/* N-PDU number an (optional) 8-bit field. This field exists if any of
		 * the E, S, or PN bits are on.
		 * The field must be interpreted only if the PN bit is on.
		 */
		*(uint8_t *)p = npdu_no;
		p = RTE_PTR_ADD(p, 1);
		gtppHdr->tot_len++;
		options++;
	}

	if (gtppHdr->version_flags & GTPu_E_FLAG) {
		/* Next extension header type an (optional) 8-bit field. This field
		 * exists if any of the E, S, or PN bits are on.
		 * The field must be interpreted only if the E bit is on.
		 */
		*(uint8_t *)p = next_ext_hdr_type;
		p = RTE_PTR_ADD(p, 1);
		gtppHdr->tot_len++;
		options++;
	}

	/* Message Length - a 16-bit field that indicates the length of the payload
	 * in bytes (rest of the packet following the mandatory 8-byte GTP header).
	 * Includes the optional fields.
	 */
	gtppHdr->tot_len = htons(pkt->pktSize - (l4HdrSize + sizeof(gtpuHdr_t) +
			pkt->ether_hdr_size));
}
