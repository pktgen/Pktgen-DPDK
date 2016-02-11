/*-
 * Copyright (c) <2015>, Intel Corporation
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
/* Created 2015 by abhinandan.gujjar@intel.com */

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
pktgen_gtpu_hdr_ctor(pkt_seq_t *pkt, gtpuHdr_t *guip, uint16_t ipProto)
{
	unsigned char *buffer = (unsigned char *)guip;
	unsigned int l4HdrSize = 0;

	if (ipProto == PG_IPPROTO_UDP)
		l4HdrSize = sizeof(udpHdr_t);
	else
		l4HdrSize = sizeof(tcpHdr_t);

	gtpuHdr_t *gtppHdr =
	        (gtpuHdr_t *)(buffer + sizeof(ipHdr_t) + l4HdrSize);

	/* Zero out the header space */
	memset((char *)guip, 0, sizeof(gtpuHdr_t));

	/* Version: It is a 3-bit field. For GTPv1, this has a value of 1. */
	gtppHdr->version_flags = GTPu_VERSION;

	/* Message Type: an 8-bit field that indicates the type of GTP message.
	 * Different types of messages are defined in 3GPP TS 29.060 section 7.1
	 */
	gtppHdr->msg_type = 0xff;

	/* Message Length - a 16-bit field that indicates the length of the payload in bytes
	 * (rest of the packet following the mandatory 8-byte GTP header). Includes the optional fields.
	 */
	gtppHdr->tot_len =
	        htons(pkt->pktSize -
	              (pkt->ether_hdr_size + sizeof(ipHdr_t) + l4HdrSize +
	               sizeof(gtpuHdr_t)));

	/* Tunnel endpoint identifier (TEID)
	 * A 32-bit(4-octet) field used to multiplex different connections in the same GTP tunnel.
	 */
	gtppHdr->teid = htonl(pkt->gtpu_teid);

	/* Sequence number an (optional) 16-bit field.
	 * This field exists if any of the E, S, or PN bits are on. The field must be interpreted only if the S bit is on.
	 */
	gtppHdr->seq_no = 0x0;

	/* N-PDU number an (optional) 8-bit field. This field exists if any of the E, S, or PN bits are on.
	 * The field must be interpreted only if the PN bit is on.
	 */
	gtppHdr->npdu_no = 0x0;

	/* Next extension header type an (optional) 8-bit field. This field exists if any of the E, S, or PN bits are on.
	 * The field must be interpreted only if the E bit is on.
	 */
	gtppHdr->next_ext_hdr_type = 0x0;
}
