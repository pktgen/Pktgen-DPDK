/*-
 * Copyright(c) <2012-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#ifndef __L2P_H
#define __L2P_H

#include <string.h>
#include <pthread.h>

#include <rte_memory.h>
#include <rte_atomic.h>

#include <pktgen-pcap.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MATRIX_ENTRIES 128
#define MAX_STRING         256

#define MAX_MAP_PORTS  (RTE_MAX_ETHPORTS + 1)
#define MAX_MAP_LCORES (RTE_MAX_LCORE + 1)

enum { LCORE_MODE_UNKNOWN = 0, LCORE_MODE_RX = 1, LCORE_MODE_TX = 2, LCORE_MODE_BOTH = 3 };

#define MAX_MAPPINGS       32                               /* Max number of mappings */
#define MAX_ALLOCA_SIZE    1024                             /* Maximum size of an allocation */
#define MEMPOOL_CACHE_SIZE (RTE_MEMPOOL_CACHE_MAX_SIZE / 2) /* Size of mempool cache */

struct port_info_s;

typedef struct l2p_port_s {
    pthread_spinlock_t lock;                       /* Lock for this port */
    struct port_info_s *pinfo;                     /* Port information pointer */
    uint16_t pid;                                  /* Port ID attached to lcore */
    uint16_t num_rx_qids;                          /* Number of Rx queues */
    uint16_t num_tx_qids;                          /* Number of Tx queues */
    struct rte_mempool *rx_mp __rte_cache_aligned; /* Rx pktmbuf mempool per queue */
    struct rte_mempool *tx_mp;                     /* Tx pktmbuf mempool per queue */
    struct rte_mempool *special_mp;                /* Pool pointer for special TX mbufs */
    pcap_info_t *pcap_info;                        /* PCAP packet structure */
} l2p_port_t;

typedef struct l2p_lport_s { /* Each lcore has one port/queue attached */
    uint16_t mode;           /* TXPKTS_MODE_RX or TXPKTS_MODE_TX or BOTH */
    uint16_t lid;            /* Lcore ID */
    uint16_t rx_qid;         /* Queue ID attached to Rx lcore */
    uint16_t tx_qid;         /* Queue ID attached to Tx lcore */
    l2p_port_t *port;        /* Port structure */
} l2p_lport_t;

typedef struct {
    l2p_lport_t *lports[RTE_MAX_LCORE]; /* Array of lcore/port structure pointers */
    l2p_port_t ports[RTE_MAX_ETHPORTS]; /* Array of port structures */
    char *mappings[RTE_MAX_ETHPORTS];   /* Array of string port/queue mappings */
    uint16_t num_mappings;              /* Number of mapping strings */
    uint16_t num_ports;                 /* Number of ports */
} l2p_t;

l2p_t *l2p_get(void);

static __inline__ l2p_port_t *
l2p_get_port(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if ((pid < RTE_MAX_ETHPORTS) && (l2p->ports[pid].pid < RTE_MAX_ETHPORTS))
        return &l2p->ports[pid];
    else
        return NULL;
}

static __inline__ uint16_t
l2p_get_port_cnt(void)
{
    l2p_t *l2p = l2p_get();

    return l2p->num_ports;
}

static __inline__ uint16_t
l2p_get_lcore_by_pid(uint16_t pid)
{
    l2p_t *l2p   = l2p_get();
    uint16_t lid = RTE_MAX_LCORE;

    for (uint16_t i = 0; i < RTE_MAX_LCORE; i++) {
        if (l2p->lports[i] && l2p->lports[i]->port->pid == pid) {
            lid = l2p->lports[i]->lid;
            break;
        }
    }
    return lid;
}

static __inline__ uint16_t
l2p_get_pid_by_lcore(uint16_t lid)
{
    l2p_t *l2p   = l2p_get();
    uint16_t pid = RTE_MAX_ETHPORTS;

    if (!l2p->lports[lid] || !l2p->lports[lid]->port)
        return pid;
    if (l2p->lports[lid]->port->pid >= RTE_MAX_ETHPORTS)
        return pid;

    return l2p->lports[lid]->port->pid;
}

static __inline__ struct rte_mempool *
l2p_get_rx_mp(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS)
        return NULL;

    return l2p->ports[pid].rx_mp;
}

static __inline__ struct rte_mempool *
l2p_get_tx_mp(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS)
        return NULL;

    return l2p->ports[pid].tx_mp;
}

static __inline__ struct rte_mempool *
l2p_get_special_mp(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS)
        return NULL;

    return l2p->ports[pid].special_mp;
}

static __inline__ int
l2p_get_type(uint16_t lid)
{
    l2p_t *l2p = l2p_get();

    if (lid >= RTE_MAX_LCORE)
        return -1;
    return l2p->lports[lid]->mode;
}

static __inline__ struct port_info_s *
l2p_get_port_pinfo(uint16_t pid)
{
    l2p_port_t *port = l2p_get_port(pid);

    if (port == NULL)
        return NULL;

    return port->pinfo;
}

static __inline__ int
l2p_set_port_pinfo(uint16_t pid, struct port_info_s *pinfo)
{
    l2p_port_t *port = l2p_get_port(pid);

    if (port == NULL)
        return -1;

    port->pinfo = pinfo;
    return 0;
}

static __inline__ struct port_info_s *
l2p_get_pinfo_by_lcore(uint16_t lid)
{
    l2p_t *l2p = l2p_get();
    l2p_port_t *port;

    if (lid >= RTE_MAX_LCORE || l2p->lports[lid]->lid >= RTE_MAX_LCORE ||
        (port = l2p->lports[lid]->port) == NULL)
        return NULL;

    return port->pinfo;
}

static __inline__ int
l2p_set_pinfo_by_lcore(uint16_t lid, struct port_info_s *pinfo)
{
    l2p_t *l2p = l2p_get();
    l2p_port_t *port;

    if (lid >= RTE_MAX_LCORE || l2p->lports[lid]->lid >= RTE_MAX_LCORE ||
        (port = l2p->lports[lid]->port) == NULL)
        return -1;

    port->pinfo = pinfo;

    return 0;
}

static __inline__ int
l2p_get_qcnt(uint16_t pid, uint16_t *rx_qid, uint16_t *tx_qid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS) {
        if (rx_qid)
            *rx_qid = l2p->ports[pid].num_rx_qids;
        if (tx_qid)
            *tx_qid = l2p->ports[pid].num_tx_qids;
        return 0;
    }
    return -1;
}

static __inline__ int
l2p_get_rxcnt(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS)
        return l2p->ports[pid].num_rx_qids;
    return 0;
}

static __inline__ int
l2p_get_txcnt(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS)
        return l2p->ports[pid].num_tx_qids;
    return 0;
}

static __inline__ int
l2p_get_rxqid(uint16_t lid)
{
    l2p_t *l2p = l2p_get();

    if (lid < RTE_MAX_LCORE && l2p->lports[lid] != NULL && l2p->lports[lid]->port != NULL)
        return l2p->lports[lid]->rx_qid;
    return 0;
}

static __inline__ int
l2p_get_txqid(uint16_t lid)
{
    l2p_t *l2p = l2p_get();

    if (lid < RTE_MAX_LCORE && l2p->lports[lid] != NULL && l2p->lports[lid]->port != NULL)
        return l2p->lports[lid]->tx_qid;
    return 0;
}

static __inline__ int
l2p_get_qids(uint16_t pid, uint16_t *rx_qid, uint16_t *tx_qid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS) {
        if (rx_qid)
            *rx_qid = l2p_get_rxqid(pid);
        if (tx_qid)
            *tx_qid = l2p_get_txqid(pid);
        return 0;
    }
    return -1;
}

static __inline__ pcap_info_t *
l2p_get_pcap(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS)
        return l2p->ports[pid].pcap_info;
    return NULL;
}

static __inline__ int
l2p_set_pcap_info(uint16_t pid, pcap_info_t *pcap_info)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS || l2p->ports[pid].pid >= RTE_MAX_ETHPORTS)
        return -1;
    l2p->ports[pid].pcap_info = pcap_info;
    return 0;
}

static __inline__ struct rte_mempool *
l2p_get_pcap_mp(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS || l2p->ports[pid].pid >= RTE_MAX_ETHPORTS ||
        l2p->ports[pid].pcap_info == NULL)
        return NULL;

    return l2p->ports[pid].pcap_info->mp;
}

static __inline__ int
l2p_set_pcap_mp(uint16_t pid, struct rte_mempool *mp)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS || l2p->ports[pid].pid >= RTE_MAX_ETHPORTS)
        return -1;

    l2p->ports[pid].pcap_info->mp = mp;
    return 0;
}

void l2p_create(void);

int l2p_parse_mappings(void);
int l2p_parse_mapping_add(const char *str);
uint32_t l2p_parse_portmask(const char *portmask);

#ifdef __cplusplus
}
#endif

#endif /* __L2P_H */
