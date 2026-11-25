/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_GRE_H_
#define _PKTGEN_GRE_H_

/**
 * @file
 *
 * GRE tunnel header construction for Pktgen transmit packets.
 */

#include <pg_inet.h>

#include "pktgen-port-cfg.h"
#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Construct a GRE-over-IP header for a packet template.
 *
 * @param pinfo
 *   Port information providing source IP and port-level configuration.
 * @param pkt
 *   Packet sequence entry whose header fields are populated.
 * @param gre
 *   Pointer to the GRE-over-IP header region in the packet buffer.
 * @return
 *   Pointer to the byte immediately following the completed GRE header.
 */
char *pktgen_gre_hdr_ctor(port_info_t *pinfo, pkt_seq_t *pkt, greIp_t *gre);

/**
 * Construct a GRE-over-Ethernet header for a packet template.
 *
 * @param pinfo
 *   Port information providing source MAC and port-level configuration.
 * @param pkt
 *   Packet sequence entry whose header fields are populated.
 * @param gre
 *   Pointer to the GRE-over-Ethernet header region in the packet buffer.
 * @return
 *   Pointer to the byte immediately following the completed GRE header.
 */
char *pktgen_gre_ether_hdr_ctor(port_info_t *pinfo, pkt_seq_t *pkt, greEther_t *gre);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_GRE_H_ */
