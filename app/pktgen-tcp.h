/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_TCP_H_
#define _PKTGEN_TCP_H_

#include <pg_inet.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 *
 * pktgen_tcp_hdr_ctor - TCP header constructor routine.
 *
 * DESCRIPTION
 * Construct a TCP header in the packet buffer provided.
 *
 * RETURNS: Next header location
 *
 * SEE ALSO:
 */

void* pktgen_tcp_hdr_ctor(pkt_seq_t * pkt, void *hdr, int type);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_TCP_H_ */
