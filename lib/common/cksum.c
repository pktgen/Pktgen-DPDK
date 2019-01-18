/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <assert.h>

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
#include <rte_lpm.h>
#include <rte_string_fns.h>
#include <rte_byteorder.h>
#include <rte_errno.h>

#include "copyright_info.h"
#include "port_config.h"

#include "pg_inet.h"
#include "mbuf.h"
#include "cksum.h"

/**************************************************************************//**
 * cksum - Compute a 16 bit ones complement checksum value.
 *
 * DESCRIPTION
 * A wrapper routine to compute the complete 16 bit checksum value for a given
 * piece of memory, when the data is contiguous. The <cksum> value is a previous
 * checksum value to allow the user to build a checksum using different parts
 * of memory.
 *
 * \is
 * \i <pBuf> Pointer to the data buffer to be checksumed.
 * \i <size> Number of bytes to checksum.
 * \i <cksum> Previous checksum value else give 0.
 * \ie
 *
 * RETURNS: 16 bit checksum value.
 *
 * ERRNO: N/A
 */
uint16_t
cksum(void *pBuf, int32_t size, uint32_t cksum)
{
	return cksumDone(cksumUpdate(pBuf, size, cksum) );
}

/**************************************************************************//**
 * cksumUpdate - Calaculate an 16 bit checksum and return the 32 bit value
 *
 * DESCRIPTION
 * Will need to call pktgen_cksumDone to finish computing the checksum. The <cksum>
 * value is from any previous checksum call. The routine will not fold the upper
 * 16 bits into the 32 bit checksum. The pktgen_cksumDone routine will do the
 * folding of the upper 16 bits into a 16 bit checksum.
 *
 * \is
 * \i <pBuf> the pointer to the data to be checksumed.
 * \i <size> the number of bytes to include in the checksum calculation.
 * \i <cksum> the initial starting checksum value allowing the developer to
 * ckecksum different pieces of memory to get a final value.
 * \ie
 *
 * RETURNS: unsigned 32 bit checksum value.
 *
 * ERRNO: N/A
 */
uint32_t
cksumUpdate(void *pBuf, int32_t size, uint32_t cksum)
{
	uint32_t nWords;
	uint16_t     *pWd = (uint16_t *)pBuf;

	for (nWords = (size >> 5); nWords > 0; nWords--) {
		cksum += *pWd++; cksum += *pWd++; cksum += *pWd++;
		cksum += *pWd++;
		cksum += *pWd++; cksum += *pWd++; cksum += *pWd++;
		cksum += *pWd++;
		cksum += *pWd++; cksum += *pWd++; cksum += *pWd++;
		cksum += *pWd++;
		cksum += *pWd++; cksum += *pWd++; cksum += *pWd++;
		cksum += *pWd++;
	}

	/* handle the odd number size */
	for (nWords = (size & 0x1f) >> 1; nWords > 0; nWords--)
		cksum   += *pWd++;

	/* Handle the odd byte length */
	if (size & 1)
		cksum   += *pWd & htons(0xFF00);

	return cksum;
}

/**************************************************************************//**
 * cksumDone - Finish up the ckecksum value by folding the checksum.
 *
 * DESCRIPTION
 * Fold the carry bits back into the checksum value to complete the 16 bit
 * checksum value. This routine is called after all of the pktgen_cksumUpdate
 * calls have been completed and the 16bit result is required.
 *
 * \is
 * \i <cksum> the initial 32 bit checksum and returns a 16bit folded value.
 * \ie
 *
 * RETURNS: 16 bit checksum value.
 *
 * ERRNO: N/A
 */
uint16_t
cksumDone(uint32_t cksum)
{
	/* Fold at most twice */
	cksum = (cksum & 0xFFFF) + (cksum >> 16);
	cksum = (cksum & 0xFFFF) + (cksum >> 16);

	return ~((uint16_t)cksum);
}

/**************************************************************************//**
 * pseudoChecksum - Compute the Pseudo Header checksum.
 *
 * DESCRIPTION
 * The pseudo header checksum is done in IP for TCP/UDP by computing the values
 * passed into the routine into a return value, which is a 32bit checksum. The
 * 32bit value contains any carry bits and will be added to the final value.
 *
 * \is
 * \i <src> Source IP address.
 * \i <dst> Destination IP address.
 * \i <pro> The protocol type.
 * \i <len> Length of the data packet.
 * \i <sum> Previous checksum value if needed.
 * \ie
 *
 * RETURNS: 32bit checksum value.
 *
 * ERRNO: N/A
 */
uint32_t
pseudoChecksum(uint32_t src, uint32_t dst, uint16_t pro, uint16_t len,
	       uint32_t sum)
{
	/* Compute the Pseudo Header checksum */
	return sum + (src & 0xFFFF) + (src >> 16) + (dst & 0xFFFF) +
	       (dst >> 16) +
	       ntohs(len) + ntohs(pro);
}

/**************************************************************************//**
 * pseudoIPv6Checksum - Compute the Pseudo Header checksum.
 *
 * DESCRIPTION
 * The pseudo header checksum is done in IP for TCP/UDP by computing the values
 * passed into the routine into a return value, which is a 32bit checksum. The
 * 32bit value contains any carry bits and will be added to the final value.
 *
 * \is
 * \i <src> Source IP address pointer.
 * \i <dst> Destination IP address pointer.
 * \i <next_hdr> The protocol type.
 * \i <total_len> Length of the data packet TCP data.
 * \i <sum> Previous checksum value if needed.
 * \ie
 *
 * RETURNS: 32bit checksum value.
 *
 * ERRNO: N/A
 */
uint32_t
pseudoIPv6Checksum(uint16_t *src,
		   uint16_t *dst,
		   uint8_t next_hdr,
		   uint32_t total_len,
		   uint32_t sum)
{
	uint32_t len = htonl(total_len), i;

	sum = (sum + (uint16_t)next_hdr + (len & 0xFFFF) + (len >> 16));

	for (i = 0; i < 8; i++) {
		sum += src[i];
		sum += dst[i];
	}
	return sum;
}
