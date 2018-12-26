/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
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

#include <rte_version.h>
#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_pci.h>
#include <rte_debug.h>
#include <rte_memory.h>

#include "l2p.h"
#include "core_info.h"
#include "utils.h"

#define countof(t)      (sizeof(t) / sizeof(t[0]))

enum { RX_IDX = 0, TX_IDX = 1, RXTX_CNT = 2 };

/* Make sure to force sizes to multiples of 8 */
typedef struct ls_s {
	uint8_t rbits[(RTE_MAX_LCORE + 7) / 8];
	uint8_t tbits[(RTE_MAX_LCORE + 7) / 8];
} ls_t;

typedef struct ps_s {
	uint8_t rbits[(RTE_MAX_ETHPORTS + 7) / 8];
	uint8_t tbits[(RTE_MAX_ETHPORTS + 7) / 8];
} ps_t;

typedef struct lcore_port_s {
	ls_t lcores;
	ps_t ports;
} lp_t;

static lp_t lp_data, *lp = &lp_data;

static inline int
_btst(uint8_t *p, uint16_t idx)
{
	int32_t c = (idx / 8);

	return p[c] & (1 << (idx - (c * 8)));
}

static inline void
_bset(uint8_t *p, uint16_t idx)
{
	int32_t c = (idx / 8);
	uint8_t t = p[c];

	p[c] = t | (1 << (idx - (c * 8)));
}

/**************************************************************************//**
 * Add an RX lcore to a port
 *
 */
static __inline__ void
l2p_connect(l2p_t *l2p,  uint16_t pid, uint16_t lid, uint16_t type)
{
	lobj_t    *lobj = &l2p->lcores[lid];
	pobj_t    *pobj = &l2p->ports[pid];

	lobj->type  = type;

	if (type & RX_TYPE) {
		lobj->pids.rx[lobj->pids.rx_cnt++]  = pid;
		lobj->qids.rx[pid]  = pg_new_rxque(l2p, pid);	/* allocate a RX qid */
		lobj->qids.rx_cnt++;
		pg_inc_rx(l2p, pid, lid);
	}

	if (type & TX_TYPE) {
		lobj->pids.tx[lobj->pids.tx_cnt++]  = pid;
		lobj->qids.tx[pid]  = pg_new_txque(l2p, pid);	/* Allocate a TX qid */
		lobj->qids.tx_cnt++;
		pg_inc_tx(l2p, pid, lid);
	}
	pobj->lids[pobj->nb_lids++] = lid;
}

/**************************************************************************//**
 *
 * pg_parse_portmask - Parse the portmask from the command line.
 *
 * DESCRIPTION
 * Parse the portmask string from the command line.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

uint32_t
pg_parse_portmask(const char *portmask)
{
	return strtoul(portmask, NULL, 0);
}

/**************************************************************************//**
 *
 * pg_parse_rt_list - Parse the Rx/Tx list string.
 *
 * DESCRIPTION
 * Parse the Rx/Tx list string and update the bitmaps.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int32_t
pg_parse_rt_list(char *list, uint8_t *map)
{
	char *p;
	int32_t k, i;
	char *arr[33];

	if (list == NULL)
		return 1;

	/* Split up the string by '/' or ',' for each list or range set */
	k = pg_strparse(list, "/", arr, countof(arr));
	if (k == 0) {
		fprintf(stderr, "No comma or '/' in list\n");
		return 1;
	}

	for (i = 0; (i < k) && arr[i]; i++) {
		p = strchr(arr[i], '-');
		if (p != NULL) {/* Found a range list */
			uint32_t l, h;
			*p++ = '\0';
			l = strtol(arr[i], NULL, 10);
			h = strtol(p, NULL, 10);
			do
				_bset(map, l);
			while (++l <= h);
		} else	/* Must be a single value */
			_bset(map, strtol(arr[i], NULL, 10));
	}
	return 0;
}

/**************************************************************************//**
 *
 * pg_parse_lcore_list - Parse the lcore list string.
 *
 * DESCRIPTION
 * Parse the lcore list strings.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int32_t
pg_parse_lcore_list(char *list, ls_t *ls)
{
	char *arr[3], *p;
	int32_t k;

	/* trim the two character sets from the front/end of string. */
	p = pg_strtrimset(list, "[]{}");
	if ( (p == NULL) || (*p == '\0') )
		return 1;

	/* Split up the string based on the ':' for Rx:Tx pairs */
	k = pg_strparse(p, ":", arr, countof(arr) );
	if ( (k == 0) || (k == 3) ) {
		fprintf(stderr, "*** Invalid string (%s)\n", p);
		return 1;
	}

	if (k == 1) {	/* Must be a lcore/port number only */
		pg_parse_rt_list(arr[0], ls->rbits);		/* Parse the list with no ':' character */
		pg_parse_rt_list(arr[0], ls->tbits);		/* Parse the list with no ':' character */
	} else {	/* k == 2 Must be a <rx-list>:<tx-list> pair */
		if (pg_parse_rt_list(arr[0], ls->rbits) )		/* parse <rx-list> */
			return 1;

		if (pg_parse_rt_list(arr[1], ls->tbits) )	/* parse <tx-list> */
			return 1;
	}
	return 0;
}

/**************************************************************************//**
 *
 * pg_parse_port_list - Parse the port list string.
 *
 * DESCRIPTION
 * Parse the port list strings.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int32_t
pg_parse_port_list(char *list, ps_t *ps)
{
	char *arr[3], *p;
	int32_t k;

	/* trim the two character sets from the front/end of string. */
	p = pg_strtrimset(list, "[]{}");
	if ( (p == NULL) || (*p == '\0') )
		return 1;

	/* Split up the string based on the ':' for Rx:Tx pairs */
	k = pg_strparse(p, ":", arr, countof(arr) );
	if ( (k == 0) || (k == 3) ) {
		fprintf(stderr, "*** Invalid string (%s)\n", p);
		return 1;
	}

	if (k == 1) {	/* Must be a lcore/port number only */
		pg_parse_rt_list(arr[0], ps->rbits);		/* Parse the list with no ':' character */
		pg_parse_rt_list(arr[0], ps->tbits);		/* Parse the list with no ':' character */
	} else {	/* k == 2 Must be a <rx-list>:<tx-list> pair */
		if (pg_parse_rt_list(arr[0], ps->rbits) )	/* parse <rx-list> */
			return 1;

		if (pg_parse_rt_list(arr[1], ps->tbits) )	/* parse <tx-list> */
			return 1;
	}
	return 0;
}

/**************************************************************************//**
 *
 * pg_parse_matrix - Parse the command line argument for port configuration
 *
 * DESCRIPTION
 * Parse the command line argument for port configuration.
 *
 * BNF: (or kind of BNF)
 *      <matrix-string> := """ <lcore-port> { "," <lcore-port>} """
 *		<lcore-port>	:= <lcore-list> "." <port-list>
 *		<lcore-list>	:= "[" <rx-list> ":" <tx-list> "]"
 *		<port-list>		:= "[" <rx-list> ":" <tx-list>"]"
 *		<rx-list>		:= <num> { "/" (<num> | <list>) }
 *		<tx-list>		:= <num> { "/" (<num> | <list>) }
 *		<list>			:= <num>           { "/" (<range> | <list>) }
 *		<range>			:= <num> "-" <num> { "/" <range> }
 *		<num>			:= <digit>+
 *		<digit>			:= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 *
 *	1.0, 2.1, 3.2                   - core 1 handles port 0 rx/tx,
 *					  core 2 handles port 1 rx/tx
 *	1.[0-2], 2.3, ...		- core 1 handle ports 0,1,2 rx/tx,
 *					  core 2 handle port 3 rx/tx
 *	[0-1].0, [2/4-5].1, ...		- cores 0-1 handle port 0 rx/tx,
 *					  cores 2,4,5 handle port 1 rx/tx
 *	[1:2].0, [4:6].1, ...		- core 1 handles port 0 rx,
 *					  core 2 handles port 0 tx,
 *	[1:2].[0-1], [4:6].[2/3], ...	- core 1 handles port 0 & 1 rx,
 *					  core 2 handles port  0 & 1 tx
 *	[1:2-3].0, [4:5-6].1, ...	- core 1 handles port 1 rx, cores 2,3 handle port 0 tx
 *					  core 4 handles port 1 rx & core 5,6 handles port 1 tx
 *	[1-2:3].0, [4-5:6].1, ...	- core 1,2 handles port 0 rx, core 3 handles port 0 tx
 *					  core 4,5 handles port 1 rx & core 6 handles port 1 tx
 *	[1-2:3-5].0, [4-5:6/8].1, ...	- core 1,2 handles port 0 rx, core 3,4,5 handles port 0 tx
 *					  core 4,5 handles port 1 rx & core 6,8 handles port 1 tx
 *	[1:2].[0:0-7], [3:4].[1:0-7],   - core 1 handles port 0 rx, core 2 handles ports 0-7 tx
 *					  core 3 handles port 1 rx & core 4 handles port 0-7 tx
 *	BTW: you can use "{}" instead of "[]" as it does not matter to the syntax.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

int
pg_parse_matrix(l2p_t *l2p, char *str)
{
	char *lcore_port[MAX_MATRIX_ENTRIES+1];
	char buff[512];
	int i, m, k;
	uint16_t pid, lid, lid_type, pid_type;
	rxtx_t cnt, n;

	pg_strccpy(buff, str, " \r\n\t\"");

	/* Split up the string into <lcore-port>, ... */
	k = pg_strparse(buff, ",", lcore_port, countof(lcore_port));
	if (k <= 0) {
		fprintf(stderr, "%s: could not parse (%s) string\n",
			__func__, buff);
		goto leave;
	}

	for (i = 0; (i < k) && lcore_port[i]; i++) {
		char *arr[3];
		char _str[128];

		memset(lp, '\0', sizeof(lp_t));

		/* Grab a private copy of the string. */
		snprintf(_str, sizeof(_str), "%s", lcore_port[i]);

		/* Parse the string into <lcore-list> and <port-list> */
		m = pg_strparse(_str, ".", arr, 3);
		if (m != 2) {
			fprintf(stderr, "%s: could not parse <lcore-list>.<port-list> (%s) string\n",
				__func__, lcore_port[i]);
			fprintf(stderr, "  Make sure to use '/' in the lcore and port list for ranges and not ','\n");
			fprintf(stderr, "  -m [2:3/4/5/6/8-12].0 or -m [2-4/7-8:9/11-14].0\n");
			goto leave;
		}

		if (pg_parse_lcore_list(arr[0], &lp->lcores) ) {
			fprintf(stderr, "%s: could not parse <lcore-list> (%s) string\n",
				__func__, arr[0]);
			goto leave;
		}

		if (pg_parse_port_list(arr[1], &lp->ports) ) {
			fprintf(stderr, "%s: could not parse <port-list> (%s) string\n",
				__func__, arr[1]);
			goto leave;
		}

		for (lid = 0; lid < RTE_MAX_LCORE; lid++) {
			lid_type = 0;

			if (_btst(lp->lcores.rbits, lid) )
				lid_type |= RX_TYPE;

			if (_btst(lp->lcores.tbits, lid) )
				lid_type |= TX_TYPE;

			if (lid_type == 0)
				continue;

			for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
				pid_type = 0;

				if (_btst(lp->ports.rbits, pid) )
					pid_type |= RX_TYPE;

				if (_btst(lp->ports.tbits, pid) )
					pid_type |= TX_TYPE;

				if (pid_type)
					l2p_connect(l2p, pid, lid, lid_type);
			}
		}
	}

	/* Count the number of ports per lcore, put in last slot */
	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		n.rxtx = 0;
		for (lid = 0; lid < RTE_MAX_LCORE; lid++) {
			cnt.rxtx = get_map(l2p, pid, lid);
			if (cnt.rxtx) {
				if (cnt.tx)
					n.tx++;
				if (cnt.rx)
					n.rx++;
			}
		}
		put_map(l2p, pid, RTE_MAX_LCORE, n.rxtx);
	}

	/* Count the number of lcores per port, put in last slot */
	for (lid = 0; lid < RTE_MAX_LCORE; lid++) {
		n.rxtx = 0;
		for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
			cnt.rxtx = get_map(l2p, pid, lid);
			if (cnt.rxtx) {
				if (cnt.tx)
					n.tx++;
				if (cnt.rx)
					n.rx++;
			}
		}
		put_map(l2p, RTE_MAX_ETHPORTS, lid, n.rxtx);
	}
	return 0;

leave:
	return -1;
}

/**************************************************************************//**
 *
 * pg_port_matrix_dump - Dump out the matrix for all ports
 *
 * DESCRIPTION
 * Display the matrix of ports and mappings for all ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pg_port_matrix_dump(l2p_t *l2p)
{
	uint32_t pid, lid;
	uint16_t first, last;
	rxtx_t cnt, tot;

	first = last = 0;

	printf("\n=== port to lcore mapping table (# lcores %d) ===\n",
	       lcore_mask(&first, &last));

	printf("   lcore:");
	for (lid = first; lid <= last; lid++)
		printf("   %2d   ", lid);
	printf("   Total\n");

	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		tot.rxtx = get_map(l2p, pid, RTE_MAX_LCORE);
		if (tot.rxtx == 0)
			continue;
		printf("port  %2d:", pid);
		for (lid = first; lid <= last; lid++) {
			cnt.rxtx = get_map(l2p, pid, lid);
			if (lid == rte_get_master_lcore() )
				printf(" (%s:%s)", " D", " T");
			else
				printf(" (%2d:%2d)", cnt.rx, cnt.tx);
		}
		printf(" = (%2d:%2d)\n", tot.rx, tot.tx);
	}

	printf("Total   :");
	for (lid = first; lid <= last; lid++) {
		cnt.rxtx = get_map(l2p, RTE_MAX_ETHPORTS, lid);
		printf(" (%2d:%2d)", cnt.rx, cnt.tx);
	}

	printf("\n  Display and Timer on lcore %d, rx:tx counts per port/lcore\n\n",
		rte_get_master_lcore());

	fflush(stdout);
}
