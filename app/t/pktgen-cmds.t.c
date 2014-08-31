/*
 * SOURCE: pktgen-cmds.c
 * STUB: pktgen.h pktgen-range.h pktgen-seq.h rte_eal.h wr_copyright_info.h
 * STUB: rte_mempool.h rte_malloc.h cmdline.h pktgen-log.h pktgen-display.h
 *
 * SYSLIBS: pcap
 */

#include "pktgen.h"

#include <wr_scrn.h>
wr_scrn_t *scrn;

pktgen_t pktgen;

/* Stubs for rte_ethdev.h. The stub generator doesn't handle #ifdef guarded
 * function declarations like those used in rte_ethdev.h yet. */
int rte_eth_led_on(uint8_t port_id) { return 0; }
void rte_eth_stats_get(uint8_t port_id, struct rte_eth_stats *stats) { return; }


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
