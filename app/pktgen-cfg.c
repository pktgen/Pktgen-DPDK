/*-
 * Copyright (c) <2017>, Intel Corporation
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

/* Created 2017 by Keith Wiles @ intel.com */
#include "pktgen-display.h"
#include "pktgen-cpu.h"
#include "pktgen-cfg.h"

/*
 * 2 sockets, 18 cores, 2 threads
 * Socket   :    0         1         2         3
 * Core   0 : [ 0,36]   [18,54]   [18,54]   [18,54]
 */
/**************************************************************************//**
 *
 * pktgen_page_config - Show the configuration page for pktgen.
 *
 * DESCRIPTION
 * Display the pktgen configuration page. (Not used)
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_page_config(void)
{
	uint32_t i, row, col, cnt, nb_sockets, nb_cores, nb_threads;
	static int counter = 0;
	char buff[2048];

	display_topline("<CPU Page>");

	if ( (pktgen.core_cnt == 0) || (pktgen.lscpu == NULL) )
		pktgen_cpu_init();

	cnt         = pktgen.core_cnt;
	nb_sockets  = coremap_cnt(pktgen.core_info, cnt, 0);
	nb_cores    = coremap_cnt(pktgen.core_info, cnt, 1);
	nb_threads  = coremap_cnt(pktgen.core_info, cnt, 2);

	if ( (counter++ & 3) != 0)
		return;

	row = 2;
	col = 1;
	scrn_printf(
		row++,
		2,
		"%d sockets, %d cores, %d threads",
		nb_sockets,
		nb_cores,
		nb_threads);

	sprintf(buff, "Socket   : ");
	for (i = 0; i < nb_sockets; i++)
		strncatf(buff, "%4d      ", i);
	scrn_printf(row, col + 2, "%s", buff);
	scrn_printf(0, 0, "Port description");

	row++;
	buff[0] = '\0';
	for (i = 0; i < nb_cores; i++) {
		strncatf(buff, "  Core %3d : [%2d,%2d]   ",
			 i, sct(0, i, 0), sct(0, i, 1));
		if (nb_sockets > 1)
			strncatf(buff, "[%2d,%2d]   ",
				 sct(1, i, 0), sct(1, i, 1));
		if (nb_sockets > 2)
			strncatf(buff, "[%2d,%2d]   ",
				 sct(2, i, 0), sct(2, i, 1));
		if (nb_sockets > 3)
			strncatf(buff, "[%2d,%2d]   ",
				 sct(3, i, 0), sct(3, i, 1));
		strncatf(buff, "\n");
	}
	scrn_printf(row, 1, "%s", buff);

	col = 13 + (nb_sockets * 10) + 1;
	for (i = 0; i < pktgen.portdesc_cnt; i++)
		scrn_printf(row + i, col, "%s", pktgen.portdesc[i]);

	row += RTE_MAX(nb_cores, pktgen.portdesc_cnt) + 2;
	if (pktgen.flags & PRINT_LABELS_FLAG) {
		pktgen.last_row = row;
		display_dashline(pktgen.last_row);

		scrn_setw(pktgen.last_row);
		scrn_printf(100, 1, "");/* Put cursor on the last row. */
	}
	pktgen.flags &= ~PRINT_LABELS_FLAG;
}
