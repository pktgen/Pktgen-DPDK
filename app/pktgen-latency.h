/*-
 * Copyright(c) <2016-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2016 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_LATENCY_H_
#define _PKTGEN_LATENCY_H_

/**
 * @file
 *
 * One-way latency measurement support for Pktgen.
 *
 * Implements timestamped probe-packet injection, latency histogram
 * collection, jitter tracking, and the latency statistics display page.
 */

#include <rte_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_JITTER_THRESHOLD (50)    /**< Default jitter threshold in microseconds */
#define DEFAULT_LATENCY_RATE     (10000) /**< Default probe injection rate in microseconds */
#define MAX_LATENCY_RATE         (1000000) /**< Maximum allowed probe injection rate in microseconds */
#define DEFAULT_LATENCY_ENTROPY  (0) /**< Default entropy seed for source port randomisation */
#define LATENCY_PKT_SIZE         RTE_ETHER_MIN_LEN /**< Latency probe packet size (64 B + 4 B FCS) */
#define LATENCY_DPORT            1028 /**< Destination UDP port used for latency probes */

/**
 * Render the latency statistics display page to the console.
 */
void pktgen_page_latency(void);

/**
 * Initialise latency measurement state for port @p pinfo to default values.
 *
 * @param pinfo
 *   Port information structure to initialise.
 */
void pktgen_latency_setup(port_info_t *pinfo);

/**
 * Set the latency probe injection rate for port @p pinfo.
 *
 * @param pinfo
 *   Port information structure to update.
 * @param value
 *   Probe injection interval in microseconds (1–MAX_LATENCY_RATE).
 */
void latency_set_rate(port_info_t *pinfo, uint32_t value);

/**
 * Set the source-port entropy value used for latency probe identification.
 *
 * The entropy value controls how many distinct source ports are cycled
 * (SPORT + (i % entropy)); a value of 0 disables cycling.
 *
 * @param pinfo
 *   Port information structure to update.
 * @param value
 *   Entropy range in [0, 0xFFFF].
 */
void latency_set_entropy(port_info_t *pinfo, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_LATENCY_H_ */
