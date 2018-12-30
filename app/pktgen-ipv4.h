/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_IPV4_H_
#define _PKTGEN_IPV4_H_

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

void pktgen_ipv4_ctor(pkt_seq_t *pkt, void *hdr);
void pktgen_send_ping4(uint32_t pid, uint8_t seq_idx);
void pktgen_process_ping4(struct rte_mbuf *m, uint32_t pid, uint32_t vlan);

#ifdef __cplusplus
}
#endif

#endif  /*  _PKTGEN_IPV4_H_ */
