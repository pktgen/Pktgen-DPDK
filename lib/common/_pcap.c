/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2011 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>
#include <netinet/in.h>

#include <rte_version.h>
#include <rte_config.h>

#include <rte_log.h>
#include <rte_tailq.h>
#if defined(RTE_VER_MAJOR) && (RTE_VER_MAJOR < 2)
#include <rte_tailq_elem.h>
#endif
#include <rte_common.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_timer.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_hash.h>
#include <rte_lpm.h>
#include <rte_string_fns.h>
#include <rte_byteorder.h>
#include <rte_errno.h>

#include "_pcap.h"
#include "pg_inet.h"

/**************************************************************************//**
 *
 * pcap_open - Open a PCAP file.
 *
 * DESCRIPTION
 * Open a PCAP file to be used in sending from a port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

pcap_info_t *
_pcap_open(char *filename, uint16_t port)
{
	pcap_info_t   *pcap = NULL;

	if (filename == NULL) {
		printf("%s: filename is NULL\n", __FUNCTION__);
		goto leave;
	}

	pcap = (pcap_info_t *)rte_malloc_socket("PCAP info",
					 sizeof(pcap_info_t),
					 RTE_CACHE_LINE_SIZE,
					 rte_socket_id());
	if (pcap == NULL) {
		printf("%s: malloc failed for pcap_info_t structure\n",
		       __FUNCTION__);
		goto leave;
	}
	memset((char *)pcap, 0, sizeof(pcap_info_t));

	pcap->fd = fopen((const char *)filename, "r");
	if (pcap->fd == NULL) {
		printf("%s: failed for (%s)\n", __FUNCTION__, filename);
		goto leave;
	}

	if (fread(&pcap->info, 1, sizeof(pcap_hdr_t),
		  pcap->fd) != sizeof(pcap_hdr_t) ) {
		printf("%s: failed to read the file header\n", __FUNCTION__);
		goto leave;
	}

	/* Default to little endian format. */
	pcap->endian    = LITTLE_ENDIAN;
	pcap->filename  = strdup(filename);

	/* Make sure we have a valid PCAP file for Big or Little Endian formats. */
	if ( (pcap->info.magic_number != PCAP_MAGIC_NUMBER) &&
	     (pcap->info.magic_number != ntohl(PCAP_MAGIC_NUMBER)) ) {
		printf("%s: Magic Number does not match!\n", __FUNCTION__);
		fflush(stdout);
		goto leave;
	}

	/* Convert from big-endian to little-endian. */
	if (pcap->info.magic_number == ntohl(PCAP_MAGIC_NUMBER) ) {
		printf(
			"PCAP: Big Endian file format found, converting to little endian\n");
		pcap->endian                = BIG_ENDIAN;
		pcap->info.magic_number     = ntohl(pcap->info.magic_number);
		pcap->info.network          = ntohl(pcap->info.network);
		pcap->info.sigfigs          = ntohl(pcap->info.sigfigs);
		pcap->info.snaplen          = ntohl(pcap->info.snaplen);
		pcap->info.thiszone         = ntohl(pcap->info.thiszone);
		pcap->info.version_major    = ntohs(pcap->info.version_major);
		pcap->info.version_minor    = ntohs(pcap->info.version_minor);
	}
	_pcap_info(pcap, port, 0);

	return pcap;

leave:
	_pcap_close(pcap);
	fflush(stdout);

	return NULL;
}

/**************************************************************************//**
 *
 * pcap_info - Display the PCAP information.
 *
 * DESCRIPTION
 * Dump out the PCAP information.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
_pcap_info(pcap_info_t *pcap, uint16_t port, int flag)
{
	printf("\nPCAP file for port %d: %s\n", port, pcap->filename);
	printf("  magic: %08x,", pcap->info.magic_number);
	printf(" Version: %d.%d,",
	       pcap->info.version_major,
	       pcap->info.version_minor);
	printf(" Zone: %d,", pcap->info.thiszone);
	printf(" snaplen: %d,", pcap->info.snaplen);
	printf(" sigfigs: %d,", pcap->info.sigfigs);
	printf(" network: %d", pcap->info.network);
	printf(" Endian: %s\n", pcap->endian == BIG_ENDIAN ? "Big" : "Little");
	if (flag)
		printf("  Packet count: %d\n", pcap->pkt_count);
	printf("\n");
	fflush(stdout);
}

/**************************************************************************//**
 *
 * pcap_rewind - Rewind or start over on a PCAP file.
 *
 * DESCRIPTION
 * Rewind or start over on a PCAP file.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
_pcap_rewind(pcap_info_t *pcap)
{
	if (pcap == NULL)
		return;

	/* Rewind to the beginning */
	rewind(pcap->fd);

	/* Seek past the pcap header */
	(void)fseek(pcap->fd, sizeof(pcap_hdr_t), SEEK_SET);
}

/**************************************************************************//**
 *
 * pcap_skip - Rewind and skip to the given packet location.
 *
 * DESCRIPTION
 * Rewind and skip to the given packet location.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
_pcap_skip(pcap_info_t *pcap, uint32_t skip)
{
	pcaprec_hdr_t hdr, *phdr;

	if (pcap == NULL)
		return;

	/* Rewind to the beginning */
	rewind(pcap->fd);

	/* Seek past the pcap header */
	(void)fseek(pcap->fd, sizeof(pcap_hdr_t), SEEK_SET);

	phdr = &hdr;
	while (skip--) {
		if (fread(phdr, 1, sizeof(pcaprec_hdr_t),
			  pcap->fd) != sizeof(pcaprec_hdr_t) )
			break;

		/* Convert the packet header to the correct format. */
		_pcap_convert(pcap, phdr);

		(void)fseek(pcap->fd, phdr->incl_len, SEEK_CUR);
	}
}

/**************************************************************************//**
 *
 * pcap_close - Close a PCAP file
 *
 * DESCRIPTION
 * Close the PCAP file for sending.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
_pcap_close(pcap_info_t *pcap)
{
	if (pcap == NULL)
		return;

	if (pcap->fd)
		fclose(pcap->fd);
	if (pcap->filename)
		free(pcap->filename);
	rte_free(pcap);
}

/**************************************************************************//**
 *
 * pg_payloadOffset - Determine the packet data offset value.
 *
 * DESCRIPTION
 * Determine the packet data offset value in bytes.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

int
_pcap_payloadOffset(const unsigned char *pkt_data, unsigned int *offset,
		    unsigned int *length) {
	const struct ipv4_hdr *iph =
		(const struct ipv4_hdr *)(pkt_data + sizeof(struct ether_hdr));
	const struct tcp_hdr *th = NULL;

	/* Ignore packets that aren't IPv4 */
	if ( (iph->version_ihl & 0xF0) != 0x40)
		return -1;

	/* Ignore fragmented packets. */
	if (iph->fragment_offset & htons(PG_OFF_MF | PG_OFF_MASK))
		return -1;

	/* IP header length, and transport header length. */
	unsigned int ihlen = (iph->version_ihl & 0x0F) * 4;
	unsigned int thlen = 0;

	switch (iph->next_proto_id) {
	case PG_IPPROTO_TCP:
		th = (const struct tcp_hdr *)((const char *)iph + ihlen);
		thlen = (th->data_off >> 4) * 4;
		break;
	case PG_IPPROTO_UDP:
		thlen = sizeof(struct udp_hdr);
		break;
	default:
		return -1;
	}

	*offset = sizeof(struct ether_hdr) + ihlen + thlen;
	*length = sizeof(struct ether_hdr) + ntohs(iph->total_length) - *offset;

	return *length != 0;
}

/**************************************************************************//**
 *
 * pcap_read - Read data from the PCAP file and parse it
 *
 * DESCRIPTION
 * Parse the data from the PCAP file.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

size_t
_pcap_read(pcap_info_t *pcap,
	   pcaprec_hdr_t *pHdr,
	   char *pktBuff,
	   uint32_t bufLen)
{
	do {
		if (fread(pHdr, 1, sizeof(pcaprec_hdr_t),
			  pcap->fd) != sizeof(pcaprec_hdr_t) )
			return 0;

		/* Convert the packet header to the correct format. */
		_pcap_convert(pcap, pHdr);

		/* Skip packets larger then the buffer size. */
		if (pHdr->incl_len > bufLen) {
			(void)fseek(pcap->fd, pHdr->incl_len, SEEK_CUR);
			return pHdr->incl_len;
		}

		return fread(pktBuff, 1, pHdr->incl_len, pcap->fd);
	} while (1);
}
