/*
 * SOURCE: commands.c
 * STUB: pktgen-cmds.h pktgen-main.h pktgen-capture.h lpktgenlib.h
 * STUB: pktgen-random.h rte_debug.h rte_cycles.h rte_timer.h cmdline.h
 * STUB: wr_copyright_info.h rte_pci.h wr_pcap.h cmdline_socket.h
 * STUB: cmdline_rdline.h pktgen-log.h pktgen-display.h
 *
 * LIBS: libwr_lua
 * SYSLIBS: m pthread
 */

#include "pktgen.h"
#include "lpktgenlib.h"


#include <wr_scrn.h>
wr_scrn_t *scrn = NULL;


pktgen_t pktgen;
int rte_cycles_vmware_tsc_map = 0;
enum timer_source eal_timer_source = EAL_TIMER_HPET;


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
