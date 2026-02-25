/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_IPV6_H_
#define _PKTGEN_IPV6_H_

/**
 * @file
 *
 * IPv6 header construction and ICMPv6 ping handling for Pktgen.
 */

#include "pktgen.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Construct an IPv6 header in the packet buffer.
 *
 * @param pkt
 *   Packet sequence entry providing source/destination IPv6 addresses,
 *   traffic class, flow label, and next-header type.
 * @param hdr
 *   Pointer to the start of the IPv6 header region in the packet buffer.
 */
void pktgen_ipv6_ctor(pkt_seq_t *pkt, void *hdr);

/**
 * Process a received ICMPv6 echo-reply (ping6 response).
 *
 * @param m
 *   Received mbuf containing the ICMPv6 reply.
 * @param pid
 *   Port index on which the packet arrived.
 * @param qid
 *   Queue index on which the packet arrived.
 * @param vlan
 *   VLAN ID extracted from the outer header (0 if untagged).
 */
void pktgen_process_ping6(struct rte_mbuf *m, uint32_t pid, uint32_t qid, uint32_t vlan);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_IPV6_H_ */
