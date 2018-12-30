/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_UDP_H_
#define _PKTGEN_UDP_H_

#include <pg_inet.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VXLAN_PORT_ID	4789

/**************************************************************************//**
 *
 * pktgen_udp_hdr_ctor - UDP header constructor routine.
 *
 * DESCRIPTION
 * Construct the UDP header in a packer buffer.
 *
 * RETURNS: Next header location
 *
 * SEE ALSO:
 */

void *pktgen_udp_hdr_ctor(pkt_seq_t *pkt, void *hdr, int type);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_UDP_H_ */
