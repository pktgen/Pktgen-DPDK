/*
 * SOURCE: pktgen-ipv4.c
 * STUB: pktgen.h pktgen-log.h
 *
 * LIBS: libwr_common libwr_scrn
 */

#include "pktgen.h"

pktgen_t pktgen;

__thread unsigned per_lcore__lcore_id;


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
