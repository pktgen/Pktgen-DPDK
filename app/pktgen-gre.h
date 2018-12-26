/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_GRE_H_
#define _PKTGEN_GRE_H_

#include <pg_inet.h>

#include "pktgen-port-cfg.h"
#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

char *pktgen_gre_hdr_ctor(port_info_t *info, pkt_seq_t *pkt,
				 greIp_t *gre);
char *pktgen_gre_ether_hdr_ctor(port_info_t *info,
				       pkt_seq_t *pkt,
				       greEther_t *gre);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_GRE_H_ */
