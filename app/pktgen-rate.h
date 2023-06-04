/*-
 * Copyright(c) <2016-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2020 by Keith Wiles @ intel.com */

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

#define RATE_MAGIC (('R' << 8) + 'y')

void pktgen_rate_init(port_info_t *info);
void pktgen_page_rate(void);
void rate_set_value(port_info_t *info, const char *what, uint32_t value);
void update_rate_values(port_info_t *info);
void pktgen_rate_setup(port_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_RATE_H_ */
