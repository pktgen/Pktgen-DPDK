/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CAPTURE_H_
#define _PKTGEN_CAPTURE_H_

/**
 * @file
 *
 * In-memory packet capture for Pktgen.
 *
 * Provides structures and functions for capturing received packets into a
 * DPDK memzone with per-packet timestamps and length metadata.
 */

#include <stddef.h>
#include <inttypes.h>

#include <rte_memzone.h>
#include <rte_mbuf.h>

#include "pktgen-port-cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Per-packet capture record header, immediately followed by packet data. */
typedef struct cap_hdr_s {
    uint64_t tstamp;   /**< Capture timestamp in TSC cycles */
    uint16_t pkt_len;  /**< Original packet length in bytes */
    uint16_t data_len; /**< Number of packet bytes stored after this header */
    uint8_t pkt[0];    /**< Inline packet data (flexible array) */
} cap_hdr_t;

/** Capture buffer state for one NUMA socket. */
typedef struct capture_s {
    const struct rte_memzone *mz; /**< Memory region to store packets */
    cap_hdr_t *tail;              /**< Current tail pointer in the pkt buffer */
    cap_hdr_t *end;               /**< Points to just before the end[-1] of the buffer */
    size_t used;                  /**< Memory used by captured packets */
    uint32_t nb_pkts;             /**< Number of packets in capture pool */
    uint16_t port;                /**< port for this memzone */
} capture_t;

/**
 * Allocate and initialise the capture memzone for a NUMA socket.
 *
 * @param socket_id
 *   NUMA socket ID on which to allocate the capture buffer.
 */
void pktgen_packet_capture_init(uint16_t socket_id);

/**
 * Enable or disable packet capture on a port.
 *
 * @param pinfo  Per-port state.
 * @param onOff  ENABLE_STATE to start capturing, DISABLE_STATE to stop.
 */
void pktgen_set_capture(port_info_t *pinfo, uint32_t onOff);

/**
 * Capture a burst of received packets into the per-socket capture buffer.
 *
 * @param pkts     Array of mbufs to capture.
 * @param nb_dump  Number of mbufs in @p pkts.
 * @param capture  Capture state for the NUMA socket.
 */
void pktgen_packet_capture_bulk(struct rte_mbuf **pkts, uint32_t nb_dump, capture_t *capture);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CAPTURE_H_ */
