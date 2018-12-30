/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <rte_lua.h>

#include "pktgen-display.h"
#include "pktgen-cpu.h"
#include "pktgen-log.h"

#include "pktgen.h"

static int
save_uname(char *line, int i __rte_unused) {
	pktgen.uname = pg_strdupf(pktgen.uname, line);
	return 0;
}

static void
pktgen_get_uname(void)
{
	do_command("uname -a", save_uname);
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
	uint32_t i, row, cnt, nb_sockets, nb_cores, nb_threads;
	static int counter = 0;
	char buff[1024];

	display_topline("<CPU Page>");

	if ( (pktgen.core_cnt == 0) || (pktgen.lscpu == NULL) )
		pktgen_cpu_init();

	cnt         = pktgen.core_cnt;
	nb_sockets  = coremap_cnt(pktgen.core_info, cnt, 0);
	nb_cores    = coremap_cnt(pktgen.core_info, cnt, 1);
	nb_threads  = coremap_cnt(pktgen.core_info, cnt, 2);

	if ( (counter++ & 3) != 0)
		return;

	pktgen_display_set_color("stats.stat.label");
	row = 3;
	scrn_printf(row++, 1, "Kernel: %s", pktgen.uname);
	row++;
	pktgen_display_set_color("stats.stat.values");
	scrn_printf(row++, 1, "Model Name: %s", pktgen.lscpu->model_name);
	scrn_printf(row++, 1, "CPU Speed : %s", pktgen.lscpu->cpu_mhz);
	scrn_printf(row++, 1, "Cache Size: %s", pktgen.lscpu->cache_size);
	row++;
	pktgen_display_set_color("top.ports");
	scrn_printf(row++, 1, "CPU Flags : %s", pktgen.lscpu->cpu_flags);
	row += 6;

	pktgen_display_set_color("stats.total.data");
	scrn_printf(row++, 5,
				"%d sockets, %d cores per socket and %d threads per core.",
	        nb_sockets,
	        nb_cores,
	        nb_threads);

	pktgen_display_set_color("stats.dyn.label");
	sprintf(buff, "Socket   : ");
	for (i = 0; i < nb_sockets; i++)
		strncatf(buff, "%4d      ", i);
	scrn_printf(row++, 3, "%s", buff);

	pktgen_display_set_color("stats.stat.label");
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
	scrn_printf(row++, 1, "%s", buff);
	row += i;

	pktgen_display_set_color("stats.total.data");
	pg_port_matrix_dump(pktgen.l2p);

	if (pktgen.flags & PRINT_LABELS_FLAG) {
		pktgen.last_row = row + 6 + pktgen.nb_ports;
		display_dashline(pktgen.last_row);

		scrn_setw(pktgen.last_row);
		scrn_printf(100, 1, "");	/* Put cursor on the last row. */
	}
	pktgen_display_set_color(NULL);
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
	pktgen.core_cnt     = coremap("array",
	                                 pktgen.core_info,
	                                 RTE_MAX_LCORE,
	                                 NULL);
	pktgen.lscpu        = lscpu_info(NULL, NULL);
}
