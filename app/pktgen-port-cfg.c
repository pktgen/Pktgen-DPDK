/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include "pg_compat.h"
#include "pktgen-cmds.h"
#include "pktgen-log.h"
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

/**
 * An array of drivers that require a pseudo-header calculation before the checksum calculation.
 * The names used are the ones used by DPDK.
 */
static const char *DRIVERS_REQUIRING_PHDR[] = {
    "net_ixgbe",
    // TODO: Add the others
};

static struct rte_eth_conf default_port_conf = {
    .rxmode =
        {
            .mq_mode          = RTE_ETH_MQ_RX_RSS,
            .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
            .offloads         = RTE_ETH_RX_OFFLOAD_CHECKSUM,
            .mtu              = RTE_ETHER_MTU,
        },
    .txmode =
        {
            .mq_mode = RTE_ETH_MQ_TX_NONE,
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
    .intr_conf =
        {
            .lsc = 0,
        },
};

static void
dump_device_info(void)
{
    printf("\n%-4s %-16s %-5s %-4s %-22s %-17s %s\n", "Port", "DevName", "Index", "NUMA",
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
 * Determines whether the pseudo-header is required when calculating the checksum.
 * Depends on the original NIC driver (e.g., ixgbe NICs expect the pseudo-header)
 * See Table 1.133: https://doc.dpdk.org/guides/nics/overview.html
 */
bool
is_cksum_phdr_required(const char *driver_name)
{
    size_t num_drivers = RTE_DIM(DRIVERS_REQUIRING_PHDR);

    for (size_t i = 0; i < num_drivers; i++) {
        if (DRIVERS_REQUIRING_PHDR[i] == NULL)
            break;
        if (strcmp(driver_name, DRIVERS_REQUIRING_PHDR[i]) == 0)
            return true;
    }

    return false;
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

static port_info_t *
allocate_port_info(uint16_t pid)
{
    port_info_t *pinfo = l2p_get_port_pinfo(pid);
    int32_t sid        = pg_eth_dev_socket_id(pid);

    pktgen_log_info("Port info setup for port %u", pid);

    /* If port info is already set ignore */
    if (pinfo) {
        pktgen_log_error("Port info already setup for port %u", pid);
        goto leave;
    }

    /* Allocate each port_info_t structure on the correct NUMA node for the port */
    if ((pinfo = rte_zmalloc_socket(NULL, sizeof(port_info_t), RTE_CACHE_LINE_SIZE, sid)) == NULL)
        goto leave;

    pinfo->pid     = pid;
    pinfo->max_mtu = RTE_ETHER_MAX_LEN;
    pinfo->conf    = default_port_conf;

    if (rte_eth_dev_info_get(pid, &pinfo->dev_info) < 0) {
        pktgen_log_error("Cannot get device info for port %u", pid);
        goto leave;
    }

    for (int qid = 0; qid < l2p_get_txcnt(pid); qid++) {
        per_queue_t *pq = &pinfo->per_queue[qid];
        char buff[64];

        snprintf(buff, sizeof(buff), "RxMbufs-%u-%d", pid, qid);
        pq->rx_pkts = rte_calloc_socket(buff, MAX_PKT_RX_BURST, sizeof(struct rte_mbuf *),
                                        RTE_CACHE_LINE_SIZE, sid);
        if (pq->rx_pkts == NULL) {
            pktgen_log_error("Cannot allocate RX burst for port %u-%d", pid, qid);
            goto leave;
        }
    }

    for (int qid = 0; qid < l2p_get_txcnt(pid); qid++) {
        per_queue_t *pq = &pinfo->per_queue[qid];
        char buff[64];

        snprintf(buff, sizeof(buff), "TxMbufs-%u-%d", pid, qid);
        pq->tx_pkts = rte_calloc_socket(buff, MAX_PKT_TX_BURST, sizeof(struct rte_mbuf *),
                                        RTE_CACHE_LINE_SIZE, sid);
        if (pq->tx_pkts == NULL) {
            pktgen_log_error("Cannot allocate TX burst for port %u-%d", pid, qid);
            goto leave;
        }
    }

    if (l2p_set_port_pinfo(pid, pinfo)) {
        pktgen_log_error("Failed to set port info for port %u", pid);
        goto leave;
    }

    pktgen_log_info("   Allocate packet sequence array");

    size_t pktsz = RTE_ETHER_MAX_LEN;
    if (pktgen.flags & JUMBO_PKTS_FLAG)
        pktsz = RTE_ETHER_MAX_JUMBO_FRAME_LEN;

    /* allocate the sequence packet array */
    pinfo->seq_pkt =
        rte_zmalloc_socket(NULL, (sizeof(pkt_seq_t) * NUM_TOTAL_PKTS), RTE_CACHE_LINE_SIZE, sid);
    if (pinfo->seq_pkt == NULL) {
        pktgen_log_error("Unable to allocate %'ld pkt_seq_t headers", (long int)NUM_TOTAL_PKTS);
        goto leave;
    }

    for (int i = 0; i < NUM_TOTAL_PKTS; i++) {
        pinfo->seq_pkt[i].hdr = rte_zmalloc_socket(NULL, pktsz, RTE_CACHE_LINE_SIZE, sid);
        if (pinfo->seq_pkt[i].hdr == NULL)
            pktgen_log_panic("Unable to allocate %ld pkt_seq_t buffer space", pktsz);

        pinfo->seq_pkt[i].seq_enabled = 1;
        pinfo->seq_pkt[i].tcp_flags   = DEFAULT_TCP_FLAGS;
        pinfo->seq_pkt[i].tcp_seq     = DEFAULT_TCP_SEQ_NUMBER;
        pinfo->seq_pkt[i].tcp_ack     = DEFAULT_TCP_ACK_NUMBER;
    }

    /* Determines if pseudo-header is needed, based on the driver type */
    pinfo->cksum_requires_phdr = is_cksum_phdr_required(pinfo->dev_info.driver_name);
    pktgen_log_info("   Checksum offload Pseudo-header required: %s",
                    pinfo->cksum_requires_phdr ? "Yes" : "No");

    return pinfo;
leave:
    if (pinfo) {
        for (int i = 0; i < MAX_QUEUES_PER_PORT; i++) {
            per_queue_t *pq = &pinfo->per_queue[i];

            rte_free(pq->rx_pkts);
            rte_free(pq->tx_pkts);
        }
        rte_free(pinfo);
        l2p_set_port_pinfo(pid, NULL);
    }
    return NULL;
}

static void
_latency_defaults(port_info_t *pinfo)
{
    latency_t *lat = &pinfo->latency;

    pktgen_log_info("   Setup latency defaults");

    lat->jitter_threshold_us = DEFAULT_JITTER_THRESHOLD;
    lat->latency_rate_us     = DEFAULT_LATENCY_RATE;
    lat->latency_entropy     = DEFAULT_LATENCY_ENTROPY;
    lat->latency_rate_cycles =
        pktgen_get_timer_hz() / ((uint64_t)MAX_LATENCY_RATE / lat->latency_rate_us);
    uint64_t ticks               = pktgen_get_timer_hz() / (uint64_t)1000000;
    lat->jitter_threshold_cycles = lat->jitter_threshold_us * ticks;
}

static void
_fill_pattern_defaults(port_info_t *pinfo)
{
    pktgen_log_info("   Setup fill pattern defaults");

    pinfo->fill_pattern_type = ABC_FILL_PATTERN;
    snprintf(pinfo->user_pattern, sizeof(pinfo->user_pattern), "%s", "0123456789abcdef");
}

static void
_mtu_defaults(port_info_t *pinfo)
{
    struct rte_eth_dev_info *dinfo = &pinfo->dev_info;
    struct rte_eth_conf *conf      = &pinfo->conf;

    pktgen_log_info("   Setup MTU defaults and Jumbo Frames are %s",
                    (pktgen.flags & JUMBO_PKTS_FLAG) ? "Enabled" : "Disabled");

    if (pinfo->max_mtu < dinfo->min_mtu)
        pinfo->max_mtu = dinfo->min_mtu;
    if (pinfo->max_mtu > dinfo->max_mtu)
        pinfo->max_mtu = dinfo->max_mtu;
    if (pktgen.flags & JUMBO_PKTS_FLAG) {
        uint32_t eth_overhead_len;

        conf->rxmode.max_lro_pkt_size = PG_JUMBO_ETHER_MTU;
        eth_overhead_len = eth_dev_get_overhead_len(dinfo->max_rx_pktlen, dinfo->max_mtu);
        pinfo->max_mtu   = dinfo->max_mtu - eth_overhead_len;

        /* device may have higher theoretical MTU e.g. for infiniband */
        if (pinfo->max_mtu > PG_JUMBO_ETHER_MTU)
            pinfo->max_mtu = PG_JUMBO_ETHER_MTU;

        pktgen_log_info(
            "     Jumbo Frames: Default Max Rx pktlen: %'u, MTU %'u, overhead len: %'u, "
            "New MTU %'d",
            dinfo->max_rx_pktlen, dinfo->max_mtu, eth_overhead_len, pinfo->max_mtu);

        pktgen_log_info("     Note: Using Scatter/Multi-segs offloads");
        pktgen_log_info("     Note: Performance may be degraded due to reduced Tx performance");

        // FIXME: Tx performance takes a big hit when enabled
        if (dinfo->rx_offload_capa & RTE_ETH_RX_OFFLOAD_SCATTER)
            conf->rxmode.offloads |= RTE_ETH_RX_OFFLOAD_SCATTER;
        if (dinfo->tx_offload_capa & RTE_ETH_TX_OFFLOAD_MULTI_SEGS)
            conf->txmode.offloads |= RTE_ETH_TX_OFFLOAD_MULTI_SEGS;
    }
    conf->rxmode.mtu = pinfo->max_mtu;
}

static void
_rx_offload_defaults(port_info_t *pinfo)
{
    struct rte_eth_dev_info *dinfo = &pinfo->dev_info;
    struct rte_eth_conf *conf      = &pinfo->conf;

    conf->rx_adv_conf.rss_conf.rss_key = NULL;
    conf->rx_adv_conf.rss_conf.rss_hf &= dinfo->flow_type_rss_offloads;
    if (dinfo->max_rx_queues == 1)
        conf->rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;

    if (dinfo->max_vfs) {
        if (conf->rx_adv_conf.rss_conf.rss_hf != 0)
            conf->rxmode.mq_mode = RTE_ETH_MQ_RX_VMDQ_RSS;
    }
    conf->rxmode.offloads &= dinfo->rx_offload_capa;
}

static void
_tx_offload_defaults(port_info_t *pinfo)
{
    struct rte_eth_dev_info *dinfo = &pinfo->dev_info;
    struct rte_eth_conf *conf      = &pinfo->conf;

    pktgen_log_info("   Setup TX offload defaults");

    if (dinfo->tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
        conf->txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

#if 0        // FIXME: Tx performance drops when using these offloads.
    if (dinfo->tx_offload_capa & RTE_ETH_TX_OFFLOAD_TCP_CKSUM) {
        pktgen_log_info("   Enabling Tx TCP_CKSUM offload");
        conf->txmode.offloads |= RTE_ETH_TX_OFFLOAD_TCP_CKSUM;
    }

    if (dinfo->tx_offload_capa & RTE_ETH_TX_OFFLOAD_UDP_CKSUM) {
        pktgen_log_info("   Enabling Tx UDP_CKSUM offload\r\n");
        conf->txmode.offloads |= RTE_ETH_TX_OFFLOAD_UDP_CKSUM;
    }

    if (dinfo->tx_offload_capa & RTE_ETH_TX_OFFLOAD_IPV4_CKSUM) {
        pktgen_log_info("   Enabling Tx IPV4_CKSUM offload\r\n");
        conf->txmode.offloads |= RTE_ETH_TX_OFFLOAD_IPV4_CKSUM;
    }
#endif
}

static void
_device_configuration(port_info_t *pinfo)
{
    uint16_t pid = pinfo->pid;
    int ret;

    pktgen_log_info("   Configure device: RxQueueCnt: %u, TxQueueCnt: %u", l2p_get_rxcnt(pid),
                    l2p_get_txcnt(pid));

    ret = rte_eth_dev_configure(pid, l2p_get_rxcnt(pid), l2p_get_txcnt(pid), &pinfo->conf);
    if (ret < 0)
        pktgen_log_panic("Cannot configure device: port=%d, Num queues %d,%d", pid,
                         l2p_get_rxcnt(pid), l2p_get_txcnt(pid));
}

static void
_rxtx_descriptors(port_info_t *pinfo)
{
    uint16_t pid = pinfo->pid;
    int ret;

    pktgen_log_info("   Setup number of descriptors RX: %u, TX: %u", pktgen.nb_rxd, pktgen.nb_txd);

    ret = rte_eth_dev_adjust_nb_rx_tx_desc(pid, &pktgen.nb_rxd, &pktgen.nb_txd);
    if (ret < 0)
        pktgen_log_panic("Can't adjust number of descriptors: port=%u:%s", pid, rte_strerror(-ret));
    pktgen_log_info("           Updated descriptors RX: %u, TX: %u", pktgen.nb_rxd, pktgen.nb_txd);
}

static void
_src_mac_address(port_info_t *pinfo)
{
    uint16_t pid = pinfo->pid;
    char buff[64];

    int ret = rte_eth_macaddr_get(pid, &pinfo->src_mac);
    if (ret < 0)
        pktgen_log_panic("Port %u, Failed to get source MAC address, (%d)%s", pinfo->pid, -ret,
                         rte_strerror(-ret));
    else
        pktgen_log_info("   Source MAC: %s", inet_mtoa(buff, sizeof(buff), &pinfo->src_mac));
}

static void
_device_ptypes(port_info_t *pinfo)
{
    pktgen_log_info("   Setup up device Ptypes");

    int ret = rte_eth_dev_set_ptypes(pinfo->pid, RTE_PTYPE_UNKNOWN, NULL, 0);
    if (ret < 0)
        pktgen_log_panic("Port %u, Failed to disable Ptype parsing", pinfo->pid);
}

static void
_device_mtu(port_info_t *pinfo)
{
    struct rte_eth_dev_info *dinfo = &pinfo->dev_info;
    struct rte_eth_conf *conf      = &pinfo->conf;
    uint32_t max_mtu               = RTE_ETHER_MAX_LEN;
    int ret;

    if (max_mtu < dinfo->min_mtu) {
        pktgen_log_warning("Increasing MTU from %u to %u", max_mtu, dinfo->min_mtu);
        max_mtu = dinfo->min_mtu;
    }
    if (max_mtu > dinfo->max_mtu) {
        pktgen_log_warning("Reducing MTU from %u to %u", max_mtu, dinfo->max_mtu);
        max_mtu = dinfo->max_mtu;
    }
    conf->rxmode.mtu = max_mtu;

    if ((ret = rte_eth_dev_set_mtu(pinfo->pid, pinfo->max_mtu)) < 0)
        pktgen_log_panic("Cannot set MTU on port %u, (%d)%s", pinfo->pid, -ret, rte_strerror(-ret));
}

static void
_rx_queues(port_info_t *pinfo)
{
    struct rte_eth_dev_info *dinfo = &pinfo->dev_info;
    struct rte_eth_conf *conf      = &pinfo->conf;
    uint16_t sid, pid = pinfo->pid;
    int ret;

    l2p_port_t *lport = l2p_get_port(pid);
    if (lport == NULL)
        pktgen_log_panic("Failed: l2p_port_t for port %u not found", pid);

    sid = pg_eth_dev_socket_id(pid);

    pktgen_log_info("   Number of RX queues %u", l2p_get_rxcnt(pid));
    for (int q = 0; q < l2p_get_rxcnt(pid); q++) {
        struct rte_eth_rxconf rxq_conf;

        rxq_conf          = dinfo->default_rxconf;
        rxq_conf.offloads = conf->rxmode.offloads;

        pktgen_log_info("     RX queue %d enabled offloads: 0x%0lx, socket_id %u, mp %p", q,
                        rxq_conf.offloads, sid, lport->rx_mp[q]);

        ret = rte_eth_rx_queue_setup(pid, q, pktgen.nb_rxd, sid, &rxq_conf, lport->rx_mp[q]);
        if (ret < 0)
            pktgen_log_panic("rte_eth_rx_queue_setup: err=%d, port=%d, %s", ret, pid,
                             rte_strerror(-ret));
    }
}

static void
_tx_queues(port_info_t *pinfo)
{
    struct rte_eth_dev_info *dinfo = &pinfo->dev_info;
    struct rte_eth_conf *conf      = &pinfo->conf;
    uint16_t sid, pid = pinfo->pid;
    int ret;

    l2p_port_t *lport = l2p_get_port(pid);
    if (lport == NULL)
        pktgen_log_panic("Failed: l2p_port_t for port %u not found", pid);

    sid = pg_eth_dev_socket_id(pid);

    pktgen_log_info("   Number of TX queues %u", l2p_get_txcnt(pid));
    for (int q = 0; q < l2p_get_txcnt(pid); q++) {
        struct rte_eth_txconf txq_conf;

        txq_conf          = dinfo->default_txconf;
        txq_conf.offloads = conf->txmode.offloads;

        pktgen_log_info("     TX queue %d enabled offloads: 0x%0lx", q, txq_conf.offloads);

        ret = rte_eth_tx_queue_setup(pid, q, pktgen.nb_txd, sid, &txq_conf);
        if (ret < 0)
            pktgen_log_panic("rte_eth_tx_queue_setup: err=%d, port=%d, %s", ret, pid,
                             rte_strerror(-ret));
    }
}

static void
_promiscuous_mode(port_info_t *pinfo)
{
    /* If enabled, put device in promiscuous mode. */
    if (pktgen.flags & PROMISCUOUS_ON_FLAG) {
        pktgen_log_info("   Enabling promiscuous mode");
        if (rte_eth_promiscuous_enable(pinfo->pid))
            pktgen_log_info("Enabling promiscuous failed: %s", rte_strerror(-rte_errno));
    }
}

static void
_debug_output(port_info_t *pinfo)
{
    if (pktgen.verbose)
        pktgen_log_info("%*sPort memory used = %6lu KB", 57, " ", (pktgen.mem_used + 1023) / 1024);

    if (pktgen.verbose)
        rte_eth_dev_info_dump(stderr, pinfo->pid);
}

static void
_device_start(port_info_t *pinfo)
{
    int ret;

    pktgen_log_info("   Start network device");

    /* Start device */
    if ((ret = rte_eth_dev_start(pinfo->pid)) < 0)
        pktgen_log_panic("rte_eth_dev_start: port=%d, %s", pinfo->pid, rte_strerror(-ret));
}

static void
_port_defaults(port_info_t *pinfo)
{
    pktgen_log_info("   Setup port defaults");

    pktgen_port_defaults(pinfo->pid);
}

static port_info_t *
initialize_port_info(uint16_t pid)
{
    port_info_t *pinfo = l2p_get_port_pinfo(pid);
    // clang-format off
    void (*setups[])(port_info_t *) = {
        _latency_defaults,
        _fill_pattern_defaults,
        _mtu_defaults,
        _rx_offload_defaults,
        _tx_offload_defaults,
        _device_configuration,
        _rxtx_descriptors,
        _src_mac_address,
        _device_ptypes,
        _device_mtu,
        _rx_queues,
        _tx_queues,
        _debug_output,
        _promiscuous_mode,
        _port_defaults,
        _device_start,
    };
    // clang-format on

    if ((pinfo = allocate_port_info(pid)) == NULL)
        pktgen_log_panic("Unable to allocate port_info_t for port %u", pid);

    for (uint16_t i = 0; i < RTE_DIM(setups); i++) {
        if (setups[i])
            setups[i](pinfo);
    }

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
