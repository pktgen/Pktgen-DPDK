/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_IPV4_H_
#define _PKTGEN_IPV4_H_

/**
 * @file
 *
 * IPv4 header construction and ICMPv4 ping handling for Pktgen.
 */

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Construct the IPv4 header for a packet template.
 *
 * @param pkt
 *   Packet sequence entry providing IP addresses, protocol, and length.
 * @param hdr
 *   Pointer to the start of the IPv4 header region in the packet buffer.
 * @param cksum_offload
 *   When true, set the checksum to 0 and rely on hardware offload;
 *   when false, compute the checksum in software.
 */
void pktgen_ipv4_ctor(pkt_seq_t *pkt, void *hdr, bool cksum_offload);

/**
 * Transmit an ICMPv4 echo-request (ping) on port @p pid.
 *
 * @param pid
 *   Port index to send the ping on.
 * @param seq_idx
 *   Packet sequence slot index to use as the ping template.
 */
void pktgen_send_ping4(uint32_t pid, uint8_t seq_idx);

/**
 * Process a received ICMPv4 echo-reply and update port statistics.
 *
 * @param m
 *   Received mbuf containing the ICMP reply.
 * @param pid
 *   Port index on which the packet arrived.
 * @param qid
 *   Queue index on which the packet arrived.
 * @param vlan
 *   VLAN ID extracted from the packet (0 if untagged).
 */
void pktgen_process_ping4(struct rte_mbuf *m, uint32_t pid, uint32_t qid, uint32_t vlan);

#ifdef __cplusplus
}
#endif

#endif /*  _PKTGEN_IPV4_H_ */
