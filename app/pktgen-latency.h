/*-
 * Copyright (c) <2016-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2016 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_LATENCY_H_
#define _PKTGEN_LATENCY_H_

#include <rte_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint64_t timestamp;
	uint16_t magic;
} latency_t;

#define LATENCY_MAGIC   (('L' << 8) + 'y')
#define DEFAULT_JITTER_THRESHOLD    (50)	/**< usec */

void pktgen_page_latency(void);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_LATENCY_H_ */
