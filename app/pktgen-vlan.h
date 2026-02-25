/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_VLAN_H_
#define _PKTGEN_VLAN_H_

/**
 * @file
 *
 * VLAN packet processing routines for Pktgen.
 */

#include <stdint.h>

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Process a received VLAN-tagged packet and update port statistics.
 *
 * @param m
 *   Pointer to the received mbuf.
 * @param pid
 *   Port index on which the packet was received.
 * @param qid
 *   Queue index on which the packet was received.
 */
void pktgen_process_vlan(struct rte_mbuf *m, uint32_t pid, uint32_t qid);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_VLAN_H_ */
