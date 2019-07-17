/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PCAP_H_
#define _PCAP_H_

#include <netinet/in.h>

#include <rte_memory.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCAP_MAGIC_NUMBER   0xa1b2c3d4
#define PCAP_MAJOR_VERSION  2
#define PCAP_MINOR_VERSION  4

typedef struct pcap_hdr_s {
	uint32_t magic_number;	/**< magic number */
	uint16_t version_major;	/**< major version number */
	uint16_t version_minor;	/**< minor version number */
	int32_t thiszone;	/**< GMT to local correction */
	uint32_t sigfigs;	/**< accuracy of timestamps */
	uint32_t snaplen;	/**< max length of captured packets, in octets */
	uint32_t network;	/**< data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
	uint32_t ts_sec;	/**< timestamp seconds */
	uint32_t ts_usec;	/**< timestamp microseconds */
	uint32_t incl_len;	/**< number of octets of packet saved in file */
	uint32_t orig_len;	/**< actual length of packet */
} pcaprec_hdr_t;

typedef struct pcap_info_s {
	FILE      *fd;		/**< File descriptor */
	char      *filename;	/**< allocated string for filename of pcap */
	uint32_t endian;	/**< Endian flag value */
	uint32_t pkt_size;	/**< Average packet size */
	uint32_t pkt_count;	/**< pcap count of packets */
	uint32_t pkt_idx;	/**< Index into the current PCAP file */
	pcap_hdr_t info;	/**< information on the PCAP file */
} pcap_info_t;

#ifndef BIG_ENDIAN
#define BIG_ENDIAN      0x4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN   0x1234
#endif

typedef struct pcap_pkt_data_s {	/**< Keep these in this order as pkt_seq_t mirrors the first three objects */
	uint8_t           *buffAddr;	/**< Start of buffer virtual address */
	uint8_t           *virtualAddr;	/**< Pointer to the start of the packet data */
	phys_addr_t physAddr;		/**< Packet physical address */
	uint32_t size;			/**< Real packet size (hdr.incl_len) */
	pcaprec_hdr_t hdr;		/**< Packet header from the .pcap file for each packet */
} pcap_pkt_data_t;

static __inline__ void
_pcap_convert(pcap_info_t *pcap, pcaprec_hdr_t *pHdr)
{
	if (pcap->endian == BIG_ENDIAN) {
		pHdr->incl_len  = ntohl(pHdr->incl_len);
		pHdr->orig_len  = ntohl(pHdr->orig_len);
		pHdr->ts_sec    = ntohl(pHdr->ts_sec);
		pHdr->ts_usec   = ntohl(pHdr->ts_usec);
	}
}

pcap_info_t *_pcap_open(char *filename, uint16_t port);
int _pcap_valid(char *filename);
void _pcap_close(pcap_info_t *pcap);
void _pcap_rewind(pcap_info_t *pcap);
void _pcap_skip(pcap_info_t *pcap, uint32_t skip);
void _pcap_info(pcap_info_t *pcap, uint16_t port, int flag);
size_t _pcap_read(pcap_info_t *pcap,
			 pcaprec_hdr_t *pHdr,
			 char *pktBuff,
			 uint32_t bufLen);
int _pcap_payloadOffset(const unsigned char *pkt_data, unsigned int *offset,
			       unsigned int *length);

#ifdef __cplusplus
}
#endif

#endif /* _PCAP_H_ */
