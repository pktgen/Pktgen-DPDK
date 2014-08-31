/*-
 * Copyright (c) <2010-2014>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2014 by Keith Wiles @ windriver.com */

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

#include "wr_scrn.h"
#include "wr_core_info.h"
#include "wr_lscpu.h"
#include "wr_utils.h"
#include "wr_coremap.h"

#define COREMASK_STRING_SIZE	256

static lc_info_t	core_map[RTE_MAX_LCORE];
static uint32_t		num_cores;

uint32_t
wr_sct_convert( char * sct[] )
{
	uint32_t	lcore = 0xFFFF, i;
	lc_info_t	val, tst;

	if ( (sct == NULL) || (sct[0] == NULL) )
		return 0xFFFF;

	val.s.socket_id	= (uint8_t)strtoul(sct[0], NULL, 10);
	val.s.core_id	= (uint8_t)strtoul(sct[1], NULL, 10);
	val.s.thread_id	= (uint8_t)strtoul(sct[2], NULL, 10);

	for(i = 0; i < num_cores; i++) {
		tst.word = core_map[i].word;
		tst.s.id = 0;
		if ( tst.word == val.word ) {
			lcore = core_map[i].s.id;
			break;
		}
	}
	return lcore;
}

uint64_t
wr_parse_coremask( const char * coremask )
{
	uint64_t	cm;
	uint32_t	lcore, n, i;
	char      * sct[4];
	char      * arr[RTE_MAX_LCORE], * p;
	char		buff[COREMASK_STRING_SIZE], * str;

	memset(arr, 0, sizeof(arr));
	memset(sct, 0, sizeof(sct));

	// Get a private copy so we can modify the string.
	strncpy(buff, &coremask[1], sizeof(buff)-1);
	str = buff;

	num_cores = wr_coremap("array", core_map, RTE_MAX_LCORE, NULL);
	if ( num_cores == 0xFFFF )
		return 0;

	n = wr_strparse(str, ",", arr, RTE_MAX_LCORE);

	for(i = 0, cm = 0; i < n; i++) {
		if ( (p = arr[i]) == NULL )
			return 0;

		if ( strchr(p, '/') ) {			// Process the S/C/T format */

			if ( wr_strparse(p, "/", sct, 4) != 3 )
				return 0;

			// Handle converting the S/C/T to a lcore number.
			lcore = wr_sct_convert(sct);
			if ( lcore == 0xFFFF )
				return 0;

			cm |= (1 << lcore);
		} else if ( *p == '[' ) {		// We have a bit range
			uint32_t	hi, lo;

			// decode the [lo-hi] format string.
			lo = strtoul(++p, NULL, 10);
			p = strchr(p, '-');
			hi = (p != NULL) ? strtoul(++p, NULL, 10) : lo;

			while( lo <= hi )
				cm |= (1ULL << lo++);
		} else {						// We have a lcore number
			lcore = strtoul(p, NULL, 10);
			cm |= (1ULL << lcore);
		}
	}

	return cm;
}
