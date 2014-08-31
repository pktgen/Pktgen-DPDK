/*
 * SOURCE: pktgen-pcap.c
 * STUB: pktgen.h rte_lcore.h wr_copyright_info.h wr_pcap.h
 * STUB: rte_mbuf.h rte_mempool.h rte_debug.h pktgen-display.h pktgen-log.h
 */


#include "pktgen.h"

/* wr_scrn.h function stub */
void wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...) { return; }
void wr_scrn_fprintf(int16_t r, int16_t c, FILE * f, const char * fmt, ...) { return; }
void wr_scrn_center(int16_t r, const char * fmt, ...) { return; }


pktgen_t pktgen;


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
