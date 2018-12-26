/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_IPV6_H_
#define _PKTGEN_IPV6_H_

#include "pktgen.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 *
 * pktgen_ipv6_ctor - IPv6 packet header constructor routine.
 *
 * DESCRIPTION
 * Construct the IPv6 header constructor routine.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void pktgen_ipv6_ctor(pkt_seq_t *pkt, void *hdr);

void pktgen_process_ping6(struct rte_mbuf *m, uint32_t pid,
				 uint32_t vlan);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_IPV6_H_ */
