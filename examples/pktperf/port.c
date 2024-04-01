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

    .rx_adv_conf =
        {
            .rss_conf =
                {
                    .rss_key = NULL,
                    .rss_hf = RTE_ETH_RSS_IP | RTE_ETH_RSS_TCP | RTE_ETH_RSS_UDP | RTE_ETH_RSS_SCTP,
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

int
port_setup(l2p_port_t *port)
{
    uint16_t pid;
    struct rte_eth_rxconf rxq_conf;
    struct rte_eth_txconf txq_conf;
    struct rte_eth_conf local_port_conf = port_conf;
    struct rte_eth_dev_info dev_info;

    if (!port)
        rte_exit(EXIT_FAILURE, "%s: port is NULL\n", __func__);

    pid = port->pid;

    port->mtu_size = RTE_ETHER_MTU;

    if (rte_atomic16_cmpset(&port->inited.cnt, 0, 1) > 0) {
        int ret;

        DBG_PRINT("Initializing port %u\n", pid);

        ret = rte_eth_dev_info_get(pid, &dev_info);
        if (ret != 0)
            rte_exit(EXIT_FAILURE, "Error during getting device (port %u) info: %s\n", pid,
                     strerror(-ret));
        DBG_PRINT("Driver: %s\n", dev_info.driver_name);

        if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
            local_port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

        DBG_PRINT("Port %u configure with %u:%u queues\n", pid, port->num_rx_qids,
                  port->num_tx_qids);

        local_port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
        local_port_conf.rx_adv_conf.rss_conf.rss_hf &= dev_info.flow_type_rss_offloads;
        if (dev_info.max_rx_queues == 1)
            local_port_conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;

        DBG_PRINT("Port %u configure with mode %" PRIx64 "\n", pid,
                  local_port_conf.rx_adv_conf.rss_conf.rss_hf);

        if (dev_info.max_vfs) {
            if (local_port_conf.rx_adv_conf.rss_conf.rss_hf != 0)
                local_port_conf.rxmode.mq_mode = RTE_ETH_MQ_RX_VMDQ_RSS;
        }
        DBG_PRINT("Port %u configure with mode %u\n", pid, local_port_conf.rxmode.mq_mode);

        /* Configure the number of queues for a port. */
        ret = rte_eth_dev_configure(pid, port->num_rx_qids, port->num_tx_qids, &local_port_conf);
        if (ret < 0)
            rte_exit(EXIT_FAILURE, "Can't configure device: err=%d, port=%u\n", ret, pid);

        ret = rte_eth_dev_adjust_nb_rx_tx_desc(pid, &info->nb_rxd, &info->nb_txd);
        if (ret < 0)
            rte_exit(EXIT_FAILURE, "Can't adjust number of descriptors: port=%u:%s\n", pid,
                     rte_strerror(-ret));

        if ((ret = rte_eth_macaddr_get(pid, &port->mac_addr)) < 0)
            rte_exit(EXIT_FAILURE, "Can't get MAC address: err=%d, port=%u\n", ret, pid);

        DBG_PRINT("Port %u MAC address: " RTE_ETHER_ADDR_PRT_FMT "\n", pid,
                  RTE_ETHER_ADDR_BYTES(&port->mac_addr));

        rxq_conf          = dev_info.default_rxconf;
        rxq_conf.offloads = local_port_conf.rxmode.offloads;

        ret = rte_eth_dev_set_ptypes(pid, RTE_PTYPE_UNKNOWN, NULL, 0);
        if (ret < 0)
            rte_exit(EXIT_FAILURE, "Port %u, Failed to disable Ptype parsing\n", pid);
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
            rte_exit(EXIT_FAILURE, "Cannot set MTU %u on port %u, (%d)%s", port->mtu_size, pid,
                     -ret, rte_strerror(-ret));

        DBG_PRINT("Port %u Rx/Tx queues %u/%u\n", pid, port->num_rx_qids, port->num_tx_qids);

        /* Setup Rx/Tx Queues */
        for (int q = 0; q < port->num_rx_qids; q++) {
            uint32_t sid = rte_eth_dev_socket_id(pid);

            ret = rte_eth_rx_queue_setup(pid, q, info->nb_rxd, sid, &rxq_conf, port->rx_mp);
            if (ret < 0)
                rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n", ret, pid);
            DBG_PRINT("Port %u:%u configured with %u RX descriptors\n", pid, q, info->nb_rxd);

            txq_conf          = dev_info.default_txconf;
            txq_conf.offloads = local_port_conf.txmode.offloads;

            ret = rte_eth_tx_queue_setup(pid, q, info->nb_txd, sid, &txq_conf);
            if (ret < 0)
                rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n", ret, pid);
            DBG_PRINT("Port %u:%u configured with %u TX descriptors\n", pid, q, info->nb_txd);
        }

        if (info->promiscuous_on) {
            ret = rte_eth_promiscuous_enable(pid);
            if (ret != 0)
                rte_exit(EXIT_FAILURE, "rte_eth_promiscuous_enable:err=%s, port=%u\n",
                         rte_strerror(-ret), pid);
            DBG_PRINT("Port %u promiscuous mode enabled\n", pid);
        }
        DBG_PRINT("Port %u promiscuous mode is '%s'\n", pid, info->promiscuous_on ? "on" : "off");

        /* Start device */
        ret = rte_eth_dev_start(pid);
        if (ret < 0)
            rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n", ret, pid);
        DBG_PRINT("Port %u started\n", pid);
    }
    DBG_PRINT("Port %u initialized\n", pid);

    return 0;
}
