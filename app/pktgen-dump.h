/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_DUMP_H_
#define _PKTGEN_DUMP_H_

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DUMP_PACKETS        32

void pktgen_packet_dump(struct rte_mbuf *m, int pid);
void pktgen_packet_dump_bulk(struct rte_mbuf **pkts, int nb_dump,
				    int pid);

void pktgen_print_packet_dump(void);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_DUMP_H_ */
