/*-
 * Copyright (c) <2015-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2015 by abhinandan.gujjar@intel.com */

#ifndef _PKTGEN_GTPU_H_
#define _PKTGEN_GTPU_H_

#include <pg_inet.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 *
 * pktgen_gtpu_hdr_ctor - GTP-U header constructor routine.
 *
 * DESCRIPTION
 * Construct the GTP-U header in a packer buffer.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_gtpu_hdr_ctor(pkt_seq_t *pkt, void *hdr, uint16_t ipProto,
		uint8_t flags, uint16_t seq_no, uint8_t npdu_no,
		uint8_t next_ext_hdr_type);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_GTPU_H_ */
