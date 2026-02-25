/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_UDP_H_
#define _PKTGEN_UDP_H_

/**
 * @file
 *
 * UDP header construction for Pktgen transmit packets.
 */

#include <pg_inet.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VXLAN_PORT_ID 4789 /**< Well-known UDP port number for VxLAN encapsulation */

/**
 * Construct a UDP header in the packet buffer.
 *
 * @param pkt
 *   Packet sequence entry providing source/destination ports and payload length.
 * @param hdr
 *   Pointer to the start of the UDP header region in the packet buffer.
 * @param type
 *   EtherType / address family identifier (e.g. RTE_ETHER_TYPE_IPV4).
 * @param cksum_offload
 *   When true, set the checksum to 0 and rely on hardware offload;
 *   when false, compute the checksum in software.
 * @param cksum_requires_phdr
 *   When true, include the IPv4/IPv6 pseudo-header in the checksum seed.
 * @return
 *   Pointer to the byte immediately following the completed UDP header.
 */
void *pktgen_udp_hdr_ctor(pkt_seq_t *pkt, void *hdr, int type, bool cksum_offload,
                          bool cksum_requires_phdr);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_UDP_H_ */
