/*-
 * Copyright(c) <2016-2024>, Intel Corporation. All rights reserved.
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

#define DEFAULT_JITTER_THRESHOLD (50)      /**< usec */
#define DEFAULT_LATENCY_RATE     (10000)   /**< micro-seconds*/
#define MAX_LATENCY_RATE         (1000000) /**< micro-seconds */
#define DEFAULT_LATENCY_ENTROPY  (0)       /**< default value to use in (SPORT + (i % N))  */
#define LATENCY_PKT_SIZE         RTE_ETHER_MIN_LEN /**< Packet size 64 + 4 FCS = 68 bytes */
#define LATENCY_DPORT            1028              /**< Reserved */

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
void pktgen_latency_setup(port_info_t *pinfo);

/**
 * latency_set_rate - Set the latency rate for a given port.
 */
void latency_set_rate(port_info_t *pinfo, uint32_t value);

/**
 * latency_set_entropy - Set the entropy value for a given port.
 *
 * value - The entropy value can be 0 >= entropy <= 0xFFFF default to 0.
 */
void latency_set_entropy(port_info_t *pinfo, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_LATENCY_H_ */
