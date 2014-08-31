/*
 * SOURCE: pktgen-cpu.c
 * STUB: wr_copyright_info.h wr_coremap.h wr_lscpu.h wr_l2p.h
 * STUB: pktgen-display.h pktgen-log.h
 *
 * xLIBS: libwr_common libwr_scrn librte_eal librte_mempool librte_malloc
 * xLIBS: librte_pmd_ring librte_ring libethdev
 * xSYSLIBS: pthread
 */

#include "pktgen.h"


/* wr_scrn.h function stub */
void wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...) { return; }


pktgen_t pktgen;


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
