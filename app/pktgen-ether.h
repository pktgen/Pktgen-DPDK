/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_ETHER_H_
#define _PKTGEN_ETHER_H_

/**
 * @file
 *
 * Ethernet header construction for Pktgen transmit packets.
 */

#include <rte_ethdev.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

struct port_info_s;

/**
 * Construct the Ethernet header for a packet template.
 *
 * Fills in the destination and source MAC addresses and EtherType based on
 * the port configuration and packet sequence entry @p pkt.
 *
 * @param info
 *   Port information providing source MAC and port-level configuration.
 * @param pkt
 *   Packet sequence entry whose Ethernet header fields are populated.
 * @return
 *   Pointer to the byte immediately following the Ethernet header within
 *   the packet template buffer.
 */
char *pktgen_ether_hdr_ctor(struct port_info_s *info, pkt_seq_t *pkt);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_ETHER_H_ */
