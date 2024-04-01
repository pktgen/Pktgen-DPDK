/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <lua_config.h>

#include "pktgen-display.h"
#include "pktgen-cpu.h"
#include "pktgen-log.h"

#include "pktgen.h"

static char *uname_str;

static int
save_uname(char *line, int i __rte_unused)
{
    uname_str = pg_strdupf(uname_str, line);
    return 0;
}

/**
 *
 * pktgen_page_cpu - Display the CPU page.
 *
 * DESCRIPTION
 * Display the CPU page.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_page_cpu(void)
{
    uint32_t row, cnt, nb_sockets, nb_cores, nb_threads;
    static int counter = 0;

    display_topline("<CPU Page>", 0, 0, 0);

    cnt = coreinfo_lcore_cnt();
    if ((cnt == 0) || (pktgen.lscpu == NULL))
        pktgen_cpu_init();

    nb_sockets = coreinfo_socket_cnt();
    nb_cores   = coreinfo_core_cnt();
    nb_threads = coreinfo_thread_cnt();

    if ((counter++ & 3) != 0)
        return;

    pktgen_display_set_color("stats.stat.label");
    row = 3;
    scrn_printf(row++, 1, "Kernel: %s", uname_str);
    row++;
    pktgen_display_set_color("stats.stat.values");
    scrn_printf(row++, 1, "Model Name: %s", pktgen.lscpu->model_name);
    scrn_printf(row++, 1, "Cache Size: %s", pktgen.lscpu->cache_size);
    row++;
    pktgen_display_set_color("top.ports");
    scrn_printf(row++, 1, "CPU Flags : %s", pktgen.lscpu->cpu_flags);
    row += 6;

    pktgen_display_set_color("stats.total.data");
    scrn_printf(row++, 1, "Number of sockets %d, cores/socket %d, threads/core %d, total %d",
                nb_sockets, nb_cores, nb_threads, cnt);

    if (pktgen.flags & PRINT_LABELS_FLAG) {
        pktgen.last_row = row + pktgen.nb_ports;
        display_dashline(pktgen.last_row);

        scrn_setw(pktgen.last_row);
        scrn_printf(100, 1, ""); /* Put cursor on the last row. */
    }
    pktgen_display_set_color(NULL);
    pktgen.flags &= ~PRINT_LABELS_FLAG;
}

/**
 *
 * pktgen_cfg_init - Init the CPU information
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
    do_command("uname -a", save_uname);
    pktgen.lscpu = lscpu_info(NULL, NULL);
}
