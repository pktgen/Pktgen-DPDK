/*-
 * Copyright(c) <2015-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2015 by abhinandan.gujjar@intel.com */

#ifndef _PKTGEN_GTPU_H_
#define _PKTGEN_GTPU_H_

/**
 * @file
 *
 * GTP-U header construction for Pktgen transmit packets.
 */

#include <pg_inet.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Construct a GTP-U header in the packet buffer.
 *
 * @param pkt
 *   Packet sequence entry providing TEID and payload length.
 * @param hdr
 *   Pointer to the start of the GTP-U header region in the packet buffer.
 * @param ipProto
 *   Inner IP protocol type carried by the GTP-U tunnel.
 * @param flags
 *   GTP-U flags byte (PT, E, S, PN bits).
 * @param seq_no
 *   GTP-U sequence number (used when the S flag is set).
 * @param npdu_no
 *   N-PDU number (used when the PN flag is set).
 * @param next_ext_hdr_type
 *   Next extension header type (used when the E flag is set).
 */
void pktgen_gtpu_hdr_ctor(pkt_seq_t *pkt, void *hdr, uint16_t ipProto, uint8_t flags,
                          uint16_t seq_no, uint8_t npdu_no, uint8_t next_ext_hdr_type);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_GTPU_H_ */
