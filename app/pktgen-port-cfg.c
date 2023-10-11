/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <pg_delay.h>
#include <cli_scrn.h>
#include <lua_config.h>

#include "pktgen-port-cfg.h"

#include "pktgen.h"
#include "pktgen-cmds.h"
#include "pktgen-log.h"
#include "pktgen-txbuff.h"

#include <link.h>

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
#include <rte_eth_bond_8023ad.h>
#endif
#include <rte_bus_pci.h>
#include <rte_bus.h>

enum {
    RX_PTHRESH = 8, /**< Default values of RX prefetch threshold reg. */
    RX_HTHRESH = 8, /**< Default values of RX host threshold reg. */
    RX_WTHRESH = 4, /**< Default values of RX write-back threshold reg. */

    TX_PTHRESH     = 36, /**< Default values of TX prefetch threshold reg. */
    TX_HTHRESH     = 0,  /**< Default values of TX host threshold reg. */
    TX_WTHRESH     = 0,  /**< Default values of TX write-back threshold reg. */
    TX_WTHRESH_1GB = 16, /**< Default value for 1GB ports */
};

static uint8_t hw_strip_crc = 0;

static struct rte_eth_conf default_port_conf = {
    .rxmode =
        {
            .mq_mode          = RTE_ETH_MQ_RX_RSS,
            .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
        },

    .rx_adv_conf =
        {
            .rss_conf =
                {
                    .rss_key = NULL,
                    .rss_hf  = RTE_ETH_RSS_IP | RTE_ETH_RSS_TCP | RTE_ETH_RSS_UDP |
                              RTE_ETH_RSS_SCTP | RTE_ETH_RSS_L2_PAYLOAD,
                },
        },
    .txmode =
        {
            .mq_mode = RTE_ETH_MQ_TX_NONE,
        },
    .intr_conf =
        {
            .lsc = 0,
        },
};

void
pktgen_set_hw_strip_crc(uint8_t val)
{
    hw_strip_crc = val;
}

int
pktgen_get_hw_strip_crc(void)
{
    return (hw_strip_crc) ? RTE_ETHER_CRC_LEN : 0;
}

/**
 *
 * pktgen_mbuf_pool_create - Create mbuf packet pool.
 *
 * DESCRIPTION
 * Callback routine for creating mbuf packets from a mempool.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static struct rte_mempool *
pktgen_mbuf_pool_create(const char *type, uint8_t pid, uint8_t queue_id, uint32_t nb_mbufs,
                        int socket_id, int cache_size)
{
    struct rte_mempool *mp;
    char name[RTE_MEMZONE_NAMESIZE];
    uint64_t sz;

    snprintf(name, sizeof(name), "%-12s%u:%u", type, pid, queue_id);

    sz = nb_mbufs * DEFAULT_MBUF_SIZE;
    sz = RTE_ALIGN_CEIL(sz + sizeof(struct rte_mempool), 1024);

    if (pktgen.verbose)
        pktgen_log_info("    Create: '%-*s' - Memory used (MBUFs %6u x size %6u) = %6lu KB", 16,
                        name, nb_mbufs, DEFAULT_MBUF_SIZE, sz / 1024);

    pktgen.mem_used += sz;
    pktgen.total_mem_used += sz;

    /* create the mbuf pool */
    mp = rte_pktmbuf_pool_create(name, nb_mbufs, cache_size, DEFAULT_PRIV_SIZE, DEFAULT_MBUF_SIZE,
                                 socket_id);
    if (mp == NULL)
        pktgen_log_panic(
            "Cannot create mbuf pool (%s) port %d, queue %d, nb_mbufs %d, socket_id %d: %s", name,
            pid, queue_id, nb_mbufs, socket_id, rte_strerror(rte_errno));

    return mp;
}

/**
 *
 * pktgen_config_ports - Configure the ports for RX and TX
 *
 * DESCRIPTION
 * Handle setting up the ports in DPDK.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_config_ports(void)
{
    struct rte_eth_conf conf;
    uint32_t pid, s, sid;
    rxtx_t rt;
    pkt_seq_t *pkt;
    port_info_t *info;
    char buff[RTE_MEMZONE_NAMESIZE];
    int32_t ret, cache_size;
    uint64_t ticks;
    uint32_t mbufs_per_port = MAX_MBUFS_PER_PORT(pktgen.nb_rxd, pktgen.nb_txd);

    /* Find out the total number of ports in the system. */
    /* We have already blocklisted the ones we needed to in main routine. */
    pktgen.nb_ports = rte_eth_dev_count_avail();
    if (pktgen.nb_ports > RTE_MAX_ETHPORTS)
        pktgen.nb_ports = RTE_MAX_ETHPORTS;

    for (int i = 0; i < pktgen.nb_ports; i++) {
        struct rte_eth_dev_info dev;
        char buff[64];

        rte_eth_dev_info_get(i, &dev);

        buff[0]                   = 0;
        const struct rte_bus *bus = NULL;
        if (dev.device)
            bus = rte_bus_find_by_device(dev.device);
        if (bus && !strcmp(rte_bus_name(bus), "pci")) {
            char name[RTE_ETH_NAME_MAX_LEN];
            char vend[8], device[8];

            printf("   %2d: %-12s   %2d     %2d   ", i, dev.driver_name, dev.if_index,
                   rte_dev_numa_node(dev.device));
            vend[0] = device[0] = '\0';
            sscanf(rte_dev_bus_info(dev.device), "vendor_id=%4s, device_id=%4s", vend, device);

            rte_eth_dev_get_name_by_port(i, name);
            snprintf(buff, sizeof(buff), "%s:%s/%s", vend, device, rte_dev_name(dev.device));
        } else
            snprintf(buff, sizeof(buff), "-1/0000:0000/00:00.0");
        printf("%s\n", buff);
    }
    printf("\n");

    if (pktgen.nb_ports == 0)
        pktgen_log_panic("*** Did not find any ports to use ***");

    pktgen.starting_port = 0;

    /* Setup the number of ports to display at a time */
    if (pktgen.nb_ports > pktgen.nb_ports_per_page)
        pktgen.ending_port = pktgen.starting_port + pktgen.nb_ports_per_page;
    else
        pktgen.ending_port = pktgen.starting_port + pktgen.nb_ports;

    if (pktgen.verbose) {
        pg_port_matrix_dump(pktgen.l2p);

        pktgen_log_info(">>>> Configuring %d ports, MBUF Size %d, MBUF Cache Size %d",
                        pktgen.nb_ports, DEFAULT_MBUF_SIZE, MBUF_CACHE_SIZE);
    }

    /* For each lcore setup each port that is handled by that lcore. */
    for (uint32_t lid = 0; lid < RTE_MAX_LCORE; lid++) {
        if (get_map(pktgen.l2p, RTE_MAX_ETHPORTS, lid) == 0)
            continue;

        /* For each port attached or handled by the lcore */
        RTE_ETH_FOREACH_DEV(pid)
        {
            /* If non-zero then this port is handled by this lcore. */
            if (get_map(pktgen.l2p, pid, lid) == 0)
                continue;
            pg_set_port_private(pktgen.l2p, pid, &pktgen.info[pid]);
            pktgen.info[pid].pid = pid;
        }
    }
    if (pktgen.verbose)
        pg_dump_l2p(pktgen.l2p);

    pktgen.total_mem_used = 0;

    RTE_ETH_FOREACH_DEV(pid)
    {
        /* Skip if we do not have any lcores attached to a port. */
        if ((rt.rxtx = get_map(pktgen.l2p, pid, RTE_MAX_LCORE)) == 0)
            continue;

        pktgen.port_cnt++;
        pktgen_log_info("Initialize Port %u -- RxQ %u, TxQ %u", pid, rt.rx, rt.tx);

        info = get_port_private(pktgen.l2p, pid);

        info->fill_pattern_type = ABC_FILL_PATTERN;
        snprintf(info->user_pattern, USER_PATTERN_SIZE, "%s", "0123456789abcdef");

        rte_spinlock_init(&info->port_lock);

        pktgen_rate_init(info);

        /* Create the pkt header structures for transmitting sequence of packets. */
        snprintf(buff, sizeof(buff), "seq_hdr_%u", pid);
        info->seq_pkt = rte_zmalloc_socket(buff, (sizeof(pkt_seq_t) * NUM_TOTAL_PKTS),
                                           RTE_CACHE_LINE_SIZE, rte_socket_id());
        if (info->seq_pkt == NULL)
            pktgen_log_panic("Unable to allocate %ld pkt_seq_t headers", (long int)NUM_TOTAL_PKTS);

        for (int i = 0; i < NUM_TOTAL_PKTS; i++) {
            info->seq_pkt[i].seq_enabled = 1;
            info->seq_pkt[i].tcp_flags   = DEFAULT_TCP_FLAGS;
            info->seq_pkt[i].tcp_seq     = DEFAULT_TCP_SEQ_NUMBER;
            info->seq_pkt[i].tcp_ack     = DEFAULT_TCP_ACK_NUMBER;
        }

        info->seqIdx = 0;
        info->seqCnt = 0;

        latency_t *lat = &info->latency;

        lat->jitter_threshold_us = DEFAULT_JITTER_THRESHOLD;
        lat->latency_rate_us     = DEFAULT_LATENCY_RATE;
        lat->latency_entropy     = DEFAULT_LATENCY_ENTROPY;
        lat->latency_rate_cycles =
            pktgen_get_timer_hz() / (MAX_LATENCY_RATE / lat->latency_rate_us);
        ticks                        = pktgen_get_timer_hz() / 1000000;
        lat->jitter_threshold_cycles = lat->jitter_threshold_us * ticks;
        info->nb_mbufs               = mbufs_per_port;
        cache_size = (info->nb_mbufs > RTE_MEMPOOL_CACHE_MAX_SIZE) ? RTE_MEMPOOL_CACHE_MAX_SIZE
                                                                   : info->nb_mbufs;
        rte_eth_dev_info_get(pid, &info->dev_info);

        if (pktgen.eth_mtu < info->dev_info.min_mtu) {
            pktgen_log_info("Increasing MTU from %u to %u", pktgen.eth_mtu, info->dev_info.min_mtu);
            pktgen.eth_mtu     = info->dev_info.min_mtu;
            pktgen.eth_max_pkt = info->dev_info.min_mtu + RTE_ETHER_HDR_LEN;
        }
        if (pktgen.eth_mtu > info->dev_info.max_mtu) {
            pktgen_log_info("Reducing MTU from %u to %u", pktgen.eth_mtu, info->dev_info.max_mtu);
            pktgen.eth_mtu     = info->dev_info.max_mtu;
            pktgen.eth_max_pkt = info->dev_info.max_mtu + RTE_ETHER_HDR_LEN;
        }

        /* Get a clean copy of the configuration structure */
        rte_memcpy(&conf, &default_port_conf, sizeof(struct rte_eth_conf));

        if (pktgen.flags & JUMBO_PKTS_FLAG) {
            conf.rxmode.max_lro_pkt_size = pktgen.eth_max_pkt;
            if (info->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MULTI_SEGS)
                conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MULTI_SEGS;
        }
        if (rt.rx > 1) {
            conf.rx_adv_conf.rss_conf.rss_key = NULL;
            conf.rx_adv_conf.rss_conf.rss_hf &= info->dev_info.flow_type_rss_offloads;
        } else {
            conf.rx_adv_conf.rss_conf.rss_key = NULL;
            conf.rx_adv_conf.rss_conf.rss_hf  = 0;
        }

        if (info->dev_info.max_vfs == 0) {
            if (conf.rx_adv_conf.rss_conf.rss_hf != 0)
                conf.rxmode.mq_mode = RTE_ETH_MQ_RX_RSS;
            else
                conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;
        } else {
            if (conf.rx_adv_conf.rss_conf.rss_hf != 0)
                conf.rxmode.mq_mode = RTE_ETH_MQ_RX_VMDQ_RSS;
            else
                conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;
        }

        info->lsc_enabled = 0;
        if (*info->dev_info.dev_flags & RTE_ETH_DEV_INTR_LSC) {
            conf.intr_conf.lsc = 1;
            info->lsc_enabled  = 1;
        }

        if ((ret = rte_eth_dev_configure(pid, rt.rx, rt.tx, &conf)) < 0)
            pktgen_log_panic("Cannot configure device: port=%d, Num queues %d,%d (%d)%s", pid,
                             rt.rx, rt.tx, -ret, rte_strerror(-ret));

        pkt = &info->seq_pkt[SINGLE_PKT];

        pktgen.mem_used = 0;

        if ((ret = rte_eth_dev_set_mtu(pid, pktgen.eth_mtu)) < 0)
            pktgen_log_panic("Cannot set MTU %u on port %u, (%d)%s", pktgen.eth_mtu, pid, -ret,
                             rte_strerror(-ret));

        for (int q = 0; q < rt.rx; q++) {
            struct rte_eth_rxconf rxq_conf;
            struct rte_eth_conf conf = {0};

            /* grab the socket id value based on the lcore being used. */
            sid = rte_lcore_to_socket_id(get_port_lid(pktgen.l2p, pid, q));

            /* Create and initialize the default Receive buffers. */
            info->q[q].rx_mp =
                pktgen_mbuf_pool_create("Default RX", pid, q, info->nb_mbufs, sid, cache_size);
            if (info->q[q].rx_mp == NULL)
                pktgen_log_panic("Cannot init port %d for Default RX mbufs", pid);

            rte_eth_dev_conf_get(pid, &conf);

            rte_eth_dev_info_get(pid, &info->dev_info);
            rxq_conf          = info->dev_info.default_rxconf;
            rxq_conf.offloads = conf.rxmode.offloads;
            ret               = rte_eth_rx_queue_setup(pid, q, pktgen.nb_rxd, sid, &rxq_conf,
                                                       pktgen.info[pid].q[q].rx_mp);
            if (ret < 0)
                pktgen_log_panic("rte_eth_rx_queue_setup: err=%d, port=%d, %s", ret, pid,
                                 rte_strerror(-ret));
            uint32_t lid = get_port_lid(pktgen.l2p, pid, q);
            rte_eth_dev_set_rx_queue_stats_mapping(pid, q, lid);
        }

        /* grab the socket id value based on the lcore being used. */
        sid = rte_lcore_to_socket_id(get_port_lid(pktgen.l2p, pid, 0));

        /* Used for sending special packets like ARP requests */
        info->special_mp = pktgen_mbuf_pool_create("Special TX", pid, 0, MAX_SPECIAL_MBUFS, sid, 0);
        if (info->special_mp == NULL)
            pktgen_log_panic("Cannot init port %d for Special TX mbufs", pid);

        if (pktgen.verbose)
            pktgen_log_info("");

        for (int q = 0; q < rt.tx; q++) {
            struct rte_eth_txconf *txconf;
            struct eth_tx_buffer *txbuff = NULL;
            int err;
            size_t sz;

            /* Add a few extra entries to account for special pkts i.e., latency */
            sz = TX_BUFFER_SIZE(MAX_PKT_RX_BURST);
            sz = RTE_ALIGN_CEIL(sz, RTE_CACHE_LINE_SIZE);
            if ((err = posix_memalign((void **)&txbuff, RTE_CACHE_LINE_SIZE, sz)) < 0)
                pktgen_log_panic("Cannot allocate eth_tx_buffer structure: %s\n", strerror(err));
            memset(txbuff, 0, sz);
            tx_buffer_init(txbuff, MAX_PKT_TX_BURST, pid, q);
            info->q[q].txbuff = txbuff;

            /* Create and initialize the default Transmit buffers. */
            info->q[q].tx_mp =
                pktgen_mbuf_pool_create("Default TX", pid, q, mbufs_per_port, sid, cache_size);
            if (info->q[q].tx_mp == NULL)
                pktgen_log_panic("Cannot init port %d for Default TX mbufs", pid);

            /* Create and initialize the range Transmit buffers. */
            info->q[q].range_mp =
                pktgen_mbuf_pool_create("Range TX", pid, q, mbufs_per_port, sid, 0);
            if (info->q[q].range_mp == NULL)
                pktgen_log_panic("Cannot init port %d for Range TX mbufs", pid);

            /* Create and initialize the rate Transmit buffers. */
            info->q[q].rate_mp = pktgen_mbuf_pool_create("Rate TX", pid, q, mbufs_per_port, sid, 0);
            if (info->q[q].rate_mp == NULL)
                pktgen_log_panic("Cannot init port %d for Rate TX mbufs", pid);

            /* Create and initialize the sequence Transmit buffers. */
            info->q[q].seq_mp =
                pktgen_mbuf_pool_create("Sequence TX", pid, q, mbufs_per_port, sid, cache_size);
            if (info->q[q].seq_mp == NULL)
                pktgen_log_panic("Cannot init port %d for Sequence TX mbufs", pid);

            /* Setup the PCAP file for each port */
            if (info->pcaps[q] != NULL) {
                if (pktgen_pcap_parse(info->pcaps[q], info, q) == -1) {
                    pktgen_log_panic("Cannot load PCAP file for port %d queue %d", pid, q);
                }
            } else if (info->pcap != NULL)
                if (pktgen_pcap_parse(info->pcap, info, q) == -1)
                    pktgen_log_panic("Cannot load PCAP file for port %d", pid);

            txconf           = &info->dev_info.default_txconf;
            txconf->offloads = default_port_conf.txmode.offloads;

            ret = rte_eth_tx_queue_setup(pid, q, pktgen.nb_txd, sid, txconf);
            if (ret < 0)
                pktgen_log_panic("rte_eth_tx_queue_setup: err=%d, port=%d, %s", ret, pid,
                                 rte_strerror(-ret));
            if (pktgen.verbose)
                pktgen_log_info("");
        }
        if (pktgen.verbose)
            pktgen_log_info("%*sPort memory used = %6lu KB", 57, " ",
                            (pktgen.mem_used + 1023) / 1024);

        /* Grab the source MAC addresses */
        rte_eth_macaddr_get(pid, &pkt->eth_src_addr);
        rte_eth_macaddr_get(pid, &info->seq_pkt[RATE_PKT].eth_src_addr);

        pktgen_log_info("   Src MAC %02x:%02x:%02x:%02x:%02x:%02x <Promiscuous is %s>",
                        pkt->eth_src_addr.addr_bytes[0], pkt->eth_src_addr.addr_bytes[1],
                        pkt->eth_src_addr.addr_bytes[2], pkt->eth_src_addr.addr_bytes[3],
                        pkt->eth_src_addr.addr_bytes[4], pkt->eth_src_addr.addr_bytes[5],
                        (pktgen.flags & PROMISCUOUS_ON_FLAG) ? "Enabled" : "Disabled");

        /* If enabled, put device in promiscuous mode. */
        if (pktgen.flags & PROMISCUOUS_ON_FLAG)
            rte_eth_promiscuous_enable(pid);

        /* Copy the first Src MAC address in SINGLE_PKT to the rest of the sequence packets. */
        for (int i = 0; i < NUM_SEQ_PKTS; i++)
            ethAddrCopy(&info->seq_pkt[i].eth_src_addr, &pkt->eth_src_addr);
        ethAddrCopy(&info->seq_pkt[RATE_PKT].eth_src_addr, &pkt->eth_src_addr);
        ethAddrCopy(&info->seq_pkt[LATENCY_PKT].eth_src_addr, &pkt->eth_src_addr);
        if (pktgen.verbose)
            rte_eth_dev_info_dump(NULL, pid);
    }
    if (pktgen.verbose)
        pktgen_log_info("%*sTotal memory used = %6lu KB", 70, " ",
                        (pktgen.total_mem_used + 1023) / 1024);
    else
        pktgen_log_info("");

    /* Start up the ports and display the port Link status */
    RTE_ETH_FOREACH_DEV(pid)
    {
        if (get_map(pktgen.l2p, pid, RTE_MAX_LCORE) == 0)
            continue;

        /* Start device */
        if ((ret = rte_eth_dev_start(pid)) < 0)
            pktgen_log_panic("rte_eth_dev_start: port=%d, %s", pid, rte_strerror(-ret));
    }

    /* Start up the ports and display the port Link status */
    RTE_ETH_FOREACH_DEV(pid)
    {
        if (get_map(pktgen.l2p, pid, RTE_MAX_LCORE) == 0)
            continue;

        info = get_port_private(pktgen.l2p, pid);

        pktgen.info[pid].seq_pkt[SINGLE_PKT].pktSize  = MIN_PKT_SIZE;
        pktgen.info[pid].seq_pkt[RATE_PKT].pktSize    = MIN_PKT_SIZE;
        pktgen.info[pid].seq_pkt[LATENCY_PKT].pktSize = LATENCY_PKT_SIZE;

        /* Setup the port and packet defaults. (must be after link speed is found) */
        for (s = 0; s < NUM_TOTAL_PKTS; s++)
            pktgen_port_defaults(pid, s);

        pktgen_range_setup(info);
        pktgen_latency_setup(info);

        pktgen_clear_stats(info);

        pktgen_rnd_bits_init(&pktgen.info[pid].rnd_bitfields);
    }

    /* Clear the log information by putting a blank line */
    pktgen_log_info("");

    /* Setup the packet capture per port if needed. */
    for (sid = 0; sid < coremap_cnt(pktgen.core_info, pktgen.core_cnt, 0); sid++)
        pktgen_packet_capture_init(&pktgen.capture[sid], sid);
}
