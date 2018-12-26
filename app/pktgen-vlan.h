/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_VLAN_H_
#define _PKTGEN_VLAN_H_

#include <stdint.h>

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

void pktgen_process_vlan(struct rte_mbuf *m, uint32_t pid);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_VLAN_H_ */
