/*
 * SOURCE: pktgen-cpu.c
 * STUB: copyright_info.h coremap.h lscpu.h l2p.h
 * STUB: pktgen-display.h pktgen-log.h
 *
 * xLIBS: libpg_common libpg_scrn librte_eal librte_mempool librte_malloc
 * xLIBS: librte_pmd_ring librte_ring libethdev
 * xSYSLIBS: pthread
 */

#include "pktgen.h"

/* scrn.h function stub */
void
scrn_printf(int16_t r, int16_t c, const char *fmt, ...) {         }

pktgen_t pktgen;

/* Test driver */
int
main(void) {
	plan(1);
	ok(1, "ok works");

	done_testing();
	return 0;
}
