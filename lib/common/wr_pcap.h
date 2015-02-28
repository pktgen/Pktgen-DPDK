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


#ifndef _WR_PCAP_H_
#define _WR_PCAP_H_

#include <netinet/in.h>

#include <rte_memory.h>


#define PCAP_MAGIC_NUMBER	0xa1b2c3d4
#define PCAP_MAJOR_VERSION	2
#define PCAP_MINOR_VERSION	4

typedef struct pcap_hdr_s {
        uint32_t magic_number;   /**< magic number */
        uint16_t version_major;  /**< major version number */
        uint16_t version_minor;  /**< minor version number */
        int32_t  thiszone;       /**< GMT to local correction */
        uint32_t sigfigs;        /**< accuracy of timestamps */
        uint32_t snaplen;        /**< max length of captured packets, in octets */
        uint32_t network;        /**< data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
        uint32_t ts_sec;         /**< timestamp seconds */
        uint32_t ts_usec;        /**< timestamp microseconds */
        uint32_t incl_len;       /**< number of octets of packet saved in file */
        uint32_t orig_len;       /**< actual length of packet */
} pcaprec_hdr_t;

typedef struct pcap_info_s {
	FILE	  * fd;				/**< File descriptor */
	char 	  * filename;		/**< allocated string for filename of pcap */
	uint32_t	endian;			/**< Endian flag value */
	uint32_t	pkt_size;		/**< Average packet size */
	uint32_t	pkt_count;		/**< pcap count of packets */
	uint32_t	pkt_idx;		/**< Index into the current PCAP file */
	pcap_hdr_t	info;			/**< information on the PCAP file */
} pcap_info_t;

#ifndef BIG_ENDIAN
#define BIG_ENDIAN		0x4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN	0x1234
#endif

typedef struct pcap_pkt_data_s {		/**< Keep these in this order as pkt_seq_t mirrors the first three objects */
	uint8_t			  * buffAddr;		/**< Start of buffer virtual address */
	uint8_t			  * virtualAddr;	/**< Pointer to the start of the packet data */
	phys_addr_t			physAddr;		/**< Packet physical address */
	uint32_t			size;			/**< Real packet size (hdr.incl_len) */
	pcaprec_hdr_t		hdr;			/**< Packet header from the .pcap file for each packet */
} pcap_pkt_data_t;

static __inline__ void
wr_pcap_convert(pcap_info_t * pcap, pcaprec_hdr_t * pHdr)
{
	if ( pcap->endian == BIG_ENDIAN ) {
		pHdr->incl_len	= ntohl(pHdr->incl_len);
		pHdr->orig_len	= ntohl(pHdr->orig_len);
		pHdr->ts_sec	= ntohl(pHdr->ts_sec);
		pHdr->ts_usec	= ntohl(pHdr->ts_usec);
	}
}

extern pcap_info_t * wr_pcap_open(char * filename, uint16_t port);
extern int wr_pcap_valid(char * filename);
extern void wr_pcap_close(pcap_info_t * pcap);
extern void wr_pcap_rewind(pcap_info_t * pcap);
extern void wr_pcap_skip(pcap_info_t * pcap, uint32_t skip);
extern void wr_pcap_info(pcap_info_t * pcap, uint16_t port, int flag);
extern size_t wr_pcap_read(pcap_info_t * pcap, pcaprec_hdr_t * pHdr, char * pktBuff, uint32_t bufLen );
extern int wr_payloadOffset(const unsigned char *pkt_data, unsigned int *offset,
                          unsigned int *length);

#endif /* _WR_PCAP_H_ */
