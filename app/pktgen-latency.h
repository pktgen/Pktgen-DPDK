/*-
 * Copyright(c) <2016-2023>, Intel Corporation. All rights reserved.
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

#define DEFAULT_JITTER_THRESHOLD (50) /**< usec */
#define DEFAULT_LATENCY_RATE     (10) /**< milliseconds (min value 1)*/
#define LATENCY_PKT_SIZE         72   /**< Packet size */
#define LATENCY_DPORT            1028 /**< Reserved */

void pktgen_page_latency(void);

/**
 *
 * pktgen_latency_setup - Setup the default values for a latency port.
 *
 * DESCRIPTION
 * Setup the default latency data for a given port.
 *
 * RETURNS: N/A
 */
void pktgen_latency_setup(port_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_LATENCY_H_ */
