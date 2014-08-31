/*
 * SOURCE: pktgen-port-cfg.c
 * STUB: rte_lcore.h rte_eal.h rte_mbuf.h rte_mempool.h rte_debug.h
 * STUB: wr_l2p.h rte_malloc.h pktgen-pcap.h pktgen-stats.h pktgen-cmds.h
 * STUB: pktgen-range.h pktgen-random.h pktgen-capture.h pktgen-log.h
 */


#include "pktgen.h"

/* Stub functions for rte_errno.h. The genstub script has difficulties parsing
 * that header file correctly.
 */
const char *rte_strerror(int errnum) { return NULL; }

/* Stub functions for rte_ethdev.h.
 */
uint8_t rte_eth_dev_count(void) { return 0; }
int rte_eth_dev_configure(uint8_t port_id, uint16_t nb_rx_queue, uint16_t nb_tx_queue, const struct rte_eth_conf *eth_conf) { return 0; }
void rte_eth_macaddr_get(uint8_t port_id, struct ether_addr *mac_addr) { return; }
int rte_eth_rx_queue_setup(uint8_t port_id, uint16_t rx_queue_id, uint16_t nb_rx_desc, unsigned int socket_id, const struct rte_eth_rxconf *rx_conf, struct rte_mempool *mb_pool) { return 0; }
int rte_eth_tx_queue_setup(uint8_t port_id, uint16_t tx_queue_id, uint16_t nb_tx_desc, unsigned int socket_id, const struct rte_eth_txconf *tx_conf) { return 0; }
int rte_eth_dev_start(uint8_t port_id) { return 0; }
void rte_eth_promiscuous_enable(uint8_t port_id) { return; }


pktgen_t pktgen;


// Test driver
int main(void) {
    plan(1);
    ok(1, "ok works");

    done_testing();
    return 0;
}
