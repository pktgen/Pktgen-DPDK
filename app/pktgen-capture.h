/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CAPTURE_H_
#define _PKTGEN_CAPTURE_H_

#include <stddef.h>
#include <inttypes.h>

#include <rte_memzone.h>
#include <rte_mbuf.h>

#include "pktgen-port-cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cap_hdr_s {
	uint16_t pkt_len;
	uint16_t data_len;
	uint8_t pkt[0];
} cap_hdr_t;

/* packet capture data */
typedef struct capture_s {
	const struct rte_memzone  *mz;	/**< Memory region to store packets */
	cap_hdr_t                 *tail;/**< Current tailt pointer in the pkt buffer */
	cap_hdr_t                 *end;	/**< Points to just before the end[-1] of the buffer */
	size_t used;			/**< Memory used by captured packets */
	uint16_t lcore;			/**< lcore that captures to this memzone */
	uint16_t port;			/**< port for this memzone */
} capture_t;

/* Capture initialization */
void pktgen_packet_capture_init(capture_t *capture, int socket_id);

/* Enable/disable capture for port */
void pktgen_set_capture(port_info_t *info, uint32_t onOff);

/* Perform capture of packets */
void pktgen_packet_capture_bulk(struct rte_mbuf **pkts,
				       uint32_t nb_dump,
				       capture_t *capture);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_CAPTURE_H_ */
