/*
 * SOURCE: commands.c
 * STUB: pktgen-cmds.h pktgen-main.h pktgen-capture.h lpktgenlib.h
 * STUB: pktgen-random.h rte_debug.h rte_cycles.h rte_timer.h cmdline.h
 * STUB: copyright_info.h rte_pci.h pcap.h cmdline_socket.h
 * STUB: cmdline_rdline.h pktgen-log.h pktgen-display.h
 *
 * LIBS: libpg_lua
 * SYSLIBS: m pthread
 */

#include "pktgen.h"
#include "lpktgenlib.h"

#include <scrn.h>
scrn_t *scrn = NULL;

pktgen_t pktgen;
int rte_cycles_vmware_tsc_map = 0;
enum timer_source eal_timer_source = EAL_TIMER_HPET;

/* Test driver */
int
main(void) {
	plan(1);
	ok(1, "ok works");

	done_testing();
	return 0;
}
