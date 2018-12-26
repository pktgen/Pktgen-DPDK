/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <rte_version.h>
#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_pci.h>
#include <rte_debug.h>

#include "core_info.h"
#include "lscpu.h"
#include "utils.h"
#include "coremap.h"

#define COREMASK_STRING_SIZE    256

static lc_info_t core_map[RTE_MAX_LCORE];
static uint32_t num_cores;

uint32_t
sct_convert(char *sct[])
{
	uint32_t lcore = 0xFFFF, i;
	lc_info_t val, tst;

	if ( (sct == NULL) || (sct[0] == NULL) )
		return 0xFFFF;

	val.s.socket_id = (uint8_t)strtoul(sct[0], NULL, 10);
	val.s.core_id   = (uint8_t)strtoul(sct[1], NULL, 10);
	val.s.thread_id = (uint8_t)strtoul(sct[2], NULL, 10);

	for (i = 0; i < num_cores; i++) {
		tst.word = core_map[i].word;
		tst.s.id = 0;
		if (tst.word == val.word) {
			lcore = core_map[i].s.id;
			break;
		}
	}
	return lcore;
}
