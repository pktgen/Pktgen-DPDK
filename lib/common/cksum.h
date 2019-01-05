/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef __CKSUM_H
#define __CKSUM_H

#ifdef __cplusplus
extern "C" {
#endif

uint16_t     cksum(void *pBuf, int32_t size, uint32_t cksum);
uint32_t     cksumUpdate(void *pBuf, int32_t size, uint32_t cksum);
uint16_t     cksumDone(uint32_t cksum);
uint32_t     pseudoChecksum(uint32_t src, uint32_t dst,
				   uint16_t proto, uint16_t len,
				   uint32_t cksum);
uint32_t     pseudoIPv6Checksum(uint16_t *src,
				       uint16_t *dst,
				       uint8_t next_hdr,
				       uint32_t total_len,
				       uint32_t sum);

#ifdef __cplusplus
}
#endif

#endif /* __CKSUM_H */
