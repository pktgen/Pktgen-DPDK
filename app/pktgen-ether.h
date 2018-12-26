/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_ETHER_H_
#define _PKTGEN_ETHER_H_

#include <rte_ethdev.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rte_eth_stats eth_stats_t;

struct port_info_s;

char *pktgen_ether_hdr_ctor(struct port_info_s *info,
				   pkt_seq_t *pkt,
				   struct ether_hdr *eth);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_ETHER_H_ */
