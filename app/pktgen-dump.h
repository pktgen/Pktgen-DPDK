/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_DUMP_H_
#define _PKTGEN_DUMP_H_

/**
 * @file
 *
 * Packet hex-dump capture and printing utilities for Pktgen debugging.
 */

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of packets that can be queued for dump output. */
#define MAX_DUMP_PACKETS 32

/**
 * Capture a single packet for later hex-dump output.
 *
 * @param m
 *   Mbuf containing the packet to capture.
 * @param pid
 *   Port index associated with the packet.
 */
void pktgen_packet_dump(struct rte_mbuf *m, int pid);

/**
 * Capture up to @p nb_dump packets from an mbuf array for later hex-dump output.
 *
 * @param pkts
 *   Array of mbufs to capture.
 * @param nb_dump
 *   Number of mbufs in @p pkts to capture (capped at MAX_DUMP_PACKETS).
 * @param pid
 *   Port index associated with the packets.
 */
void pktgen_packet_dump_bulk(struct rte_mbuf **pkts, int nb_dump, int pid);

/**
 * Print all captured packet hex-dumps to the console and clear the dump queue.
 */
void pktgen_print_packet_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_DUMP_H_ */
