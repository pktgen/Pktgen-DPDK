/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#ifndef __WR_L2P_H
#define __WR_L2P_H

#include <string.h>

#include <rte_memory.h>
#include <rte_atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MATRIX_ENTRIES      128
#define MAX_STRING              256

#define MAX_MAP_PORTS           (RTE_MAX_ETHPORTS + 1)
#define MAX_MAP_LCORES          (RTE_MAX_LCORE + 1)

enum { NO_TYPE = 0, RX_TYPE = 0x01, TX_TYPE = 0x02 };

typedef struct pq_s {
	uint16_t rx_cnt;
	uint16_t tx_cnt;
	uint16_t rx[RTE_MAX_ETHPORTS];
	uint16_t tx[RTE_MAX_ETHPORTS];
} pq_t;

typedef struct {
	uint16_t lid;
	uint16_t type;
	pq_t pids;
	pq_t qids;
	void      *private;
} lobj_t;	/* lcore type and ports/qids */

typedef struct {
	uint16_t pid;
	uint16_t rx_qid;
	uint16_t tx_qid;
	uint16_t nb_lids;
	uint16_t lids[RTE_MAX_LCORE];
	void      *private;
} pobj_t;	/* ports pointer lcores */

typedef union {
	struct {
		uint16_t rx;
		uint16_t tx;
	};
	uint32_t rxtx;
} rxtx_t;

typedef struct {
	volatile uint8_t stop[RTE_MAX_LCORE];
	lobj_t lcores[RTE_MAX_LCORE];
	pobj_t ports[RTE_MAX_ETHPORTS];
	rxtx_t map[MAX_MAP_PORTS][MAX_MAP_LCORES];
} l2p_t;

/**************************************************************************//**
 * Dump the L2P structure in a raw format.
 *
 */
static __inline__ void
pg_raw_dump_l2p(l2p_t *l2p)
{
	uint16_t i, j;
	lobj_t    *lobj;
	pobj_t    *pobj;
	const char    *types[] = { "Unkn", " RX ", " TX ", "RXTX", NULL };

	for (j = 0; j < RTE_MAX_LCORE; j++) {
		lobj = &l2p->lcores[j];
		if (lobj->pids.rx_cnt || lobj->pids.tx_cnt) {
			printf("lcore %2d: type %s private %p\n", lobj->lid,
			       types[lobj->type], lobj->private);
			printf("    Pid rx_cnt(%2d) ", lobj->pids.rx_cnt);
			for (i = 0; i < lobj->pids.rx_cnt; i++)
				printf("%2d ", lobj->pids.rx[i]);
			printf("\n");
			printf("        tx_cnt(%2d) ", lobj->pids.tx_cnt);
			for (i = 0; i < lobj->pids.tx_cnt; i++)
				printf("%2d ", lobj->pids.tx[i]);
			printf("\n");
		}
		if (lobj->qids.rx_cnt || lobj->qids.tx_cnt) {
			printf("    Qid rx_cnt(%2d) ", lobj->qids.rx_cnt);
			for (i = 0; i < lobj->qids.rx_cnt; i++)
				printf("%2d ", lobj->qids.rx[i]);
			printf("\n");
			printf("        tx_cnt(%2d) ", lobj->qids.tx_cnt);
			for (i = 0; i < lobj->qids.tx_cnt; i++)
				printf("%2d ", lobj->qids.tx[i]);
			printf("\n");
		}
	}

	printf("Ports:\n");
	for (i = 0; i < RTE_MAX_ETHPORTS; i++) {
		pobj = &l2p->ports[i];
		if (pobj->nb_lids == 0)
			continue;
		printf("  Lids idx %2d: RX %2d, TX %2d, private %p: ",
		       pobj->pid, pobj->rx_qid, pobj->tx_qid, pobj->private);
		for (j = 0; j < pobj->nb_lids; j++)
			printf("%2d ", pobj->lids[j]);
		printf("\n");
	}
	printf("\n");
}

/**************************************************************************//**
 * Create and initialize the lcore to port object
 *
 * RETURNS: Pointer to l2p_t or NULL on error.
 */
static __inline__ l2p_t *
l2p_create(void)
{
	l2p_t *l2p;
	uint32_t i;

	l2p = (l2p_t *)calloc(1, sizeof(l2p_t));
	if (l2p == NULL)
		return NULL;

	/* I know calloc zeros memory, but just to be safe. */
	memset(l2p, 0, sizeof(l2p_t));

	for (i = 0; i < RTE_MAX_LCORE; i++)
		l2p->lcores[i].lid  = i;

	for (i = 0; i < RTE_MAX_ETHPORTS; i++)
		l2p->ports[i].pid   = i;

	/* Set them to stopped first. */
	for (i = 0; i < RTE_MAX_LCORE; i++)
		l2p->stop[i] = 1;

	return l2p;
}

/**************************************************************************//**
 * Get a new RX queue value
 *
 */
static __inline__ uint16_t
pg_new_rxque(l2p_t *l2p, uint16_t pid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->rx_qid++;
}

/**************************************************************************//**
 * Get a new tx queue value
 *
 */
static __inline__ uint16_t
pg_new_txque(l2p_t *l2p, uint16_t pid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->tx_qid++;
}

/**************************************************************************//**
 * Increase the number of RX ports.
 *
 */
static __inline__ void
pg_inc_rx(l2p_t *l2p, uint16_t pid, uint16_t lid) {
	l2p->map[pid][lid].rx++;
}

/**************************************************************************//**
 * Increase the number of TX ports
 *
 */
static __inline__ void
pg_inc_tx(l2p_t *l2p, uint16_t pid, uint16_t lid) {
	l2p->map[pid][lid].tx++;
}

/**************************************************************************//**
 * return the rxtx_t value at given lcore/port index
 *
 */
static __inline__ uint32_t
get_map(l2p_t *l2p, uint16_t pid, uint16_t lid)
{
	return l2p->map[pid][lid].rxtx;
}

/**************************************************************************//**
 * return the rxtx_t value at given lcore/port index
 *
 */
static __inline__ void
put_map(l2p_t *l2p, uint16_t pid, uint16_t lid, uint32_t rxtx)
{
	l2p->map[pid][lid].rxtx = rxtx;
}

/**************************************************************************//**
 * Grab the cnt value from the given lcore id.
 *
 */
static __inline__ uint16_t
get_type(l2p_t *l2p, uint16_t lid)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->type;
}

/**************************************************************************//**
 * Grab the cnt value from the given lcore id.
 *
 */
static __inline__ uint16_t
get_lcore_rxcnt(l2p_t *l2p, uint16_t lid)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->pids.rx_cnt;
}

/**************************************************************************//**
 * Grab the cnt value from the given lcore id.
 *
 */
static __inline__ uint16_t
get_lcore_txcnt(l2p_t *l2p, uint16_t lid)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->pids.tx_cnt;
}

/**************************************************************************//**
 * Grab the cnt value from the given lcore id.
 *
 */
static __inline__ uint16_t
get_port_rxcnt(l2p_t *l2p, uint16_t pid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->rx_qid;
}

/**************************************************************************//**
 * Grab the cnt value from the given lcore id.
 *
 */
static __inline__ uint16_t
get_port_txcnt(l2p_t *l2p, uint16_t pid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->tx_qid;
}

/**************************************************************************//**
 * Grab the cnt value from the given port id.
 *
 */
static __inline__ uint16_t
get_port_nb_lids(l2p_t *l2p, uint16_t pid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->nb_lids;
}

/**************************************************************************//**
 * Set the private pointer.
 *
 */
static __inline__ void
pg_set_port_private(l2p_t *l2p, uint16_t pid, void *ptr)
{
	pobj_t    *pobj = &l2p->ports[pid];

	pobj->private = ptr;
}

/**************************************************************************//**
 * Grab the private pointer.
 *
 */
static __inline__ void *
get_port_private(l2p_t *l2p, uint16_t pid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->private;
}

/**************************************************************************//**
 * set the private pointer.
 *
 */
static __inline__ void
pg_set_lcore_private(l2p_t *l2p, uint16_t lid, void *ptr)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	lobj->private = ptr;
}

/**************************************************************************//**
 * Grab the private pointer.
 *
 */
static __inline__ void *
get_lcore_private(l2p_t *l2p, uint16_t lid)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->private;
}

/**************************************************************************//**
 * Get RX pid
 *
 */
static __inline__ uint16_t
get_rx_pid(l2p_t *l2p, uint16_t lid, uint16_t idx)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->pids.rx[idx];
}

/**************************************************************************//**
 * Get TX pid
 *
 */
static __inline__ uint16_t
get_tx_pid(l2p_t *l2p, uint16_t lid, uint16_t idx)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->pids.tx[idx];
}

/**************************************************************************//**
 * Get the lid for this pid/qid
 *
 */
static __inline__ uint16_t
get_port_lid(l2p_t *l2p, uint16_t pid, uint16_t qid)
{
	pobj_t    *pobj = &l2p->ports[pid];

	return pobj->lids[qid];
}

/**************************************************************************//**
 * Get the number of rx qids
 *
 */
static __inline__ uint16_t
get_rxque(l2p_t *l2p, uint16_t lid, uint16_t pid)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->qids.rx[pid];
}

/**************************************************************************//**
 * Get the number of tx qids
 *
 */
static __inline__ uint16_t
get_txque(l2p_t *l2p, uint16_t lid, uint16_t pid)
{
	lobj_t    *lobj = &l2p->lcores[lid];

	return lobj->qids.tx[pid];
}

/**************************************************************************//**
 * Stop the given lcore
 *
 */
static __inline__ void
pg_stop_lcore(l2p_t *l2p, uint16_t lid)
{
	l2p->stop[lid] = 1;
}

/**************************************************************************//**
 * Stop the given lcore
 *
 */
static __inline__ void
pg_start_lcore(l2p_t *l2p, uint16_t lid)
{
	l2p->stop[lid] = 0;
}

/**************************************************************************//**
 * Return stop flag
 *
 */
static __inline__ int32_t
pg_lcore_is_running(l2p_t *l2p, uint16_t lid)
{
	return l2p->stop[lid] == 0;
}

/******************************************************************************/

/**************************************************************************//**
 * Dump out l2p_t structure.
 *
 */
static __inline__ void
pg_dump_l2p(l2p_t *l2p)
{
	lobj_t        *lobj;
	pobj_t        *pobj;
	uint16_t lid, pid, i;
	const char    *types[] =
	{ "Unknown", "RX-Only", "TX-Only", "RX-TX  ", NULL };

	printf("Lcore:\n");
	for (lid = 0; lid < RTE_MAX_LCORE; lid++) {
		lobj = &l2p->lcores[lid];

		if (lobj->pids.rx_cnt || lobj->pids.tx_cnt) {
			printf("   %2d, %s\n",
			       lobj->lid, types[lobj->type]);

			if (lobj->pids.rx_cnt) {
				printf("                RX_cnt(%2d): ",
				       lobj->pids.rx_cnt);
				for (i = 0; i < lobj->pids.rx_cnt; i++)
					printf("(pid=%2d:qid=%2d) ",
					       lobj->pids.rx[i],
					       lobj->qids.rx[lobj->pids.rx[i]]);

				printf("\n");
			}
			if (lobj->pids.tx_cnt) {
				printf("                TX_cnt(%2d): ",
				       lobj->pids.tx_cnt);
				for (i = 0; i < lobj->pids.tx_cnt; i++)
					printf("(pid=%2d:qid=%2d) ",
					       lobj->pids.tx[i],
					       lobj->qids.tx[lobj->pids.tx[i]]);

				printf("\n");
			}
		}
	}
	printf("\n");
	printf("Port :\n");
	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		pobj = &l2p->ports[pid];

		if (pobj->nb_lids) {
			printf("   %2d, nb_lcores %2d, private %p,",
			       pobj->pid, pobj->nb_lids, pobj->private);

			printf(" lcores: ");
			for (i = 0; i < pobj->nb_lids; i++)
				printf("%2d ", pobj->lids[i]);
			printf("\n");
		}
	}
	printf("\n\n");
}

void pg_port_matrix_dump(l2p_t *l2p);
int pg_parse_matrix(l2p_t *l2p, char *str);
uint32_t pg_parse_portmask(const char *portmask);

#ifdef __cplusplus
}
#endif

#endif /* __WR_L2P_H */
