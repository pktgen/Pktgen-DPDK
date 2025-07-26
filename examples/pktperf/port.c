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

#include <pktperf.h>

static struct rte_eth_conf port_conf = {
    .rxmode =
        {
            .mq_mode          = RTE_ETH_MQ_RX_RSS,
            .max_lro_pkt_size = RTE_ETHER_MAX_LEN,
            .offloads         = RTE_ETH_RX_OFFLOAD_CHECKSUM,
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
                    .rss_hf = RTE_ETH_RSS_IP | RTE_ETH_RSS_TCP | RTE_ETH_RSS_UDP | RTE_ETH_RSS_SCTP,
                },
        },
    .intr_conf =
        {
            .lsc = 0,
        },
};

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

int
port_setup(l2p_port_t *port)
{
    uint16_t pid;
    struct rte_eth_rxconf rxq_conf;
    struct rte_eth_txconf txq_conf;
    struct rte_eth_conf conf = port_conf;
    struct rte_eth_dev_info dev_info;

    if (!port)
        ERR_RET("%s: port is NULL\n", __func__);

    pid = port->pid;

    port->mtu_size = RTE_ETHER_MTU;

    if (rte_atomic16_cmpset(&port->inited.cnt, 0, 1) > 0) {
        int ret;

        DBG_PRINT("Initializing port %u\n", pid);

        ret = rte_eth_dev_info_get(pid, &dev_info);
        if (ret != 0)
            ERR_RET("Error during getting device (port %u) info: %s\n", pid, strerror(-ret));
        DBG_PRINT("Driver: %s\n", dev_info.driver_name);

        if (info->jumbo_frame_on) {
            uint32_t eth_overhead_len;
            uint32_t max_mtu;

            conf.rxmode.max_lro_pkt_size = JUMBO_ETHER_MTU;
            eth_overhead_len = eth_dev_get_overhead_len(dev_info.max_rx_pktlen, dev_info.max_mtu);
            max_mtu          = dev_info.max_mtu - eth_overhead_len;
            printf("Jumbo Frames enabled: Default Max Rx pktlen: %'u, MTU %'u, overhead len: %'u, "
                   "New MTU %'d\n",
                   dev_info.max_rx_pktlen, dev_info.max_mtu, eth_overhead_len, max_mtu);

            /* device may have higher theoretical MTU e.g. for infiniband */
            if (max_mtu > JUMBO_ETHER_MTU)
                max_mtu = JUMBO_ETHER_MTU;
            printf("Jumbo Frames enabled: Using Max MTU: %'d", max_mtu);

            conf.rxmode.mtu = max_mtu;
#if 0        // Tx performance takes a big hit when enabled
            if (dev_info.rx_offload_capa & RTE_ETH_RX_OFFLOAD_SCATTER)
                conf.rxmode.offloads |= RTE_ETH_RX_OFFLOAD_SCATTER;
            if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MULTI_SEGS)
                conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MULTI_SEGS;
#endif
        }

        if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
            conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

        DBG_PRINT("Port %u configure with %u:%u queues\n", pid, port->num_rx_qids,
                  port->num_tx_qids);

        conf.rx_adv_conf.rss_conf.rss_key = NULL;
        conf.rx_adv_conf.rss_conf.rss_hf &= dev_info.flow_type_rss_offloads;
        if (dev_info.max_rx_queues == 1)
            conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;

        DBG_PRINT("Port %u configure with mode %" PRIx64 "\n", pid,
                  conf.rx_adv_conf.rss_conf.rss_hf);

        if (dev_info.max_vfs) {
            if (conf.rx_adv_conf.rss_conf.rss_hf != 0)
                conf.rxmode.mq_mode = RTE_ETH_MQ_RX_VMDQ_RSS;
        }
        DBG_PRINT("Port %u configure with mode %u\n", pid, conf.rxmode.mq_mode);

        /* Configure the number of queues for a port. */
        ret = rte_eth_dev_configure(pid, port->num_rx_qids, port->num_tx_qids, &conf);
        if (ret < 0)
            ERR_RET("Can't configure device: err=%d, port=%u\n", ret, pid);

        ret = rte_eth_dev_adjust_nb_rx_tx_desc(pid, &info->nb_rxd, &info->nb_txd);
        if (ret < 0)
            ERR_RET("Can't adjust number of descriptors: port=%u:%s\n", pid, rte_strerror(-ret));

        if ((ret = rte_eth_macaddr_get(pid, &port->mac_addr)) < 0)
            ERR_RET("Can't get MAC address: err=%d, port=%u\n", ret, pid);

        DBG_PRINT("Port %u MAC address: " RTE_ETHER_ADDR_PRT_FMT "\n", pid,
                  RTE_ETHER_ADDR_BYTES(&port->mac_addr));

        rxq_conf          = dev_info.default_rxconf;
        rxq_conf.offloads = conf.rxmode.offloads;

        ret = rte_eth_dev_set_ptypes(pid, RTE_PTYPE_UNKNOWN, NULL, 0);
        if (ret < 0)
            ERR_RET("Port %u, Failed to disable Ptype parsing\n", pid);
        DBG_PRINT("Port %u configured with %08x Ptypes\n", pid, RTE_PTYPE_UNKNOWN);

        if (port->mtu_size < dev_info.min_mtu) {
            INFO_PRINT("Increasing MTU from %u to %u", port->mtu_size, dev_info.min_mtu);
            port->mtu_size     = dev_info.min_mtu;
            port->max_pkt_size = dev_info.min_mtu + RTE_ETHER_HDR_LEN;
        }
        if (port->mtu_size > dev_info.max_mtu) {
            INFO_PRINT("Reducing MTU from %u to %u", port->mtu_size, dev_info.max_mtu);
            port->mtu_size     = dev_info.max_mtu;
            port->max_pkt_size = dev_info.max_mtu + RTE_ETHER_HDR_LEN;
        }

        if ((ret = rte_eth_dev_set_mtu(pid, port->mtu_size)) < 0)
            ERR_RET("Cannot set MTU %u on port %u, (%d)%s", port->mtu_size, pid, -ret,
                    rte_strerror(-ret));

        DBG_PRINT("Port %u Rx/Tx queues %u/%u\n", pid, port->num_rx_qids, port->num_tx_qids);

        /* Setup Rx/Tx Queues */
        for (int q = 0; q < port->num_rx_qids; q++) {
            uint32_t sid = pg_eth_dev_socket_id(pid);

            ret = rte_eth_rx_queue_setup(pid, q, info->nb_rxd, sid, &rxq_conf, port->rx_mp);
            if (ret < 0)
                ERR_RET("rte_eth_rx_queue_setup:err=%d, port=%u\n", ret, pid);
            DBG_PRINT("Port %u:%u configured with %u RX descriptors\n", pid, q, info->nb_rxd);
        }
        for (int q = 0; q < port->num_tx_qids; q++) {
            uint32_t sid = pg_eth_dev_socket_id(pid);

            txq_conf          = dev_info.default_txconf;
            txq_conf.offloads = conf.txmode.offloads;

            ret = rte_eth_tx_queue_setup(pid, q, info->nb_txd, sid, &txq_conf);
            if (ret < 0)
                ERR_RET("rte_eth_tx_queue_setup:err=%d, port=%u\n", ret, pid);
            DBG_PRINT("Port %u:%u configured with %u TX descriptors\n", pid, q, info->nb_txd);
        }

        if (info->promiscuous_on) {
            ret = rte_eth_promiscuous_enable(pid);
            if (ret != 0)
                DBG_PRINT("INFO: rte_eth_promiscuous_enable:err=%s, port=%u\n", rte_strerror(-ret),
                          pid);
            else
                DBG_PRINT("Port %u promiscuous mode enabled\n", pid);
        }
        DBG_PRINT("Port %u promiscuous mode is '%s'\n", pid, info->promiscuous_on ? "on" : "off");

        /* Start device */
        ret = rte_eth_dev_start(pid);
        if (ret < 0)
            ERR_RET("rte_eth_dev_start:err=%d, port=%u\n", ret, pid);
        DBG_PRINT("Port %u started\n", pid);
    }
    DBG_PRINT("Port %u initialized\n", pid);

    return 0;
}
