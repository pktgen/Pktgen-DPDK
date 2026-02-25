/*-
 * Copyright(c) <2012-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#ifndef __L2P_H
#define __L2P_H

/**
 * @file
 *
 * Lcore-to-Port (L2P) mapping for Pktgen.
 *
 * Manages the assignment of logical cores to Ethernet port/queue pairs,
 * the per-port mempool ownership, and provides accessors for obtaining
 * port_info_t pointers, queue counts, and PCAP state from any lcore or
 * port index.
 */

#include <string.h>
#include <pthread.h>

#include <rte_memory.h>
#include <rte_atomic.h>

#include <pktgen-pcap.h>
#include <pktgen-log.h>
#include <pktgen-stats.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MATRIX_ENTRIES 128 /**< Maximum number of lcore/port mapping entries */
#define MAX_STRING         256 /**< Maximum length of a mapping string */

#define MAX_MAP_PORTS  (RTE_MAX_ETHPORTS + 1) /**< Sentinel-inclusive port array size */
#define MAX_MAP_LCORES (RTE_MAX_LCORE + 1)    /**< Sentinel-inclusive lcore array size */

/** Lcore operating mode: unknown, RX-only, TX-only, or both. */
enum { LCORE_MODE_UNKNOWN = 0, LCORE_MODE_RX = 1, LCORE_MODE_TX = 2, LCORE_MODE_BOTH = 3 };

#define MAX_MAPPINGS       32   /**< Maximum number of port/queue mappings */
#define MAX_ALLOCA_SIZE    1024 /**< Maximum size of a stack allocation */
#define MEMPOOL_CACHE_SIZE (RTE_MEMPOOL_CACHE_MAX_SIZE / 2) /**< Per-lcore mempool cache size */

struct port_info_s;

/**
 * Per-port L2P state: mempools, queue counts, and PCAP context.
 */
typedef struct l2p_port_s {
    struct port_info_s *pinfo;                      /**< Port information pointer */
    uint16_t pid;                                   /**< Port ID attached to lcore */
    uint16_t num_rx_qids;                           /**< Number of Rx queues */
    uint16_t num_tx_qids;                           /**< Number of Tx queues */
    struct rte_mempool *rx_mp[MAX_QUEUES_PER_PORT]; /**< Rx pktmbuf pool per queue */
    struct rte_mempool *tx_mp[MAX_QUEUES_PER_PORT]; /**< Tx pktmbuf pool per queue */
    struct rte_mempool *sp_mp[MAX_QUEUES_PER_PORT]; /**< Special TX pktmbuf pool per queue */
    pcap_info_t *pcap_info;                         /**< PCAP packet structure */
} l2p_port_t;

/**
 * Per-lcore L2P state: mode, queue IDs, and a pointer to the owning port.
 */
typedef struct l2p_lport_s {
    uint16_t mode;    /**< LCORE_MODE_RX, LCORE_MODE_TX, or LCORE_MODE_BOTH */
    uint16_t lid;     /**< Lcore ID */
    uint16_t rx_qid;  /**< RX queue ID assigned to this lcore */
    uint16_t tx_qid;  /**< TX queue ID assigned to this lcore */
    l2p_port_t *port; /**< Pointer to the owning port structure */
} l2p_lport_t;

/**
 * Top-level L2P mapping table.
 */
typedef struct {
    l2p_lport_t *lports[RTE_MAX_LCORE]; /**< Per-lcore port/queue assignment */
    l2p_port_t ports[RTE_MAX_ETHPORTS]; /**< Per-port L2P state */
    char *mappings[RTE_MAX_ETHPORTS];   /**< Raw mapping strings from the command line */
    uint16_t num_mappings;              /**< Number of mapping strings */
    uint16_t num_ports;                 /**< Number of active ports */
} l2p_t;

/** Log an error message with the enclosing function name as prefix. */
#define L2P_ERR(fmt, args...) printf("%s: " fmt "\n", __func__, ##args);

/** Log an error message and return -1. */
#define L2P_ERR_RET(fmt, args...) \
    do {                          \
        L2P_ERR(fmt, ##args);     \
        return -1;                \
    } while (0)

/** Log an error message and return NULL. */
#define L2P_NULL_RET(fmt, args...) \
    do {                           \
        L2P_ERR(fmt, ##args);      \
        return NULL;               \
    } while (0)

/**
 * Return the global l2p_t singleton.
 *
 * @return
 *   Pointer to the global l2p_t mapping table.
 */
l2p_t *l2p_get(void);

/**
 * Return the l2p_port_t for a given port ID.
 *
 * @param pid
 *   Port ID to look up.
 * @return
 *   Pointer to the l2p_port_t, or NULL if @p pid is invalid.
 */
static __inline__ l2p_port_t *
l2p_get_port(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    return ((pid < RTE_MAX_ETHPORTS) && (l2p->ports[pid].pid < RTE_MAX_ETHPORTS)) ? &l2p->ports[pid]
                                                                                  : NULL;
}

/**
 * Return the number of active ports in the L2P mapping.
 *
 * @return
 *   Number of configured ports.
 */
static __inline__ uint16_t
l2p_get_port_cnt(void)
{
    l2p_t *l2p = l2p_get();

    return l2p->num_ports;
}

/**
 * Return the lcore ID associated with a port.
 *
 * Scans the lport table and returns the first lcore mapped to @p pid.
 *
 * @param pid
 *   Port ID to search for.
 * @return
 *   Lcore ID, or RTE_MAX_LCORE if none found.
 */
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

/**
 * Return the port ID assigned to a given lcore.
 *
 * @param lid
 *   Lcore ID to look up.
 * @return
 *   Port ID, or RTE_MAX_ETHPORTS if @p lid has no port assignment.
 */
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

/**
 * Return the RX mempool for a given port and queue.
 *
 * @param pid
 *   Port ID.
 * @param qid
 *   RX queue ID.
 * @return
 *   Pointer to the RX mempool, or NULL on error.
 */
static __inline__ struct rte_mempool *
l2p_get_rx_mp(uint16_t pid, uint16_t qid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS)
        L2P_NULL_RET("Invalid port ID %u", pid);

    if (qid >= MAX_QUEUES_PER_PORT)
        L2P_NULL_RET("Invalid queue ID %u", qid);

    return l2p->ports[pid].rx_mp[qid];
}

/**
 * Return the TX mempool for a given port and queue.
 *
 * @param pid
 *   Port ID.
 * @param qid
 *   TX queue ID.
 * @return
 *   Pointer to the TX mempool, or NULL on error.
 */
static __inline__ struct rte_mempool *
l2p_get_tx_mp(uint16_t pid, uint16_t qid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS)
        L2P_NULL_RET("Invalid port ID %u", pid);

    if (qid >= MAX_QUEUES_PER_PORT)
        L2P_NULL_RET("Invalid queue ID %u", qid);

    return l2p->ports[pid].tx_mp[qid];
}

/**
 * Return the special TX mempool for a given port and queue.
 *
 * @param pid
 *   Port ID.
 * @param qid
 *   TX queue ID.
 * @return
 *   Pointer to the special TX mempool, or NULL on error.
 */
static __inline__ struct rte_mempool *
l2p_get_sp_mp(uint16_t pid, uint16_t qid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS)
        L2P_NULL_RET("Invalid port ID %u", pid);

    if (qid >= MAX_QUEUES_PER_PORT)
        L2P_NULL_RET("Invalid queue ID %u", qid);

    return l2p->ports[pid].sp_mp[qid];
}

/**
 * Return the lcore operating mode (RX, TX, or both).
 *
 * @param lid
 *   Lcore ID to query.
 * @return
 *   LCORE_MODE_* constant, or -1 on error.
 */
static __inline__ int
l2p_get_type(uint16_t lid)
{
    l2p_t *l2p = l2p_get();

    if (lid >= RTE_MAX_LCORE)
        L2P_ERR_RET("Invalid lcore ID %u", lid);

    return l2p->lports[lid]->mode;
}

/**
 * Return the port_info_t for a given port ID.
 *
 * @param pid
 *   Port ID to look up.
 * @return
 *   Pointer to port_info_t, or NULL if @p pid is invalid.
 */
static __inline__ struct port_info_s *
l2p_get_port_pinfo(uint16_t pid)
{
    l2p_port_t *port = l2p_get_port(pid);

    return (port == NULL) ? NULL : port->pinfo;
}

/**
 * Set the port_info_t pointer for a given port ID.
 *
 * @param pid
 *   Port ID to update.
 * @param pinfo
 *   Pointer to the port_info_t to associate.
 * @return
 *   0 on success, -1 on invalid @p pid.
 */
static __inline__ int
l2p_set_port_pinfo(uint16_t pid, struct port_info_s *pinfo)
{
    l2p_port_t *port = l2p_get_port(pid);

    if (port == NULL)
        L2P_ERR_RET("Invalid port ID %u", pid);

    port->pinfo = pinfo;
    return 0;
}

/**
 * Return the port_info_t for the port assigned to a given lcore.
 *
 * @param lid
 *   Lcore ID to look up.
 * @return
 *   Pointer to port_info_t, or NULL if @p lid is invalid.
 */
static __inline__ struct port_info_s *
l2p_get_pinfo_by_lcore(uint16_t lid)
{
    l2p_t *l2p = l2p_get();
    l2p_port_t *port;

    if (lid >= RTE_MAX_LCORE || l2p->lports[lid]->lid >= RTE_MAX_LCORE ||
        (port = l2p->lports[lid]->port) == NULL)
        L2P_NULL_RET("Invalid lcore ID %u", lid);

    return port->pinfo;
}

/**
 * Set the port_info_t pointer for the port assigned to a given lcore.
 *
 * @param lid
 *   Lcore ID to update.
 * @param pinfo
 *   Pointer to the port_info_t to associate.
 * @return
 *   0 on success, -1 on invalid @p lid.
 */
static __inline__ int
l2p_set_pinfo_by_lcore(uint16_t lid, struct port_info_s *pinfo)
{
    l2p_t *l2p = l2p_get();
    l2p_port_t *port;

    if (lid >= RTE_MAX_LCORE || l2p->lports[lid]->lid >= RTE_MAX_LCORE ||
        (port = l2p->lports[lid]->port) == NULL)
        L2P_ERR_RET("Invalid lcore ID %u", lid);

    port->pinfo = pinfo;

    return 0;
}

/**
 * Return the RX and TX queue counts for a port.
 *
 * @param pid
 *   Port ID to query.
 * @param rx_qid
 *   Output: number of RX queues (may be NULL).
 * @param tx_qid
 *   Output: number of TX queues (may be NULL).
 * @return
 *   0 on success, -1 on invalid @p pid.
 */
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

/**
 * Return the number of RX queues for a port.
 *
 * @param pid
 *   Port ID to query.
 * @return
 *   RX queue count, or 0 on invalid @p pid.
 */
static __inline__ int
l2p_get_rxcnt(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS)
        return l2p->ports[pid].num_rx_qids;
    return 0;
}

/**
 * Return the number of TX queues for a port.
 *
 * @param pid
 *   Port ID to query.
 * @return
 *   TX queue count, or 0 on invalid @p pid.
 */
static __inline__ int
l2p_get_txcnt(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS)
        return l2p->ports[pid].num_tx_qids;
    return 0;
}

/**
 * Return the RX queue ID assigned to a given lcore.
 *
 * @param lid
 *   Lcore ID to query.
 * @return
 *   RX queue ID, or 0 on invalid @p lid.
 */
static __inline__ int
l2p_get_rxqid(uint16_t lid)
{
    l2p_t *l2p = l2p_get();

    if (lid < RTE_MAX_LCORE && l2p->lports[lid] != NULL && l2p->lports[lid]->port != NULL)
        return l2p->lports[lid]->rx_qid;
    return 0;
}

/**
 * Return the TX queue ID assigned to a given lcore.
 *
 * @param lid
 *   Lcore ID to query.
 * @return
 *   TX queue ID, or 0 on invalid @p lid.
 */
static __inline__ int
l2p_get_txqid(uint16_t lid)
{
    l2p_t *l2p = l2p_get();

    if (lid < RTE_MAX_LCORE && l2p->lports[lid] != NULL && l2p->lports[lid]->port != NULL)
        return l2p->lports[lid]->tx_qid;
    return 0;
}

/**
 * Return the RX and TX queue IDs for a port (via the first mapped lcore).
 *
 * @param pid
 *   Port ID to query.
 * @param rx_qid
 *   Output: RX queue ID (may be NULL).
 * @param tx_qid
 *   Output: TX queue ID (may be NULL).
 * @return
 *   0 on success, -1 on invalid @p pid.
 */
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

/**
 * Return the PCAP info structure for a port.
 *
 * @param pid
 *   Port ID to look up.
 * @return
 *   Pointer to pcap_info_t, or NULL on invalid @p pid.
 */
static __inline__ pcap_info_t *
l2p_get_pcap(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid < RTE_MAX_ETHPORTS && l2p->ports[pid].pid < RTE_MAX_ETHPORTS)
        return l2p->ports[pid].pcap_info;
    L2P_NULL_RET("Invalid port ID %u", pid);
}

/**
 * Set the PCAP info pointer for a port.
 *
 * @param pid
 *   Port ID to update.
 * @param pcap_info
 *   PCAP info structure to associate with the port.
 * @return
 *   0 on success, -1 on invalid @p pid.
 */
static __inline__ int
l2p_set_pcap_info(uint16_t pid, pcap_info_t *pcap_info)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS || l2p->ports[pid].pid >= RTE_MAX_ETHPORTS)
        L2P_ERR_RET("Invalid port ID %u", pid);
    l2p->ports[pid].pcap_info = pcap_info;
    return 0;
}

/**
 * Return the PCAP mempool for a port.
 *
 * @param pid
 *   Port ID to look up.
 * @return
 *   Pointer to the PCAP mempool, or NULL on error.
 */
static __inline__ struct rte_mempool *
l2p_get_pcap_mp(uint16_t pid)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS || l2p->ports[pid].pid >= RTE_MAX_ETHPORTS ||
        l2p->ports[pid].pcap_info == NULL)
        L2P_NULL_RET("Invalid port ID %u", pid);

    return l2p->ports[pid].pcap_info->mp;
}

/**
 * Set the PCAP mempool for a port.
 *
 * @param pid
 *   Port ID to update.
 * @param mp
 *   Mempool to associate with the port's PCAP state.
 * @return
 *   0 on success, -1 on invalid @p pid.
 */
static __inline__ int
l2p_set_pcap_mp(uint16_t pid, struct rte_mempool *mp)
{
    l2p_t *l2p = l2p_get();

    if (pid >= RTE_MAX_ETHPORTS || l2p->ports[pid].pid >= RTE_MAX_ETHPORTS)
        L2P_ERR_RET("Invalid port ID %u", pid);

    l2p->ports[pid].pcap_info->mp = mp;
    return 0;
}

/**
 * Allocate and initialise the global l2p_t mapping table.
 */
void l2p_create(void);

/**
 * Parse all registered mapping strings and populate the l2p_t table.
 *
 * @return
 *   0 on success, negative on parse error.
 */
int l2p_parse_mappings(void);

/**
 * Register a mapping string for later parsing.
 *
 * @param str
 *   Mapping string in the form expected by the -m argument parser.
 * @return
 *   0 on success, negative on error.
 */
int l2p_parse_mapping_add(const char *str);

/**
 * Parse a hex portmask string into a bitmask of port indices.
 *
 * @param portmask
 *   Null-terminated hex string (e.g. "0x3" selects ports 0 and 1).
 * @return
 *   Bitmask of selected port IDs.
 */
uint32_t l2p_parse_portmask(const char *portmask);

#ifdef __cplusplus
}
#endif

#endif /* __L2P_H */
