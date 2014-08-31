/*
 * SOURCE: pktgen-main.c
 * STUB: pktgen.h rte_cycles.h rte_debug.h rte_eal.h rte_timer.h cmdline.h
 * STUB: wr_l2p.h wr_pcap.h pktgen-display.h wr_copyright_info.h
 * STUB: wr_port_config.h rte_pci.h lua-socket.h pktgen-port-cfg.h
 * STUB: rte_launch.h pktgen-cmds.h commands.h pktgen-log.h
 */


#include <wr_scrn.h>
wr_scrn_t *scrn = NULL;
/* wr_scrn.h function stub */
void wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...) { return; }


#include "pktgen.h"

pktgen_t pktgen;
int rte_cycles_vmware_tsc_map;
enum timer_source eal_timer_source;
__thread unsigned per_lcore__lcore_id;
struct rte_logs rte_logs;


/* Stub functions for rte_ethdev.h. The genstub script has difficulties parsing
 * that header file correctly.
 */
int rte_igb_pmd_init(void) { return 0; }
int rte_igbvf_pmd_init(void) { return 0; }
int rte_em_pmd_init(void) { return 0; }
int rte_ixgbe_pmd_init(void) { return 0; }
int rte_ixgbevf_pmd_init(void) { return 0; }
int rte_virtio_pmd_init(void) { return 0; }
int rte_vmxnet3_pmd_init(void) { return 0; }

/* Stub functions for rte_log.h. Functions declared with __attribute__(())'s
 * are not parsed correctly.
 */
int rte_log (uint32_t level, uint32_t logtype, const char *format, ...) { return 0; }

/* Stub functions for rte_errno.h. The genstub script has difficulties parsing
 * that header file correctly.
 */
const char *rte_strerror(int errnum) { return NULL; }


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
