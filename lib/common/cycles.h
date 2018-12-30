/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CYCLES_H_
#define _CYCLES_H_

#ifdef __cplusplus
extern "C" {
#endif

static __inline__ void
_delay_us(uint32_t us) {
	uint64_t start;
	uint64_t ticks;
	uint32_t resolution_fs;

	resolution_fs =
		(uint32_t)((1000ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL) /
			   rte_get_timer_hz());

	ticks = (uint64_t)us * 1000ULL * 1000ULL * 1000ULL;
	ticks /= resolution_fs;

	start = rte_get_timer_cycles();

	while ((rte_get_timer_cycles() - start) < ticks) {
		rte_timer_manage();
		rte_pause();
	}
}

static __inline__ void
_delay_ms(uint32_t ms) {
	_delay_us(ms * 1000);
}

static __inline__ void
_sleep(uint32_t secs) {
	_delay_us(secs * (1000 * 1000));
}

#ifdef __cplusplus
}
#endif

#endif /* _CYCLES_H_ */
