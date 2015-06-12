/*-
 * Copyright (c) <2010>, Intel Corporation
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
/* Created 2010 by Keith Wiles @ intel.com */

#include "pktgen-display.h"
#include "pktgen-cpu.h"
#include "pktgen-log.h"

#include "pktgen.h"

static int
save_uname(char * line, __attribute__ ((unused))int i) {
	pktgen.uname = wr_strdupf(pktgen.uname, line);
	return 0;
}

static void
pktgen_get_uname(void)
{
	do_command("uname -a", save_uname);
}

static __inline__ uint8_t
sct( uint8_t s, uint8_t c, uint8_t t) {
	lc_info_t	* lc = &pktgen.core_info[0];
	uint8_t		i;

	for(i=0; i<pktgen.core_cnt; i++, lc++) {
		if ( lc->s.socket_id == s && lc->s.core_id == c && lc->s.thread_id == t )
			return lc->s.id;
	}

	return 0;
}

/**************************************************************************//**
*
* pktgen_page_cpu - Display the CPU data page.
*
* DESCRIPTION
* Display the CPU data page for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_page_cpu(void)
{
    uint32_t    i, row, cnt, nb_sockets, nb_cores, nb_threads;
	static int counter = 0;
    char buff[1024];

    display_topline("<CPU Page>");

	if ( (pktgen.core_cnt == 0) || (pktgen.lscpu == NULL) )
		pktgen_cpu_init();

	cnt			= pktgen.core_cnt;
    nb_sockets	= wr_coremap_cnt(pktgen.core_info, cnt, 0);
    nb_cores	= wr_coremap_cnt(pktgen.core_info, cnt, 1);
    nb_threads	= wr_coremap_cnt(pktgen.core_info, cnt, 2);

    if ( (counter++ & 3) != 0 )
    	return;

    row = 3;
    wr_scrn_printf(row++, 1, "Kernel: %s", pktgen.uname);
    row++;
    wr_scrn_printf(row++, 1, "Model Name: %s", pktgen.lscpu->model_name);
    wr_scrn_printf(row++, 1, "CPU Speed : %s", pktgen.lscpu->cpu_mhz);
    wr_scrn_printf(row++, 1, "Cache Size: %s", pktgen.lscpu->cache_size);
    row++;
    wr_scrn_printf(row++, 1, "CPU Flags : %s", pktgen.lscpu->cpu_flags);
    row += 4;

    wr_scrn_printf(row++, 5, "%d sockets, %d cores per socket and %d threads per core.",
    	nb_sockets, nb_cores, nb_threads);

    sprintf(buff, "Socket   : ");
    for(i = 0; i< nb_sockets; i++)
        strncatf(buff, "%4d      ", i);
    wr_scrn_printf(row++, 3, "%s", buff);

    buff[0] = '\0';
	for(i = 0; i< nb_cores; i++) {
        strncatf(buff, "  Core %3d : [%2d,%2d]   ", i, sct(0, i, 0),   sct(0, i, 1));
		if ( nb_sockets > 1 )
			strncatf(buff, "[%2d,%2d]   ", sct(1, i, 0), sct(1, i, 1));
		if ( nb_sockets > 2 )
			strncatf(buff, "[%2d,%2d]   ", sct(2, i, 0), sct(2, i, 1));
		if ( nb_sockets > 3 )
			strncatf(buff, "[%2d,%2d]   ", sct(3, i, 0), sct(3, i, 1));
		strncatf(buff, "\n");
	}
	wr_scrn_printf(row++, 1, "%s", buff);

	wr_port_matrix_dump(pktgen.l2p);

	if ( pktgen.flags & PRINT_LABELS_FLAG ) {

		pktgen.last_row = 36;
	    display_dashline(pktgen.last_row);

		wr_scrn_setw(pktgen.last_row);
		wr_scrn_printf(100, 1, "");        // Put cursor on the last row.
	}
    pktgen.flags &= ~PRINT_LABELS_FLAG;
}


/**************************************************************************//**
*
* pktgen_cpu_init - Init the CPU information
*
* DESCRIPTION
* initialize the CPU information
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_cpu_init(void)
{
    pktgen_get_uname();
    memset(&pktgen.core_info, 0xff, (sizeof(lc_info_t) * RTE_MAX_LCORE));
    pktgen.core_cnt		= wr_coremap("array", pktgen.core_info, RTE_MAX_LCORE, NULL);
    pktgen.lscpu		= wr_lscpu_info(NULL, NULL);
}
