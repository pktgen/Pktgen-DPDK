/*-
 * Copyright (c) <2010-2013>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
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

#include "wr_scrn.h"
#include "wr_l2p.h"
#include "wr_core_info.h"
#include "wr_utils.h"

#define countof(t)		(sizeof(t)/sizeof(t[0]))

enum { RX_IDX = 0, TX_IDX = 1, RXTX_CNT = 2 };

typedef struct ls_s {
	uint8_t		ls[RTE_MAX_LCORE/8];
} ls_t;

typedef struct ps_s {
	uint8_t		ps[RTE_MAX_ETHPORTS/8];
} ps_t;

typedef struct lcore_port_s {
	ls_t		lcores[RXTX_CNT];
	ps_t		ports[RXTX_CNT];
} lp_t;

static inline int _btst(uint8_t *p, uint8_t idx) {
	int32_t	c = (idx/8);
	return (p[c] & (1 << (idx - (c * 8))));
}

static inline void _bset(uint8_t * p, uint8_t idx) {
	int32_t	c = (idx/8);
	p[c] |= (1 << (idx - (c * 8)));
}

/**************************************************************************//**
*
* wr_parse_portmask - Parse the portmask from the command line.
*
* DESCRIPTION
* Parse the portmask string from the command line.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

uint32_t
wr_parse_portmask(const char *portmask)
{
    return strtoul(portmask, NULL, 0);
}

/**************************************************************************//**
*
* wr_parse_rt_list - Parse the Rx/Tx list string.
*
* DESCRIPTION
* Parse the Rx/Tx list string and update the bitmaps.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int32_t
wr_parse_rt_list(char * list, uint8_t * map)
{
	char	  * p;
	int32_t		k, i;
	char	  * arr[33];

	if ( list == NULL )
		return 1;

	p = wr_strtrimset(list, "[]{}");		// trim the two character sets from the front/end of string.
	if ( (p == NULL) || (*p == '\0') )
		return 1;

	// Split up the string by '/' for each list or range set
	k = wr_strparse(p, "/", arr, countof(arr));
	if ( k == 0 )
		return 1;

	for(i = 0; (i < k) && arr[i]; i++) {
		p = strchr(arr[i], '-');
		if ( p != NULL ) {				// Found a range list
			uint32_t	l, h;
			*p++ = '\0';
			l = strtol(arr[i], NULL, 10);
			h = strtol(p, NULL, 10);
			do {
				_bset(map, l);
			} while( ++l <= h );
		} else							// Must be a single value
			_bset(map, strtol(arr[i], NULL, 10));
	}
	return 0;
}

/**************************************************************************//**
*
* wr_parse_lcore_list - Parse the lcore list string.
*
* DESCRIPTION
* Parse the lcore list strings.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int32_t
wr_parse_lcore_list( char * list, ls_t *ls)
{
	char	  * arr[3];
	int32_t		k;

	// Split up the string based on the ':' for Rx:Tx pairs
    k = wr_strparse( list, ":", arr, countof(arr) );
    if ( (k == 0) || (k == 3) ) {
    	fprintf(stderr, "*** Invalid string (%s)\n", list);
    	return 1;
    }

    if ( k == 1 ) {								// Must be a lcore/port number only
    	wr_parse_rt_list(arr[0], ls[RX_IDX].ls);	// Parse the list with no ':' character
    	memcpy(ls[TX_IDX].ls, ls[RX_IDX].ls, sizeof(ls_t)); // Update the tx bitmap too.
    } else /* k == 2 */ {						// Must be a <rx-list>:<tx-list> pair
		if ( wr_parse_rt_list(arr[0], ls[RX_IDX].ls) )	// parse <rx-list>
			return 1;

		if ( wr_parse_rt_list(arr[1], ls[TX_IDX].ls) )	// parse <tx-list>
			return 1;
    }
	return 0;
}

/**************************************************************************//**
*
* wr_parse_port_list - Parse the port list string.
*
* DESCRIPTION
* Parse the port list strings.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int32_t
wr_parse_port_list( char * list, ps_t *ps)
{
	char	  * arr[3];
	int32_t		k;

	// Split up the string based on the ':' for Rx:Tx pairs
    k = wr_strparse( list, ":", arr, countof(arr) );
    if ( (k == 0) || (k == 3) ) {
    	fprintf(stderr, "*** Invalid string (%s)\n", list);
    	return 1;
    }

    if ( k == 1 ) {								// Must be a lcore/port number only
    	wr_parse_rt_list(arr[0], ps[RX_IDX].ps);	// Parse the list with no ':' character
    	memcpy(ps[TX_IDX].ps, ps[RX_IDX].ps, sizeof(ps_t)); // Update the tx bitmap too.
    } else /* k == 2 */ {						// Must be a <rx-list>:<tx-list> pair
		if ( wr_parse_rt_list(arr[0], ps[RX_IDX].ps) )	// parse <rx-list>
			return 1;

		if ( wr_parse_rt_list(arr[1], ps[TX_IDX].ps) )	// parse <tx-list>
			return 1;
    }
	return 0;
}

static inline void wr_dump_lcore_map(const char * t, uint8_t *m) {
	int		i;

	fprintf(stderr, "%s( ", t);

	for(i=0; i < RTE_MAX_LCORE; i++){
		if ( _btst(m, i) )
			fprintf(stderr, "%d ", i);
	}
	fprintf(stderr, ")");
}

static inline void wr_dump_port_map(const char * t, uint8_t *m) {
	int		i;

	fprintf(stderr, "%s( ", t);

	for(i=0; i < RTE_MAX_ETHPORTS; i++){
		if ( _btst(m, i) )
			fprintf(stderr, "%d ", i);
	}
	fprintf(stderr, ")");
}

/**************************************************************************//**
*
* wr_parse_matrix - Parse the command line argument for port configuration
*
* DESCRIPTION
* Parse the command line argument for port configuration.
*
* BNF: (or kind of BNF)
* 		<matrix-string> := """ <lcore-port> { "," <lcore-port>} """
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
*	1.0, 2.1, 3.2 					- core 1 handles port 0 rx/tx,
*									  core 2 handles port 1 rx/tx
*	1.[0-2], 2.3, ...				- core 1 handle ports 0,1,2 rx/tx,
*									  core 2 handle port 3 rx/tx
*	[0-1].0, [2/4-5].1, ...			- cores 0-1 handle port 0 rx/tx,
*									  cores 2,4,5 handle port 1 rx/tx
*	[1:2].0, [4:6].1, ...			- core 1 handles port 0 rx,
*									  core 2 handles port 0 tx,
*	[1:2].[0-1], [4:6].[2/3], ...	- core 1 handles port 0 & 1 rx,
*									  core 2 handles port  0 & 1 tx
*	[1:2-3].0, [4:5-6].1, ...		- core 1 handles port 1 rx, cores 2,3 handle port 0 tx
*									  core 4 handles port 1 rx & core 5,6 handles port 1 tx
*	[1-2:3].0, [4-5:6].1, ...		- core 1,2 handles port 0 rx, core 3 handles port 0 tx
*									  core 4,5 handles port 1 rx & core 6 handles port 1 tx
*	[1-2:3-5].0, [4-5:6/8].1, ...	- core 1,2 handles port 0 rx, core 3,4,5 handles port 0 tx
*									  core 4,5 handles port 1 rx & core 6,8 handles port 1 tx
*	[1:2].[0:0-7], [3:4].[1:0-7], 	- core 1 handles port 0 rx, core 2 handles ports 0-7 tx
*									  core 3 handles port 1 rx & core 4 handles port 0-7 tx
*	BTW: you can use "{}" instead of "[]" as it does not matter to the syntax.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

int
wr_parse_matrix(l2p_t * l2p, char * str)
{
    char      * lcore_port[MAX_MATRIX_ENTRIES];
    char		buff[256];
    int         i, m, k, lid_type, pid_type;
	uint32_t	pid, lid;
    lp_t		lp;
    rxtx_t		cnt, n;

    wr_strccpy(buff, str, " \r\n\t\"");

    // Split up the string into <lcore-port>, ...
    k = wr_strparse(buff, ",", lcore_port, countof(lcore_port));
    if ( k <= 0 ) {
    	fprintf(stderr, "%s: could not parse (%s) string\n",
    			__FUNCTION__, buff);
    	return 0;
    }

    for(i = 0; (i < k) && lcore_port[i]; i++) {
    	char	  * arr[3];
    	char		str[64];

    	// Grab a private copy of the string.
    	strncpy(str, lcore_port[i], sizeof(str));

        // Parse the string into <lcore-list> and <port-list>
		m = wr_strparse( lcore_port[i], ".", arr, 3 );
		if ( m != 2 ) {
			fprintf(stderr, "%s: could not parse <lcore-list>.<port-list> (%s) string\n",
					__FUNCTION__, lcore_port[i]);
			return 0;
		}

        memset(&lp, '\0', sizeof(lp));

		if ( wr_parse_lcore_list(arr[0], lp.lcores) ) {
			fprintf(stderr, "%s: could not parse <lcore-list> (%s) string\n",
					__FUNCTION__, arr[0]);
			return 0;
		}

		if ( wr_parse_port_list(arr[1], lp.ports) ) {
			fprintf(stderr, "%s: could not parse <port-list> (%s) string\n",
					__FUNCTION__, arr[1]);
			return 0;
		}

		// Handle the lcore and port list maps
		fprintf(stderr, "%-16s lcores: ", str);
		wr_dump_lcore_map("RX", lp.lcores[RX_IDX].ls);
		wr_dump_lcore_map("TX", lp.lcores[TX_IDX].ls);
		fprintf(stderr, " ports: ");
		wr_dump_port_map("RX", lp.ports[RX_IDX].ps);
		wr_dump_port_map("TX", lp.ports[TX_IDX].ps);
		fprintf(stderr, "\n");

    	for(lid = 0; lid < RTE_MAX_LCORE; lid++) {
    		lid_type = 0;
    		if ( _btst(lp.lcores[RX_IDX].ls, lid) )
    			lid_type |= RX_TYPE;
    		if ( _btst(lp.lcores[TX_IDX].ls, lid) )
    			lid_type |= TX_TYPE;
	   		if ( lid_type == 0 )
	   			continue;

	   		for(pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
	   			pid_type = 0;
	    		if ( _btst(lp.ports[RX_IDX].ps, pid) )
	    			pid_type |= RX_TYPE;
				if ( _btst(lp.ports[TX_IDX].ps, pid) )
	    			pid_type |= TX_TYPE;
		   		if ( pid_type == 0 )
		   			continue;
	    		wr_l2p_connect(l2p, pid, lid, lid_type);
	   		}
	   	}
    }

	for(pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
 		n.rxtx = 0;
 		for(lid = 0; lid < RTE_MAX_LCORE; lid++) {
            if ( (cnt.rxtx = wr_get_map(l2p, pid, lid)) > 0) {
            	if ( cnt.tx > 0 )
            		n.tx++;
            	if ( cnt.rx > 0 )
            		n.rx++;
            }
        }
        l2p->map[pid][lid].rxtx = n.rxtx;          // Update the lcores per port
    }
   	for(lid = 0; lid < RTE_MAX_LCORE; lid++) {
   		n.rxtx = 0;
   		for(pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
			if ( (cnt.rxtx = wr_get_map(l2p, pid, lid)) > 0) {
				if ( cnt.tx > 0 )
					n.tx++;
				if ( cnt.rx > 0 )
					n.rx++;
			 }
        }
        l2p->map[pid][lid].rxtx = n.rxtx;          // Update the ports per lcore
    }

    return 0;
}

/**************************************************************************//**
*
* wr_port_matrix_dump - Dump out the matrix for all ports
*
* DESCRIPTION
* Display the matrix of ports and mappings for all ports.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
wr_port_matrix_dump(l2p_t * l2p)
{
    uint32_t    pid, lid;
    uint8_t		first, last;
    rxtx_t		cnt;

    first = last = 0;

    printf("\n=== port to lcore mapping table (# lcores %d) ===\n",
    		wr_lcore_mask(&first, &last));

    printf("   lcore: ");
    for(lid = first; lid <= last; lid++)
    		printf("   %2d ", lid);
    printf("\n");

    for(pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
    	cnt.rxtx = wr_get_map(l2p, pid, RTE_MAX_LCORE);
    	if ( cnt.rxtx == 0 )
    		continue;
        printf("port  %2d:", pid);
        for(lid = first; lid <= last; lid++) {
			cnt.rxtx = wr_get_map(l2p, pid, lid);
			if ( lid == rte_get_master_lcore() )
				printf(" %s:%s", " D", " T");
			else
				printf(" %2d:%2d", cnt.rx, cnt.tx);
        }
        cnt.rxtx = wr_get_map(l2p, pid, RTE_MAX_LCORE);
        printf(" = %2d:%2d\n", cnt.rx, cnt.tx);
    }

    printf("Total   :");
    for(lid = first; lid <= last; lid++) {
    	cnt.rxtx = wr_get_map(l2p, RTE_MAX_ETHPORTS, lid);
    	printf(" %2d:%2d", cnt.rx, cnt.tx);
    }

    printf("\n    Display and Timer on lcore %d, rx:tx counts per port/lcore\n\n",
    		rte_get_master_lcore());
    fflush(stdout);
}
