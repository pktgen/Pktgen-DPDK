/*-
 * Copyright(c) <2012-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2012 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <alloca.h>
#include <pthread.h>

#include <rte_version.h>
#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_pci.h>
#include <rte_debug.h>
#include <rte_memory.h>

#include <pktgen.h>
#include <pg_strings.h>
#include "l2p.h"
#include "utils.h"

static l2p_t l2p_info, *l2p;

l2p_t *
l2p_get(void)
{
    return l2p;
}

/**
 * Create and initialize the lcore to port object
 *
 * RETURNS: Pointer to l2p_t or NULL on error.
 */
void
l2p_create(void)
{
    memset(&l2p_info, 0, sizeof(l2p_t));

    l2p = &l2p_info;
    for (int i = 0; i < RTE_MAX_ETHPORTS; i++) {
        l2p->ports[i].pid = RTE_MAX_ETHPORTS + 1;
        pthread_spin_init(&l2p->ports[i].lock, PTHREAD_PROCESS_PRIVATE);
    }
}

static struct rte_mempool *
l2p_pktmbuf_create(const char *type, l2p_lport_t *lport, l2p_port_t *port, int nb_mbufs,
                   int cache_size)
{
    struct rte_mempool *mp;
    char name[RTE_MEMZONE_NAMESIZE];
    uint64_t sz;
    int sid;

    sid = pg_eth_dev_socket_id(port->pid);

    snprintf(name, sizeof(name), "%s-L%u/P%u/S%u", type, lport->lid, port->pid, sid);

    const int bufSize = PG_JUMBO_FRAME_LEN;

    sz = nb_mbufs * bufSize;
    sz = RTE_ALIGN_CEIL(sz + sizeof(struct rte_mempool), 1024);

    pktgen.mem_used += sz;
    pktgen.total_mem_used += sz;

    /* create the mbuf pool */
    mp = rte_pktmbuf_pool_create(name, nb_mbufs, cache_size, DEFAULT_PRIV_SIZE, bufSize, sid);
    if (mp == NULL)
        rte_exit(EXIT_FAILURE,
                 "Cannot create mbuf pool (%s) port %d, queue %d, nb_mbufs %d, NUMA %d: %s\n", name,
                 port->pid, lport->rx_qid, nb_mbufs, sid, rte_strerror(rte_errno));

    pktgen_log_info("  Create: '%-*s' - Memory used (MBUFs %'6u x size %'6u) = %'8lu KB @ %p\n", 16,
                    name, nb_mbufs, bufSize, sz / 1024, mp);

    return mp;
}

static int
parse_cores(uint16_t pid, const char *cores, int mode)
{
    l2p_t *l2p       = l2p_get();
    l2p_port_t *port = &l2p->ports[pid];
    char *core_map   = NULL;
    int num_cores    = 0, l, h, num_fields;
    char *fields[3];
    char name[64];
    int mbuf_count = MAX_MBUFS_PER_PORT(1024, 1024);

    core_map = alloca(MAX_ALLOCA_SIZE);
    if (!core_map)
        rte_exit(EXIT_FAILURE, "out of memory for core string\n");

    snprintf(core_map, MAX_ALLOCA_SIZE, "%s", cores);

    num_fields = rte_strsplit(core_map, strlen(core_map), fields, RTE_DIM(fields), '-');
    if (num_fields <= 0 || num_fields > 2)
        rte_exit(EXIT_FAILURE, "invalid core mapping '%s'\n", cores);

    if (num_fields == 1) {
        l = h = strtol(fields[0], NULL, 10);
    } else if (num_fields == 2) {
        l = strtol(fields[0], NULL, 10);
        h = strtol(fields[1], NULL, 10);
    }

    do {
        l2p_lport_t *lport;

        lport = l2p->lports[l];
        if (lport == NULL) {
            snprintf(name, sizeof(name), "lport-%u:%u", l, port->pid);
            lport = rte_zmalloc_socket(name, sizeof(l2p_lport_t), RTE_CACHE_LINE_SIZE,
                                       pg_eth_dev_socket_id(port->pid));
            if (!lport)
                rte_exit(EXIT_FAILURE, "Failed to allocate memory for lport info\n");
            lport->lid = l;

            l2p->lports[l] = lport;
        } else
            printf("Err: lcore %u already in use\n", l);

        num_cores++;
        lport->port = port;
        lport->mode = mode;
        switch (mode) {
        case LCORE_MODE_RX:
            lport->rx_qid = port->num_rx_qids++;
            break;
        case LCORE_MODE_TX:
            lport->tx_qid = port->num_tx_qids++;
            break;
        case LCORE_MODE_BOTH:
            lport->rx_qid = port->num_rx_qids++;
            lport->tx_qid = port->num_tx_qids++;
            break;
        default:
            rte_exit(EXIT_FAILURE, "invalid port mode\n");
            break;
        }

        if (port->rx_mp == NULL) {
            /* Create the Rx mbuf pool one per lcore/port/queue */
            port->rx_mp = l2p_pktmbuf_create("RX", lport, port, mbuf_count, MEMPOOL_CACHE_SIZE);
            if (port->rx_mp == NULL)
                rte_exit(EXIT_FAILURE, "Cannot init port %d for Default RX mbufs", port->pid);
        }
        if (port->tx_mp == NULL) {
            port->tx_mp = l2p_pktmbuf_create("TX", lport, port, mbuf_count, MEMPOOL_CACHE_SIZE);
            if (port->tx_mp == NULL)
                rte_exit(EXIT_FAILURE, "Cannot init port %d for Default TX mbufs", port->pid);
        }
        if (port->special_mp == NULL) {
            /* Used for sending special packets like ARP requests */
            port->special_mp = l2p_pktmbuf_create("SP", lport, port, MAX_SPECIAL_MBUFS, 0);
            if (port->special_mp == NULL)
                rte_exit(EXIT_FAILURE, "Cannot init port %d for Special TX mbufs", pid);
        }
    } while (l++ < h);

    return num_cores;
}

static int
parse_mapping(const char *map)
{
    l2p_t *l2p = l2p_get();
    char *fields[3], *lcores[3];
    char *mapping = NULL;
    int num_fields, num_cores, num_lcores;
    uint16_t pid;

    if (!map || strlen(map) == 0) {
        printf("no mapping specified or string empty\n");
        goto leave;
    }

    mapping = alloca(MAX_ALLOCA_SIZE);
    if (!mapping) {
        printf("unable to allocate map string\n");
        goto leave;
    }
    snprintf(mapping, MAX_ALLOCA_SIZE, "%s", map);

    /* parse map into a lcore list and port number */
    num_fields = rte_strsplit(mapping, strlen(mapping), fields, RTE_DIM(fields), '.');
    if (num_fields != 2) {
        printf("Invalid mapping format '%s'\n", map);
        goto leave;
    }

    pid = strtol(fields[1], NULL, 10);
    if (pid >= RTE_MAX_ETHPORTS) {
        printf("Invalid port number '%s'\n", fields[1]);
        goto leave;
    }

    l2p->ports[pid].pid = pid;
    l2p->num_ports++;

    num_lcores = rte_strsplit(fields[0], strlen(mapping), lcores, RTE_DIM(lcores), ':');
    if (num_lcores == 1) {
        if (lcores[0][0] == '{' || lcores[0][0] == '[')
            lcores[0]++;
        num_cores = parse_cores(pid, lcores[0], LCORE_MODE_BOTH);
        if (num_cores <= 0) {
            printf("Invalid mapping format '%s'\n", map);
            goto leave;
        }
    } else {
        if (lcores[0][0] == '{' || lcores[0][0] == '[')
            lcores[0]++;
        num_cores = parse_cores(pid, lcores[0], LCORE_MODE_RX);
        if (num_cores <= 0) {
            printf("Invalid mapping format '%s'\n", map);
            goto leave;
        }

        if (lcores[1][strlen(lcores[1]) - 1] == '}' || lcores[1][strlen(lcores[1]) - 1] == ']')
            lcores[1][strlen(lcores[1]) - 1] = '\0';
        num_cores = parse_cores(pid, lcores[1], LCORE_MODE_TX);
        if (num_cores <= 0) {
            printf("Invalid mapping format '%s'\n", map);
            goto leave;
        }
    }
    return 0;
leave:
    return -1;
}

/**
 *
 * pg_parse_matrix - Parse the command line argument for port configuration
 *
 * DESCRIPTION
 * Parse the command line argument for port configuration.
 *
 * BNF: (or kind of BNF)
 *      <matrix-string> := """ <lcore-port> { "," <lcore-port>} """
 *		<lcore-port>	:= <lcore-list> "." <port>
 *		<lcore-list>	:= "[" <rx-list> ":" <tx-list> "]"
 *		<port>      	:= <num>
 *		<rx-list>		:= <num> { "/" (<num> | <list>) }
 *		<tx-list>		:= <num> { "/" (<num> | <list>) }
 *		<list>			:= <num>           { "/" (<range> | <list>) }
 *		<range>			:= <num> "-" <num> { "/" <range> }
 *		<num>			:= <digit>+
 *		<digit>			:= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 *
 * BTW: A single lcore can only handle a single port/queue, which means
 *      you can not have a single core processing more then one network device or port.
 *
 *	1.0, 2.1, 3.2                   - core 1 handles port 0 rx/tx,
 *					  core 2 handles port 1 rx/tx
 *	[0-1].0, [2/4-5].1, ...		- cores 0-1 handle port 0 rx/tx,
 *					  cores 2,4,5 handle port 1 rx/tx
 *	[1:2].0, [4:6].1, ...		- core 1 handles port 0 rx,
 *					  core 2 handles port 0 tx,
 *	[1:2-3].0, [4:5-6].1, ...	- core 1 handles port 1 rx, cores 2,3 handle port 0 tx
 *					  core 4 handles port 1 rx & core 5,6 handles port 1 tx
 *	[1-2:3].0, [4-5:6].1, ...	- core 1,2 handles port 0 rx, core 3 handles port 0 tx
 *					  core 4,5 handles port 1 rx & core 6 handles port 1 tx
 *	[1-2:3-5].0, [4-5:6/8].1, ...	- core 1,2 handles port 0 rx, core 3,4,5 handles port 0 tx
 *					  core 4,5 handles port 1 rx & core 6,8 handles port 1 tx
 *	BTW: you can use "{}" instead of "[]" as it does not matter to the syntax.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
int
l2p_parse_mappings(void)
{
    l2p_t *l2p = l2p_get();

    for (int i = 0; i < l2p->num_mappings; i++)
        if (parse_mapping(l2p->mappings[i]))
            return -1;
    pktgen_log_info("%*sTotal memory used = %'8" PRIu64 " KB", 54, " ",
                    (pktgen.total_mem_used + 1023) / 1024);
    return 0;
}

int
l2p_parse_mapping_add(const char *str)
{
    l2p_t *l2p = l2p_get();

    if (!str || l2p->num_mappings > RTE_MAX_ETHPORTS)
        return -1;
    l2p->mappings[l2p->num_mappings++] = strdup(str);
    return 0;
}
