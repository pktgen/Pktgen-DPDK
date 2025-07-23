/*-
 * Copyright(c) <2020-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2018 by Keith Wiles @ intel.com */

#include <lua_config.h>

#include "pktgen-display.h"
#include "pktgen-cpu.h"
#include "pktgen-sys.h"

#include "coreinfo.h"

/**
 *
 * pktgen_page_system - Show the system page for pktgen.
 *
 * DESCRIPTION
 * Display the pktgen system page. (Not used)
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_page_system(void)
{
    uint32_t i, row, col, cnt, nb_sockets, nb_cores, nb_threads;
    static int counter = 0;
    char buff[2048];

    pktgen_display_set_color("top.page");
    display_topline("<System Page>", 0, 0, 0);

    cnt = coreinfo_lcore_cnt();
    if ((cnt == 0) || (pktgen.lscpu == NULL))
        pktgen_cpu_init();

    nb_sockets = coreinfo_socket_cnt();
    nb_cores   = coreinfo_core_cnt();
    nb_threads = coreinfo_thread_cnt();

    if ((counter++ & 3) != 0)
        return;

    row = 3;
    col = 1;
    pktgen_display_set_color("stats.total.data");
    scrn_printf(row++, 1, "Number of sockets %d, cores/socket %d, threads/core %d, total %d",
                nb_sockets, nb_cores, nb_threads, cnt);

    pktgen_display_set_color("stats.dyn.label");
    sprintf(buff, "Socket   : ");
    for (i = 0; i < nb_sockets; i++)
        strncatf(buff, "%4d      ", i);
    scrn_printf(row, col + 2, "%s", buff);
    scrn_printf(0, 0, "Port description");

    pktgen_display_set_color("stats.stat.label");
    row++;
    buff[0] = '\0';
    for (i = 0; i < nb_cores; i++) {
        strncatf(buff, "  Core %3d : [%2d,%2d]   ", i, sct(0, i, 0), sct(0, i, 1));
        if (nb_sockets > 1)
            strncatf(buff, "[%2d,%2d]   ", sct(1, i, 0), sct(1, i, 1));
        if (nb_sockets > 2)
            strncatf(buff, "[%2d,%2d]   ", sct(2, i, 0), sct(2, i, 1));
        if (nb_sockets > 3)
            strncatf(buff, "[%2d,%2d]   ", sct(3, i, 0), sct(3, i, 1));
        strncatf(buff, "\n");
    }
    scrn_printf(row, 1, "%s", buff);

    pktgen_display_set_color("stats.bdf");
    col = 13 + (nb_sockets * 10) + 1;
    for (i = 0; i < pktgen.portdesc_cnt; i++)
        scrn_printf(row + i, col, "%s", pktgen.portdesc[i]);

    row += RTE_MAX(nb_cores, pktgen.portdesc_cnt);
    scrn_pos(row, 1);
    pktgen_display_set_color("stats.stat.values");

    row += pktgen.nb_ports + 4;
    if (pktgen.flags & PRINT_LABELS_FLAG) {
        pktgen.last_row = row + pktgen.nb_ports;
        display_dashline(pktgen.last_row);

        scrn_setw(pktgen.last_row);
        scrn_printf(100, 1, ""); /* Put cursor on the last row. */
    }
    pktgen_display_set_color(NULL);
    pktgen.flags &= ~PRINT_LABELS_FLAG;
}
