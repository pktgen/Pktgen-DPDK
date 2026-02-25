/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_TCP_H_
#define _PKTGEN_TCP_H_

/**
 * @file
 *
 * TCP header construction for Pktgen transmit packets.
 */

#include <pg_inet.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Construct a TCP header in the packet buffer.
 *
 * @param pkt
 *   Packet sequence entry providing source/destination ports and TCP flags.
 * @param hdr
 *   Pointer to the start of the TCP header region in the packet buffer.
 * @param type
 *   EtherType / address family identifier (e.g. RTE_ETHER_TYPE_IPV4).
 * @param cksum_offload
 *   When true, set the checksum to 0 and rely on hardware offload;
 *   when false, compute the checksum in software.
 * @param cksum_requires_phdr
 *   When true, include the IPv4/IPv6 pseudo-header in the checksum seed.
 * @return
 *   Pointer to the byte immediately following the completed TCP header.
 */
void *pktgen_tcp_hdr_ctor(pkt_seq_t *pkt, void *hdr, int type, bool cksum_offload,
                          bool cksum_requires_phdr);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_TCP_H_ */
