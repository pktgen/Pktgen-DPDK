/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2023 Intel Corporation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <alloca.h>
#include <locale.h>
#include <pthread.h>

#include <pktperf.h>

#include <fgen.h>

txpkts_info_t *info;

static __inline__ void
mbuf_iterate_cb(struct rte_mempool *mp, void *opaque, void *obj, unsigned obj_idx __rte_unused)
{
    l2p_lport_t *lport = (l2p_lport_t *)opaque;
    struct rte_mbuf *m = (struct rte_mbuf *)obj;
    uint16_t plen      = info->pkt_size - RTE_ETHER_CRC_LEN;

    packet_constructor(lport, rte_pktmbuf_mtod(m, uint8_t *));

    m->pool     = mp;
    m->next     = NULL;
    m->data_len = plen;
    m->pkt_len  = plen;
    m->port     = 0;
    m->ol_flags = 0;
}

static __inline__ void
do_rx_process(l2p_lport_t *lport, struct rte_mbuf **mbufs, uint32_t n_mbufs, uint64_t curr_tsc)
{
    l2p_port_t *port = lport->port;
    qstats_t *c;
    uint16_t nb_pkts, rx_qid;

    rx_qid = lport->rx_qid;
    c      = &port->pq[rx_qid].curr;

    /* drain the RX queue */
    nb_pkts = rte_eth_rx_burst(port->pid, rx_qid, mbufs, n_mbufs);
    if (nb_pkts) {
        for (uint16_t i = 0; i < nb_pkts; i++)
            c->q_ibytes[rx_qid] += rte_pktmbuf_pkt_len(mbufs[i]);
        c->q_ipackets[rx_qid] += nb_pkts;

        rte_pktmbuf_free_bulk(mbufs, nb_pkts);
        c->q_rx_time[rx_qid] = rte_rdtsc() - curr_tsc;
    }
}

static __inline__ void
do_tx_process(l2p_lport_t *lport, struct rte_mbuf **mbufs, uint16_t n_mbufs, uint64_t curr_tsc)
{
    l2p_port_t *port = lport->port;
    struct rte_mempool *mp;
    qstats_t *c;
    uint16_t nb_pkts, pid, tx_qid;

    pid    = port->pid;
    tx_qid = lport->tx_qid;
    c      = &port->pq[tx_qid].curr;
    mp     = lport->port->tx_mp;

    /* Use mempool routines instead of pktmbuf to make sure the mbufs is not altered */
    if (rte_mempool_get_bulk(mp, (void **)mbufs, n_mbufs) == 0) {
        uint16_t plen = info->pkt_size - RTE_ETHER_CRC_LEN;

        nb_pkts = rte_eth_tx_burst(pid, tx_qid, mbufs, n_mbufs);
        if (unlikely(nb_pkts != n_mbufs)) {
            uint32_t n = n_mbufs - nb_pkts;

            rte_mempool_put_bulk(mp, (void **)&mbufs[nb_pkts], n);
            c->q_tx_drops[tx_qid] += n;
            return;
        }
        c->q_opackets[tx_qid] += nb_pkts;
        c->q_obytes[tx_qid] += (nb_pkts * plen); /* does not include FCS */

        c->q_tx_time[tx_qid] = rte_rdtsc() - curr_tsc;
    } else
        c->q_no_txmbufs[tx_qid]++;
}

/* main processing loop */
static void
rx_loop(void)
{
    l2p_lport_t *lport;
    uint16_t rx_burst = info->burst_count * 2;
    struct rte_mbuf *mbufs[rx_burst];

    lport = info->lports[rte_lcore_id()];

    DBG_PRINT("Starting loop for lcore:port:queue %3u:%2u:%2u\n", rte_lcore_id(), lport->port->pid,
              lport->rx_qid);

    while (!info->force_quit)
        do_rx_process(lport, mbufs, rx_burst, rte_rdtsc());

    DBG_PRINT("Exiting loop for lcore:port:queue %3u:%2u:%2u\n", rte_lcore_id(), lport->port->pid,
              lport->rx_qid);
}

static void
tx_loop(void)
{
    l2p_lport_t *lport;
    l2p_port_t *port;
    uint64_t curr_tsc, burst_tsc;
    uint16_t tx_burst = info->burst_count;
    struct rte_mbuf *mbufs[tx_burst];

    lport = info->lports[rte_lcore_id()];
    port  = lport->port;

    DBG_PRINT("Starting loop for lcore:port:queue %3u:%2u:%2u\n", rte_lcore_id(), port->pid,
              lport->tx_qid);

    pthread_spin_lock(&port->tx_lock);
    if (port->tx_inited == 0) {
        port->tx_inited = 1;
        /* iterate over all buffers in the pktmbuf pool and setup the packet data */
        rte_mempool_obj_iter(port->tx_mp, mbuf_iterate_cb, (void *)lport);
    }
    pthread_spin_unlock(&port->tx_lock);

    burst_tsc = rte_rdtsc() + port->tx_cycles;

    while (!info->force_quit) {
        curr_tsc = rte_rdtsc();

        if (unlikely(curr_tsc >= burst_tsc)) {
            burst_tsc = curr_tsc + port->tx_cycles;

            if (likely(port->tx_cycles))
                do_tx_process(lport, mbufs, tx_burst, curr_tsc);
        }
    }
    DBG_PRINT("Exiting loop for lcore:port:queue %3u:%2u:%2u\n", rte_lcore_id(), port->pid,
              lport->tx_qid);
}

static void
rxtx_loop(void)
{
    l2p_lport_t *lport;
    l2p_port_t *port;
    uint64_t curr_tsc, burst_tsc;
    uint16_t rx_burst = info->burst_count * 2;
    uint16_t tx_burst = info->burst_count;
    struct rte_mbuf *mbufs[rx_burst];

    lport = info->lports[rte_lcore_id()];
    port  = lport->port;

    DBG_PRINT("Starting loop for lcore:port:queue %3u:%2u:%2u.%2u\n", rte_lcore_id(), port->pid,
              lport->rx_qid, lport->tx_qid);

    pthread_spin_lock(&port->tx_lock);
    if (port->tx_inited == 0) {
        port->tx_inited = 1;
        /* iterate over all buffers in the pktmbuf pool and setup the packet data */
        rte_mempool_obj_iter(port->tx_mp, mbuf_iterate_cb, (void *)lport);
    }
    pthread_spin_unlock(&port->tx_lock);

    burst_tsc = rte_rdtsc() + port->tx_cycles;

    while (!info->force_quit) {
        curr_tsc = rte_rdtsc();

        do_rx_process(lport, mbufs, rx_burst, curr_tsc);

        if (unlikely(curr_tsc >= burst_tsc)) {
            burst_tsc = curr_tsc + port->tx_cycles;

            if (likely(port->tx_cycles))
                do_tx_process(lport, mbufs, tx_burst, curr_tsc);
        }
    }
    DBG_PRINT("Exiting loop for lcore:port:queue %3u:%2u:%2u.%u\n", rte_lcore_id(), port->pid,
              lport->rx_qid, lport->tx_qid);
}

static int
txpkts_launch_one_lcore(__rte_unused void *dummy)
{
    l2p_lport_t *lport = info->lports[rte_lcore_id()];

    if (lport == NULL || lport->port == NULL || lport->port->pid >= RTE_MAX_ETHPORTS)
        return 0;

    if (lport == NULL || lport->port == NULL || lport->port->pid >= RTE_MAX_ETHPORTS)
        return 0;

    switch (lport->mode) {
    case LCORE_MODE_RX:
        rx_loop();
        break;
    case LCORE_MODE_TX:
        tx_loop();
        break;
    case LCORE_MODE_BOTH:
        rxtx_loop();
        break;
    case LCORE_MODE_UNKNOWN:
    default:
        rte_exit(EXIT_FAILURE, "Invalid mode %u\n", lport->mode);
        break;
    }
    return 0;
}

static void
signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        DBG_PRINT("\n\nSignal %d received, preparing to exit...\n", signum);
        info->force_quit = true;
    }
}

int
main(int argc, char **argv)
{
    int ret, lid;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    srandom(RANDOM_SEED);
    setlocale(LC_ALL, "");

    (void)rte_set_application_usage_hook(usage);

    /* Init EAL. */
    if ((ret = rte_eal_init(argc, argv)) < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
    argc -= ret;
    argv += ret;

    /* parse application arguments (after the EAL ones) */
    if (parse_args(argc, argv) < 0)
        rte_exit(EXIT_FAILURE, "Invalid PKTPERF arguments\n");

    for (int pid = 0; pid < info->num_ports; pid++) {
        if (port_setup(&info->ports[pid]) < 0)
            rte_exit(EXIT_FAILURE, "Port setup failed\n");
    }

    info->fgen = fgen_create(0);

    /* launch per-lcore init on every worker lcore */
    rte_eal_mp_remote_launch(txpkts_launch_one_lcore, NULL, SKIP_MAIN);

    for (int i = 0; i < 10; i++)
        PRINT("\n\n\n\n\n\n\n\n\n\n\n");

    /* Display the statistics  */
    do {
        print_stats();
        rte_delay_us_sleep(info->timeout_secs * Million);
    } while (!info->force_quit);

    RTE_LCORE_FOREACH_WORKER(lid)
    {
        DBG_PRINT("Waiting for lcore %d to exit\n", lid);
        if (rte_eal_wait_lcore(lid) < 0)
            rte_exit(EXIT_FAILURE, "Error waiting for lcore %d to exit\n", lid);
    }

    for (uint16_t portid = 0; portid < info->num_ports; portid++) {
        DBG_PRINT("Closing port %d... ", portid);
        if (rte_eth_dev_stop(portid) == 0)
            rte_eth_dev_close(portid);
        DBG_PRINT("\n");
    }

    rte_eal_cleanup();
    free(info);
    PRINT("Bye...\n");

    return 0;
}
