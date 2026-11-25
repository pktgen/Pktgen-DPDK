/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_ARP_H_
#define _PKTGEN_ARP_H_

/**
 * @file
 *
 * ARP packet transmission, processing, and debug dump for Pktgen.
 */

#include <stdint.h>

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Transmit an ARP request or reply on port @p pid.
 *
 * @param pid
 *   Port index to send the ARP packet on.
 * @param type
 *   ARP operation type (e.g. RTE_ARP_OP_REQUEST or RTE_ARP_OP_REPLY).
 * @param seq_idx
 *   Packet sequence slot index used as the ARP template.
 */
void pktgen_send_arp(uint32_t pid, uint32_t type, uint8_t seq_idx);

/**
 * Process a received ARP packet and send a reply if warranted.
 *
 * @param m
 *   Received mbuf containing the ARP packet.
 * @param pid
 *   Port index on which the packet arrived.
 * @param qid
 *   Queue index on which the packet arrived.
 * @param vlan
 *   VLAN ID extracted from the outer header (0 if untagged).
 */
void pktgen_process_arp(struct rte_mbuf *m, uint32_t pid, uint32_t qid, uint32_t vlan);

/**
 * Hex-dump the ARP fields of mbuf @p m to stdout for debugging.
 *
 * @param m
 *   Mbuf containing the ARP packet to dump.
 */
void arp_pkt_dump(struct rte_mbuf *m);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_ARP_H_ */
