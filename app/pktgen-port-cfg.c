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

// clang-format off
static struct rte_eth_conf default_port_conf = {
    .rxmode = {
        .mq_mode          = RTE_ETH_MQ_RX_RSS,
        .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
        .offloads         = RTE_ETH_RX_OFFLOAD_CHECKSUM,
        .mtu              = RTE_ETHER_MTU
    },
    .txmode = {
        .mq_mode = RTE_ETH_MQ_TX_NONE,
    },
    .rx_adv_conf = {
        .rss_conf = {
            .rss_key = NULL,
            .rss_hf  = RTE_ETH_RSS_IP | RTE_ETH_RSS_TCP | RTE_ETH_RSS_UDP |
                      RTE_ETH_RSS_SCTP | RTE_ETH_RSS_L2_PAYLOAD,
        },
    },
    .intr_conf = {
        .lsc = 0,
    },
};
// clang-format on

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

static uint32_t
eth_dev_get_overhead_len(uint32_t max_rx_pktlen, uint16_t max_mtu)
{
    uint32_t overhead_len;

    if (max_mtu != UINT16_MAX && max_rx_pktlen > max_mtu)
        overhead_len = max_rx_pktlen - max_mtu;
    else
        overhead_len = RTE_ETHER_HDR_LEN + RTE_ETHER_CRC_LEN;

    return overhead_len;
}

#define MAX_JUMBO_PKT_LEN 9600
static port_info_t *
initialize_port_info(uint16_t pid)
{
    port_info_t *pinfo = l2p_get_port_pinfo(pid);
    int32_t sid = pg_eth_dev_socket_id(pid), ret = 0;
    struct rte_eth_conf conf;
    uint32_t eth_overhead_len;
    uint32_t max_mtu;

    /* If port info is already set ignore */
    if (pinfo) {
        pktgen_log_panic("Port info already setup for port %u", pid);
        return pinfo;
    }

    /* Allocate each port_info_t structure on the correct NUMA node for the port */
    pinfo = rte_zmalloc_socket(NULL, sizeof(port_info_t), RTE_CACHE_LINE_SIZE, sid);
    if (!pinfo)
        pktgen_log_panic("Cannot allocate memory for port_info_t");

    pinfo->pid = pid;
    l2p_set_port_pinfo(pid, pinfo);

    if (rte_eth_dev_info_get(pid, &pinfo->dev_info) < 0)
        rte_exit(EXIT_FAILURE, "Cannot get device info for port %u", pid);

    /* Get a clean copy of the configuration structure */
    rte_memcpy(&conf, &default_port_conf, sizeof(struct rte_eth_conf));

    if (pktgen.flags & JUMBO_PKTS_FLAG) {
        conf.rxmode.max_lro_pkt_size = RTE_ETHER_MAX_JUMBO_FRAME_LEN;
        eth_overhead_len =
            eth_dev_get_overhead_len(pinfo->dev_info.max_rx_pktlen, pinfo->dev_info.max_mtu);
        max_mtu = pinfo->dev_info.max_mtu - eth_overhead_len;
        /* device may have higher theoretical MTU e.g. for infiniband */
        if (max_mtu > MAX_JUMBO_PKT_LEN)
            max_mtu = MAX_JUMBO_PKT_LEN;
        pktgen_log_info("   Max MTU: %d", max_mtu);
        conf.rxmode.mtu = max_mtu;
        if (pinfo->dev_info.rx_offload_capa & RTE_ETH_RX_OFFLOAD_SCATTER)
            conf.rxmode.offloads |= RTE_ETH_RX_OFFLOAD_SCATTER;
        if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MULTI_SEGS)
            conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MULTI_SEGS;
    }

    if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_TCP_CKSUM) {
        pktgen_log_info("   Enabling Tx TCP_CKSUM offload");
        conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_TCP_CKSUM;
    }

    if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_UDP_CKSUM) {
        pktgen_log_info("   Enabling Tx UDP_CKSUM offload\r\n");
        conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_UDP_CKSUM;
    }

    if (pinfo->dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) {
        pktgen_log_info("   Enabling Tx IPV4_CKSUM offload\r\n");
        conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_IPV4_CKSUM;
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

    pktgen_log_info("   Allocate packet sequence array");

    /* allocate the sequence packet array */
    pinfo->seq_pkt =
        rte_zmalloc_socket(NULL, (sizeof(pkt_seq_t) * NUM_TOTAL_PKTS), RTE_CACHE_LINE_SIZE, sid);
    if (pinfo->seq_pkt == NULL)
        pktgen_log_panic("Unable to allocate %'ld pkt_seq_t headers", (long int)NUM_TOTAL_PKTS);

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

    pktgen_log_info("   Setup latency defaults");

    latency_t *lat           = &pinfo->latency;
    lat->jitter_threshold_us = DEFAULT_JITTER_THRESHOLD;
    lat->latency_rate_us     = DEFAULT_LATENCY_RATE;
    lat->latency_entropy     = DEFAULT_LATENCY_ENTROPY;
    lat->latency_rate_cycles =
        pktgen_get_timer_hz() / ((uint64_t)MAX_LATENCY_RATE / lat->latency_rate_us);
    uint64_t ticks               = pktgen_get_timer_hz() / (uint64_t)1000000;
    lat->jitter_threshold_cycles = lat->jitter_threshold_us * ticks;

    pktgen_log_info("   Setup fill pattern defaults");

    pinfo->fill_pattern_type = ABC_FILL_PATTERN;
    snprintf(pinfo->user_pattern, sizeof(pinfo->user_pattern), "%s", "0123456789abcdef");

    pktgen_log_info("   Configure device");
    if ((ret = rte_eth_dev_configure(pid, l2p_get_rxcnt(pid), l2p_get_txcnt(pid), &conf)) < 0)
        pktgen_log_panic("Cannot configure device: port=%d, Num queues %d,%d", pid,
                         l2p_get_rxcnt(pid), l2p_get_txcnt(pid));

    pktgen_log_info("   Setup number of descriptors RX: %u, TX: %u", pktgen.nb_rxd, pktgen.nb_txd);
    ret = rte_eth_dev_adjust_nb_rx_tx_desc(pid, &pktgen.nb_rxd, &pktgen.nb_txd);
    if (ret < 0)
        pktgen_log_panic("Can't adjust number of descriptors: port=%u:%s", pid, rte_strerror(-ret));
    pktgen_log_info("           Updated descriptors RX: %u, TX: %u", pktgen.nb_rxd, pktgen.nb_txd);

    if ((ret = rte_eth_macaddr_get(pid, &pinfo->src_mac)) < 0)
        pktgen_log_panic("Can't get MAC address: err=%d, port=%u", ret, pid);

    char buff[64];
    pktgen_log_info("   Source MAC: %s", inet_mtoa(buff, sizeof(buff), &pinfo->src_mac));

    pktgen_log_info("   Setup up Ptypes");
    ret = rte_eth_dev_set_ptypes(pid, RTE_PTYPE_UNKNOWN, NULL, 0);
    if (ret < 0)
        pktgen_log_panic("Port %u, Failed to disable Ptype parsing", pid);

    l2p_port_t *lport = l2p_get_port(pid);
    if (lport == NULL)
        pktgen_log_panic("Failed: l2p_port_t for port %u not found", pid);

    pktgen_log_info("   Number of RX/TX queues %u/%u", l2p_get_rxcnt(pid), l2p_get_txcnt(pid));
    for (int q = 0; q < l2p_get_rxcnt(pid); q++) {
        struct rte_eth_rxconf rxq_conf;
        struct rte_eth_conf conf = {0};

        if (rte_eth_dev_conf_get(pid, &conf) < 0)
            pktgen_log_panic("rte_eth_dev_conf_get: err=%d, port=%d", ret, pid);

        rxq_conf          = pinfo->dev_info.default_rxconf;
        rxq_conf.offloads = conf.rxmode.offloads;

        pktgen_log_info("   RX queue %d enabled offloads: 0x%0lx", q, rxq_conf.offloads);

        ret = rte_eth_rx_queue_setup(pid, q, pktgen.nb_rxd, sid, &rxq_conf, lport->rx_mp);
        if (ret < 0)
            pktgen_log_panic("rte_eth_rx_queue_setup: err=%d, port=%d, %s", ret, pid,
                             rte_strerror(-ret));
    }

    for (int q = 0; q < l2p_get_txcnt(pid); q++) {
        struct rte_eth_txconf *txconf;

        txconf           = &pinfo->dev_info.default_txconf;
        txconf->offloads = conf.txmode.offloads;

        pktgen_log_info("   TX queue %d enabled offloads: 0x%0lx", q, txconf->offloads);

        ret = rte_eth_tx_queue_setup(pid, q, pktgen.nb_txd, sid, txconf);
        if (ret < 0)
            pktgen_log_panic("rte_eth_tx_queue_setup: err=%d, port=%d, %s", ret, pid,
                             rte_strerror(-ret));
    }
    if (pktgen.verbose)
        pktgen_log_info("%*sPort memory used = %6lu KB", 57, " ", (pktgen.mem_used + 1023) / 1024);

    if (pktgen.verbose)
        rte_eth_dev_info_dump(stderr, pid);

    /* If enabled, put device in promiscuous mode. */
    if (pktgen.flags & PROMISCUOUS_ON_FLAG) {
        pktgen_log_info("   Enabling promiscuous mode");
        if (rte_eth_promiscuous_enable(pid))
            pktgen_log_info("Enabling promiscuous failed: %s", rte_strerror(-rte_errno));
    }

    pktgen_log_info("   Setup port defaults");
    pktgen_port_defaults(pid);

    pktgen_log_info("   Start network device");
    /* Start device */
    if ((ret = rte_eth_dev_start(pid)) < 0)
        pktgen_log_panic("rte_eth_dev_start: port=%d, %s", pid, rte_strerror(-ret));

    pktgen_set_port_flags(pinfo, SEND_SINGLE_PKTS);
    return pinfo;
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
    uint16_t pid;
    port_info_t *pinfo;

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

    RTE_ETH_FOREACH_DEV(pid)
    {
        pktgen_log_info("Initialize Port %u ... ", pid);

        pinfo = initialize_port_info(pid);
        if (pinfo == NULL) {
            pktgen_log_info("Failed: port_info_t for port %u initialize\n", pid);
            return;
        }
    }

    RTE_ETH_FOREACH_DEV(pid)
    {
        pktgen_log_info("Initialize Port %u sequence ... ", pid);
        pinfo = l2p_get_port_pinfo(pid);

        pktgen_log_info("   Setup sequence defaults");
        /* Setup the port and packet defaults */
        pktgen_seq_defaults(pid);

        pktgen_log_info("   Setup range defaults");
        pktgen_range_setup(pinfo);

        pktgen_log_info("   Setup latency defaults");
        pktgen_latency_setup(pinfo);

        pktgen_log_info("   Setup clear stats");
        pktgen_clear_stats(pinfo);

        pktgen_log_info("   Setup random bits");
        pktgen_rnd_bits_init(&pinfo->rnd_bitfields);
    }

    /* Clear the log information by putting a blank line */
    if (pktgen.verbose)
        dump_device_info();

    /* Setup the packet capture per port if needed. */
    for (uint16_t sid = 0; sid < coreinfo_socket_cnt(); sid++)
        pktgen_packet_capture_init(sid);
}
