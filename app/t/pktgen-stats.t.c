/*
 * SOURCE: pktgen-stats.c
 * STUB: rte_cycles.h copyright_info.h pktgen-cmds.h
 * STUB: pktgen-display.h
 */

/* scrn.h function stub */
void
scrn_printf(int16_t r, int16_t c, const char *fmt, ...) {         }
void
scrn_snprintf(char *buff, int16_t len, const char *fmt, ...) {         }

/*
 * rte_ethdev.h fake functions.
 */
struct rte_eth_link;
struct rte_eth_stats;

void
rte_eth_link_get_nowait(uint8_t port_id, struct rte_eth_link *eth_link) {
}

int
rte_eth_led_on(uint8_t port_id) { return 0; }
int
rte_eth_led_off(uint8_t port_id) { return 0; }
void
rte_eth_stats_get(uint8_t port_id, struct rte_eth_stats *stats) {         }

#include "pktgen.h"

pktgen_t pktgen;

/* Test driver */
int
main(void) {
	plan(1);
	ok(1, "ok works");

	done_testing();
	return 0;
}
