/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
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
#include "l2p.h"

#include <rte_dev.h>

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

static struct rte_eth_conf default_port_conf = {
    .rxmode =
        {
            .mq_mode          = RTE_ETH_MQ_RX_RSS,
            .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
            .offloads         = RTE_ETH_RX_OFFLOAD_CHECKSUM,
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

static void
dump_device_info(void)
{
    printf("\n%-4s %-16s %-5s %-4s %-17s %-17s %s\n", "Port", "DevName", "Index", "NUMA",
           "PCI Information", "Src MAC", "Promiscuous");

    for (uint16_t i = 0; i < pktgen.nb_ports; i++) {
        struct rte_eth_dev_info dev;
        const struct rte_bus *bus = NULL;
        port_info_t *pinfo;
        pkt_seq_t *pkt;
        char buff[128];

        if (rte_eth_dev_info_get(i, &dev)) {
            printf(buff, sizeof(buff), "%6u No device information: %s", i, rte_strerror(rte_errno));
            continue;
        }

        buff[0] = 0;
        if (dev.device)
            bus = rte_bus_find_by_device(dev.device);
        if (bus && !strcmp(rte_bus_name(bus), "pci")) {
            char name[RTE_ETH_NAME_MAX_LEN];
            char vend[8], device[8];

            vend[0] = device[0] = '\0';
            sscanf(rte_dev_bus_info(dev.device), "vendor_id=%4s, device_id=%4s", vend, device);

            rte_eth_dev_get_name_by_port(i, name);
            snprintf(buff, sizeof(buff), "%s:%s/%s", vend, device, rte_dev_name(dev.device));
        } else
            snprintf(buff, sizeof(buff), "non-PCI device");
        pinfo = l2p_get_port_pinfo(i);
        pkt   = &pinfo->seq_pkt[SINGLE_PKT];
        printf("%3u  %-16s %5d %4d %s %02x:%02x:%02x:%02x:%02x:%02x <%s>\n", i, dev.driver_name,
               dev.if_index, rte_dev_numa_node(dev.device), buff, pkt->eth_src_addr.addr_bytes[0],
               pkt->eth_src_addr.addr_bytes[1], pkt->eth_src_addr.addr_bytes[2],
               pkt->eth_src_addr.addr_bytes[3], pkt->eth_src_addr.addr_bytes[4],
               pkt->eth_src_addr.addr_bytes[5],
               (pktgen.flags & PROMISCUOUS_ON_FLAG) ? "Enabled" : "Disabled");
    }
    printf("\n");
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
    uint16_t pid;
    pkt_seq_t *pkt;
    port_info_t *pinfo;
    int32_t ret, sid;
    l2p_port_t *port;

    /* Find out the total number of ports in the system. */
    /* We have already block list the ones we needed to in main routine. */
    pktgen.nb_ports = rte_eth_dev_count_avail();
    if (pktgen.nb_ports == 0)
        pktgen_log_panic("*** Did not find any ports to use ***");
    if (pktgen.nb_ports > RTE_MAX_ETHPORTS)
        pktgen_log_panic("*** Too many ports in the system %d ***", pktgen.nb_ports);

    /* Setup the number of ports to display at a time */
    pktgen.ending_port =
        ((pktgen.nb_ports > pktgen.nb_ports_per_page) ? pktgen.nb_ports_per_page : pktgen.nb_ports);

    /* For each lcore setup each port that is handled by that lcore. */
    for (uint16_t lid = 0; lid < RTE_MAX_LCORE; lid++) {
        if ((pid = l2p_get_pid_by_lcore(lid)) >= RTE_MAX_ETHPORTS)
            continue;

        sid = rte_eth_dev_socket_id(pid);

        pinfo = l2p_get_port_pinfo(pid);
        if (pinfo == NULL) {
            /* Allocate each port_info_t structure on the correct NUMA node for the port */
            pinfo = rte_zmalloc_socket(NULL, sizeof(port_info_t), RTE_CACHE_LINE_SIZE, sid);
            if (!pinfo)
                rte_exit(EXIT_FAILURE, "Cannot allocate memory for port_info_t\n");

            pinfo->pid = pid;

            pinfo->fill_pattern_type = ABC_FILL_PATTERN;
            snprintf(pinfo->user_pattern, sizeof(pinfo->user_pattern), "%s", "0123456789abcdef");

            pinfo->seq_pkt = rte_zmalloc_socket(NULL, (sizeof(pkt_seq_t) * NUM_TOTAL_PKTS),
                                                RTE_CACHE_LINE_SIZE, sid);
            if (pinfo->seq_pkt == NULL)
                pktgen_log_panic("Unable to allocate %'ld pkt_seq_t headers",
                                 (long int)NUM_TOTAL_PKTS);

            size_t pktsz = RTE_ETHER_MAX_LEN;
            if (pktgen.flags & JUMBO_PKTS_FLAG)
                pktsz = RTE_ETHER_MAX_JUMBO_FRAME_LEN;

            for (int i = 0; i < NUM_TOTAL_PKTS; i++) {
                pinfo->seq_pkt[i].hdr = rte_zmalloc_socket(NULL, pktsz, RTE_CACHE_LINE_SIZE, sid);
                if (pinfo->seq_pkt[i].hdr == NULL)
                    pktgen_log_panic("Unable to allocate %ld pkt_seq_t buffer space", pktsz);

                pinfo->seq_pkt[i].seq_enabled = 1;
                pinfo->seq_pkt[i].tcp_flags   = DEFAULT_TCP_FLAGS;
                pinfo->seq_pkt[i].tcp_seq     = DEFAULT_TCP_SEQ_NUMBER;
                pinfo->seq_pkt[i].tcp_ack     = DEFAULT_TCP_ACK_NUMBER;
            }

            latency_t *lat           = &pinfo->latency;
            lat->jitter_threshold_us = DEFAULT_JITTER_THRESHOLD;
            lat->latency_rate_us     = DEFAULT_LATENCY_RATE;
            lat->latency_entropy     = DEFAULT_LATENCY_ENTROPY;
            lat->latency_rate_cycles =
                pktgen_get_timer_hz() / ((uint64_t)MAX_LATENCY_RATE / lat->latency_rate_us);
            uint64_t ticks               = pktgen_get_timer_hz() / (uint64_t)1000000;
            lat->jitter_threshold_cycles = lat->jitter_threshold_us * ticks;

            l2p_set_port_pinfo(pid, pinfo);
        }
    }

    RTE_ETH_FOREACH_DEV(pid)
    {
        pinfo = l2p_get_port_pinfo(pid);
        port  = l2p_get_port(pid);
        if (pinfo == NULL || port == NULL)
            continue;

        /* grab the socket id value based on the pid being used. */
        sid = rte_eth_dev_socket_id(pid);

        rte_eth_dev_info_get(pid, &pinfo->dev_info);

        pktgen_log_info("Initialize Port %u ... ", pid);

        /* Get a clean copy of the configuration structure */
        rte_memcpy(&conf, &default_port_conf, sizeof(struct rte_eth_conf));

        if (pktgen.flags & JUMBO_PKTS_FLAG) {
            conf.rxmode.max_lro_pkt_size = RTE_ETHER_MAX_JUMBO_FRAME_LEN;
            if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MULTI_SEGS)
                conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MULTI_SEGS;
        }

        conf.rx_adv_conf.rss_conf.rss_key = NULL;
        conf.rx_adv_conf.rss_conf.rss_hf &= pinfo->dev_info.flow_type_rss_offloads;
        if (pinfo->dev_info.max_rx_queues == 1)
            conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;

        if (pinfo->dev_info.max_vfs) {
            if (conf.rx_adv_conf.rss_conf.rss_hf != 0)
                conf.rxmode.mq_mode = RTE_ETH_MQ_RX_VMDQ_RSS;
        }

        pinfo->lsc_enabled = 0;
        if (*pinfo->dev_info.dev_flags & RTE_ETH_DEV_INTR_LSC) {
            conf.intr_conf.lsc = 1;
            pinfo->lsc_enabled = 1;
        }

        conf.rxmode.offloads &= pinfo->dev_info.rx_offload_capa;

        if ((ret = rte_eth_dev_configure(pid, l2p_get_rxcnt(pid), l2p_get_txcnt(pid), &conf)) < 0)
            pktgen_log_panic("Cannot configure device: port=%d, Num queues %d,%d", pid,
                             l2p_get_rxcnt(pid), l2p_get_txcnt(pid));

        ret = rte_eth_dev_adjust_nb_rx_tx_desc(pid, &pktgen.nb_rxd, &pktgen.nb_txd);
        if (ret < 0)
            rte_exit(EXIT_FAILURE, "Can't adjust number of descriptors: port=%u:%s\n", pid,
                     rte_strerror(-ret));

        pkt = &pinfo->seq_pkt[SINGLE_PKT];

        if ((ret = rte_eth_macaddr_get(pid, &pkt->eth_src_addr)) < 0)
            rte_exit(EXIT_FAILURE, "Can't get MAC address: err=%d, port=%u\n", ret, pid);

        ret = rte_eth_dev_set_ptypes(pid, RTE_PTYPE_UNKNOWN, NULL, 0);
        if (ret < 0)
            rte_exit(EXIT_FAILURE, "Port %u, Failed to disable Ptype parsing\n", pid);

        for (int q = 0; q < l2p_get_rxcnt(pid); q++) {
            struct rte_eth_rxconf rxq_conf;
            struct rte_eth_conf conf = {0};

            rte_eth_dev_conf_get(pid, &conf);

            rxq_conf          = pinfo->dev_info.default_rxconf;
            rxq_conf.offloads = conf.rxmode.offloads;

            ret = rte_eth_rx_queue_setup(pid, q, pktgen.nb_rxd, sid, &rxq_conf, port->rx_mp);
            if (ret < 0)
                pktgen_log_panic("rte_eth_rx_queue_setup: err=%d, port=%d, %s", ret, pid,
                                 rte_strerror(-ret));
        }

        for (int q = 0; q < l2p_get_txcnt(pid); q++) {
            struct rte_eth_txconf *txconf;

            txconf           = &pinfo->dev_info.default_txconf;
            txconf->offloads = default_port_conf.txmode.offloads;

            ret = rte_eth_tx_queue_setup(pid, q, pktgen.nb_txd, sid, txconf);
            if (ret < 0)
                pktgen_log_panic("rte_eth_tx_queue_setup: err=%d, port=%d, %s", ret, pid,
                                 rte_strerror(-ret));
        }
        if (pktgen.verbose)
            pktgen_log_info("%*sPort memory used = %6lu KB", 57, " ",
                            (pktgen.mem_used + 1023) / 1024);

        /* If enabled, put device in promiscuous mode. */
        if (pktgen.flags & PROMISCUOUS_ON_FLAG)
            if (rte_eth_promiscuous_enable(pid))
                rte_exit(EXIT_FAILURE, "Enabling promiscuous failed: %s\n",
                         rte_strerror(-rte_errno));

        /* Copy the first Src MAC address in SINGLE_PKT to the rest of the sequence packets. */
        for (int i = 0; i < NUM_SEQ_PKTS; i++)
            ethAddrCopy(&pinfo->seq_pkt[i].eth_src_addr, &pkt->eth_src_addr);
        ethAddrCopy(&pinfo->seq_pkt[RANGE_PKT].eth_src_addr, &pkt->eth_src_addr);
        ethAddrCopy(&pinfo->seq_pkt[LATENCY_PKT].eth_src_addr, &pkt->eth_src_addr);
        if (pktgen.verbose)
            rte_eth_dev_info_dump(NULL, pid);

        pinfo->seq_pkt[SINGLE_PKT].pkt_size  = RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN;
        pinfo->seq_pkt[RANGE_PKT].pkt_size   = RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN;
        pinfo->seq_pkt[LATENCY_PKT].pkt_size = RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN;

        /* Setup the port and packet defaults */
        for (uint8_t s = 0; s < NUM_TOTAL_PKTS; s++)
            pktgen_port_defaults(pid, s);

        pktgen_range_setup(pinfo);
        pktgen_latency_setup(pinfo);

        pktgen_clear_stats(pinfo);

        pktgen_rnd_bits_init(&pinfo->rnd_bitfields);

        /* Start device */
        if ((ret = rte_eth_dev_start(pid)) < 0)
            pktgen_log_panic("rte_eth_dev_start: port=%d, %s", pid, rte_strerror(-ret));
    }

    /* Clear the log information by putting a blank line */
    pktgen_log_info("");

    dump_device_info();

    /* Setup the packet capture per port if needed. */
    for (sid = 0; sid < coreinfo_socket_cnt(); sid++)
        pktgen_packet_capture_init(sid);
}
