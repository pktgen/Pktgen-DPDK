/*-
 * Copyright (c) <2014-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#ifndef __COREMAP_H
#define __COREMAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROC_CPUINFO "/proc/cpuinfo"

typedef union {
	struct {
		uint8_t id;		/* Logical core ID */
		uint8_t socket_id;	/* CPU socket ID */
		uint8_t core_id;	/* Physical CPU core ID */
		uint8_t thread_id;	/* Hyper-thread ID */
	} s;
	uint32_t word;
} lc_info_t;

int coremap(const char *opt,
		   lc_info_t *get,
		   int cnt,
		   const char *proc_cpuinfo);
unsigned coremap_cnt(const lc_info_t *lc, unsigned max_cnt,
			    unsigned t);

#ifdef __cplusplus
}
#endif

#endif  /*_COREMAP_H */
