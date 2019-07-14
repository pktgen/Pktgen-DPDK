/*-
 * Copyright (c) <2016-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2019 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_RATE_H_
#define _PKTGEN_RATE_H_

#include <rte_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint64_t timestamp;
	uint16_t magic;
} rate_stamp_t;

typedef struct {
	uint16_t	fps;
	uint16_t	frame_size;
	uint16_t	color_bits;
	uint16_t	payload_size;
	uint16_t	overhead;
	uint16_t	pad0;
	uint32_t	mbps;
	uint32_t	pps;
	double		fps_rate;
	uint64_t	next_tsc;
} rate_info_t;

#define RATE_MAGIC   (('R' << 8) + 'y')

extern rate_info_t rates[RTE_MAX_ETHPORTS];

void pktgen_rate_init(void);
void pktgen_page_rate(void);
void rate_set_value(port_info_t *info, const char *what, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_RATE_H_ */
