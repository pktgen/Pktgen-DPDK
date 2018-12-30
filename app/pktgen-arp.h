/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_ARP_H_
#define _PKTGEN_ARP_H_

#include <stdint.h>

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

void pktgen_send_arp(uint32_t pid, uint32_t type, uint8_t seq_idx);
void pktgen_process_arp(struct rte_mbuf *m, uint32_t pid, uint32_t vlan);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_ARP_H_ */
