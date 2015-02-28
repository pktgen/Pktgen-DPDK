/**
 * Copyright (c) <2014>, Wind River Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2014 by Keith Wiles @ intel.com */

#ifndef __WR_L2P_H
#define __WR_L2P_H

#include <string.h>

#include <rte_memory.h>
#include <rte_atomic.h>


#define MAX_MATRIX_ENTRIES      128
#define MAX_STRING              256
#define	MAX_MAP_PORTS			(RTE_MAX_ETHPORTS + 1)
#define MAX_MAP_LCORES			(RTE_MAX_LCORE + 1)

enum { NO_TYPE = 0, RX_TYPE = 0x01, TX_TYPE = 0x02 };

typedef struct pq_s {
	uint8_t		rx_cnt;
	uint8_t		tx_cnt;
	uint8_t		pad0[2];
	uint16_t	rx[RTE_MAX_ETHPORTS];
	uint16_t	tx[RTE_MAX_ETHPORTS];
} pq_t;

typedef struct {
	uint8_t		lid;
    uint8_t		type;
    uint8_t		pad0[2];
    pq_t		pids;
    pq_t		qids;
    void	  * private;
} __rte_cache_aligned lobj_t;

typedef struct {
	uint8_t		pid;
    uint8_t		rx_qid;
    uint8_t		tx_qid;
    uint8_t		nb_lids;
	uint8_t		lids[RTE_MAX_LCORE];
	void	  * private;
} __rte_cache_aligned pobj_t;

typedef union {
	struct {
		uint8_t	rx;
		uint8_t	tx;
	};
	uint16_t	rxtx;
} rxtx_t;

typedef struct {
	volatile uint8_t	stop[RTE_MAX_LCORE];
	lobj_t				lcores[RTE_MAX_LCORE];
	pobj_t				ports[RTE_MAX_ETHPORTS];
	rxtx_t				map[MAX_MAP_PORTS][MAX_MAP_LCORES];
} __rte_cache_aligned l2p_t;

/**************************************************************************//**
* Dump the L2P structure in a raw format.
*
*/
static __inline__ void
wr_raw_dump_l2p(l2p_t * l2p)
{
	uint8_t		i, j;
	lobj_t	  * lobj;
	pobj_t	  * pobj;
	const char	  * types[] = { "Unkn", " RX ", " TX ", "RXTX", NULL };

	printf("\nRunning lcores: ");
	for(i = 0; i  <RTE_MAX_LCORE; i++) {
		if ( l2p->stop[i] == 0 )
			printf("%2d ", i);
	}
	printf("\n");

	printf("Lcores:\n");
	for(j = 0; j < RTE_MAX_LCORE; j++) {
		lobj = &l2p->lcores[j];
		if ( lobj->pids.rx_cnt || lobj->pids.tx_cnt ) {
			printf(" %2d: type %s private %p\n", lobj->lid, types[lobj->type], lobj->private);
			printf("    Pids RX %2d: ", lobj->pids.rx_cnt);
			for(i = 0; i < lobj->pids.rx_cnt; i++)
				printf("%2d ", lobj->pids.rx[i]);
			printf("\n");
			printf("    Pids TX %2d: ", lobj->pids.tx_cnt);
			for(i = 0; i < lobj->pids.tx_cnt; i++)
				printf("%2d ", lobj->pids.tx[i]);
			printf("\n");
		}
		if ( lobj->qids.rx_cnt || lobj->qids.tx_cnt ) {
			printf("    Qids RX %2d: ", lobj->qids.rx_cnt);
			for(i = 0; i < RTE_MAX_ETHPORTS; i++)
				printf("%2d ", lobj->qids.rx[i]);
			printf("\n");
			printf("    Qids TX %2d: ", lobj->qids.tx_cnt);
			for(i = 0; i < RTE_MAX_ETHPORTS; i++)
				printf("%2d ", lobj->qids.tx[i]);
			printf("\n");
		}
	}

	printf("Ports:\n");
	for(i = 0; i < RTE_MAX_ETHPORTS; i++) {
		pobj = &l2p->ports[i];
		if ( pobj->nb_lids == 0 )
			continue;
		printf("  Lids idx %2d: RX %2d, TX %2d, private %p: ",
				pobj->pid, pobj->rx_qid, pobj->tx_qid, pobj->private);
		for(j = 0; j < pobj->nb_lids; j++)
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
wr_l2p_create(void)
{
	l2p_t	  * l2p;
	uint32_t	i;

	l2p = (l2p_t *)calloc(1, sizeof(l2p_t));
	if ( l2p == NULL )
		return NULL;

	// I know calloc zeros memory, but just to be safe.
	memset(l2p, 0, sizeof(l2p_t));

	for(i = 0; i < RTE_MAX_LCORE; i++)
		l2p->lcores[i].lid	= i;

	for(i = 0; i < RTE_MAX_ETHPORTS; i++)
		l2p->ports[i].pid	= i;

	// Set them to stopped first.
	for(i = 0; i < RTE_MAX_LCORE; i++)
		l2p->stop[i] = 1;

	return l2p;
}

/**************************************************************************//**
* Get a new RX queue value
*
*/
static __inline__ uint8_t
wr_new_rxque(l2p_t * l2p, uint8_t pid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->rx_qid++;
}

/**************************************************************************//**
* Get a new tx queue value
*
*/
static __inline__ uint8_t
wr_new_txque(l2p_t * l2p, uint8_t pid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->tx_qid++;
}

/**************************************************************************//**
 * Increase the number of RX ports.
 *
 */
static __inline__ void
wr_inc_rx(l2p_t * l2p, uint8_t pid, uint8_t lid) {
	l2p->map[pid][lid].rx++;
}

/**************************************************************************//**
 * Increase the number of TX ports
 *
 */
static __inline__ void
wr_inc_tx(l2p_t * l2p, uint8_t pid, uint8_t lid) {
	l2p->map[pid][lid].tx++;
}

/**************************************************************************//**
 * return the rxtx_t value at given lcore/port index
 *
 */
static __inline__ uint16_t
wr_get_map(l2p_t * l2p, uint8_t pid, uint8_t lid)
{
	return l2p->map[pid][lid].rxtx;
}

/**************************************************************************//**
* Add an RX lcore to a port
*
*/
static __inline__ void
wr_l2p_connect( l2p_t * l2p,  uint8_t pid, uint8_t lid, uint32_t type )
{
	lobj_t	  * lobj = &l2p->lcores[lid];
	pobj_t	  * pobj = &l2p->ports[pid];

    lobj->type	= type;

    if ( type & RX_TYPE ) {
        lobj->pids.rx[lobj->pids.rx_cnt++]	= pid;
    	lobj->qids.rx[pid]	= wr_new_rxque(l2p, pid);	// allocate a RX qid
    	lobj->qids.rx_cnt++;
		wr_inc_rx(l2p, pid, lid);
    }

    if ( type & TX_TYPE ) {
        lobj->pids.tx[lobj->pids.tx_cnt++]	= pid;
    	lobj->qids.tx[pid]	= wr_new_txque(l2p, pid);	// Allocate a TX qid
    	lobj->qids.tx_cnt++;
		wr_inc_tx(l2p, pid, lid);
    }
    pobj->lids[pobj->nb_lids++]	= lid;
}

/**************************************************************************//**
* Grab the cnt value from the given lcore id.
*
*/
static __inline__ uint8_t
wr_get_type(l2p_t * l2p, uint8_t lid)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->type;
}

/**************************************************************************//**
* Grab the cnt value from the given lcore id.
*
*/
static __inline__ uint8_t
wr_get_lcore_rxcnt(l2p_t * l2p, uint8_t lid)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->pids.rx_cnt;
}

/**************************************************************************//**
* Grab the cnt value from the given lcore id.
*
*/
static __inline__ uint8_t
wr_get_lcore_txcnt(l2p_t * l2p, uint8_t lid)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->pids.tx_cnt;
}

/**************************************************************************//**
* Grab the cnt value from the given lcore id.
*
*/
static __inline__ uint8_t
wr_get_port_rxcnt(l2p_t * l2p, uint8_t pid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->rx_qid;
}

/**************************************************************************//**
* Grab the cnt value from the given lcore id.
*
*/
static __inline__ uint8_t
wr_get_port_txcnt(l2p_t * l2p, uint8_t pid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->tx_qid;
}

/**************************************************************************//**
* Grab the cnt value from the given port id.
*
*/
static __inline__ uint8_t
wr_get_port_nb_lids(l2p_t * l2p, uint8_t pid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->nb_lids;
}

/**************************************************************************//**
* Set the private pointer.
*
*/
static __inline__ void
wr_set_port_private(l2p_t * l2p, uint8_t pid, void * ptr)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	pobj->private = ptr;
}

/**************************************************************************//**
* Grab the private pointer.
*
*/
static __inline__ void *
wr_get_port_private(l2p_t * l2p, uint8_t pid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->private;
}

/**************************************************************************//**
* set the private pointer.
*
*/
static __inline__ void
wr_set_lcore_private(l2p_t * l2p, uint8_t lid, void * ptr)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	lobj->private = ptr;
}

/**************************************************************************//**
* Grab the private pointer.
*
*/
static __inline__ void *
wr_get_lcore_private(l2p_t * l2p, uint8_t lid)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->private;
}

/**************************************************************************//**
* Get RX pid
*
*/
static __inline__ uint8_t
wr_get_rx_pid(l2p_t * l2p, uint8_t lid, uint8_t idx)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->pids.rx[idx];
}

/**************************************************************************//**
* Get TX pid
*
*/
static __inline__ uint8_t
wr_get_tx_pid(l2p_t * l2p, uint8_t lid, uint8_t idx)
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->pids.tx[idx];
}

/**************************************************************************//**
 * Get the lid for this pid/qid
 *
 */
static __inline__ uint8_t
wr_get_port_lid(l2p_t * l2p, uint8_t pid, uint8_t qid)
{
	pobj_t	  * pobj = &l2p->ports[pid];

	return pobj->lids[qid];
}

/**************************************************************************//**
 * Get the number of rx qids
 *
 */
static __inline__ uint16_t
wr_get_rxque( l2p_t * l2p, uint8_t lid, uint8_t pid )
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->qids.rx[pid];
}

/**************************************************************************//**
 * Get the number of tx qids
 *
 */
static __inline__ uint16_t
wr_get_txque( l2p_t * l2p, uint8_t lid, uint8_t pid )
{
	lobj_t	  * lobj = &l2p->lcores[lid];

	return lobj->qids.tx[pid];
}

/**************************************************************************//**
 * Stop the given lcore
 *
 */
static __inline__ void
wr_stop_lcore( l2p_t * l2p, uint8_t lid )
{
	l2p->stop[lid] = 1;
	rte_mb();
}

/**************************************************************************//**
 * Stop the given lcore
 *
 */
static __inline__ void
wr_start_lcore( l2p_t * l2p, uint8_t lid )
{
	l2p->stop[lid] = 0;
	rte_mb();
}

/**************************************************************************//**
 * Return stop flag
 *
 */
static __inline__ int32_t
wr_lcore_is_running( l2p_t * l2p, uint8_t lid )
{
	rte_mb();
	return (l2p->stop[lid] == 0);
}

/******************************************************************************/

/**************************************************************************//**
 * Dump out l2p_t structure.
 *
 */
static __inline__ void
wr_dump_l2p( l2p_t * l2p)
{
	lobj_t	  	  * lobj;
	pobj_t		  * pobj;
	uint8_t			lid, pid, i;
	const char	  * types[] = { "Unknown", "RX-Only", "TX-Only", "RX-TX  ", NULL };

	printf("Lcore:\n");
	for(lid = 0; lid < RTE_MAX_LCORE; lid++) {
		lobj = &l2p->lcores[lid];

		if ( lobj->pids.rx_cnt || lobj->pids.tx_cnt ) {
			printf("   %2d, %s\n",
				lobj->lid, types[lobj->type]);

			if ( lobj->pids.rx_cnt ) {
				printf("                RX(%2d): ", lobj->pids.rx_cnt);
				for(i = 0; i < lobj->pids.rx_cnt; i++)
					printf("(%2d:%2d) ", lobj->pids.rx[i], lobj->qids.rx[lobj->pids.rx[i]]);
				printf("\n");
			}
			if ( lobj->pids.tx_cnt ) {
				printf("                TX(%2d): ", lobj->pids.tx_cnt);
				for(i = 0; i < lobj->pids.tx_cnt; i++)
					printf("(%2d:%2d) ", lobj->pids.tx[i], lobj->qids.tx[lobj->pids.tx[i]]);
				printf("\n");
			}
		}
	}
	printf("\n");
	printf("Port :\n");
	for(pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		pobj = &l2p->ports[pid];

		if ( pobj->nb_lids ) {
			printf("   %2d, nb_lcores %2d, private %p,",
				pobj->pid, pobj->nb_lids, pobj->private);

			printf(" lcores: ");
			for(i = 0; i < pobj->nb_lids; i++)
				printf("%2d ", pobj->lids[i]);
			printf("\n");
		}
	}
	printf("\n\n");
}

extern void wr_port_matrix_dump(l2p_t * l2p);
extern int wr_parse_matrix(l2p_t * l2p, char * str);
extern uint32_t wr_parse_portmask(const char *portmask);

#endif /* __WR_L2P_H */
