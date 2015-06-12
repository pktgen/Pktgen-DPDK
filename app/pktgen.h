/*-
 * Copyright (c) <2010>, Intel Corporation
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
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_H_
#define _PKTGEN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <assert.h>

#include <rte_version.h>
#include <rte_config.h>

#include <rte_errno.h>
#include <rte_log.h>
#include <rte_tailq.h>
#include <rte_common.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_timer.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <wr_scrn.h>

#include <wr_copyright_info.h>
#include <wr_l2p.h>
#include <wr_port_config.h>
#include <wr_core_info.h>

#include <cmdline_rdline.h>
#include <cmdline_parse.h>
#include <cmdline_socket.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_num.h>
#include <cmdline_parse_ipaddr.h>
#include <cmdline_parse_etheraddr.h>
#include <cmdline_parse_portlist.h>
#include <cmdline.h>

#include <wr_pcap.h>
#include <wr_inet.h>
#include <wr_cksum.h>

#include <wr_cycles.h>
#include <wr_mbuf.h>
#include <wr_coremap.h>
#include <wr_lscpu.h>
#include <wr_utils.h>

#include "pktgen-port-cfg.h"
#include "pktgen-capture.h"
#include "pktgen-log.h"


#define PKTGEN_VERSION			"2.9.1"
#define PKTGEN_APP_NAME			"Pktgen"
#define PKTGEN_CREATED_BY		"Keith Wiles"

#define MAX_MATRIX_ENTRIES      128
#define MAX_STRING              256
#define Million					(uint64_t)(1000ULL * 1000ULL)
#define Mega					(uint64_t)(1024ULL * 1024ULL)

#define iBitsTotal(_x) \
    (((_x.ipackets * (INTER_FRAME_GAP + PKT_PREAMBLE_SIZE)) + _x.ibytes) << 3)
#define oBitsTotal(_x) \
    (((_x.opackets * (INTER_FRAME_GAP + PKT_PREAMBLE_SIZE)) + _x.obytes) << 3)

#define _do(_exp)		do { _exp; } while((0))

#define forall_ports(_action)							\
	do {												\
		uint32_t		pid;							\
		for(pid = 0; pid < pktgen.nb_ports; pid++) {	\
			port_info_t	  * info;						\
			info = &pktgen.info[pid];					\
			if ( info->seq_pkt == NULL )				\
				continue;								\
			_action;									\
		}												\
	} while((0))

#define foreach_port(_portlist, _action)				\
	do {												\
		uint32_t	* _pl = (uint32_t *)&_portlist;		\
		uint32_t	pid, idx, bit;						\
		for(pid = 0; pid < pktgen.nb_ports; pid++) {	\
			port_info_t	  * info;						\
			idx = (pid / (sizeof(uint32_t) * 8));		\
			bit = (pid - (idx * (sizeof(uint32_t) * 8)));\
			if ( (_pl[idx] & (1 << bit)) == 0 )			\
				continue;								\
			info = &pktgen.info[pid];					\
			if ( info->seq_pkt == NULL )				\
				continue;								\
			_action;									\
		}												\
	} while((0));

/**
 * Free a list of packet mbufs back into its original mempool.
 *
 * Free a list of mbufs by calling rte_pktmbuf_free() in a loop as a wrapper function.
 *
 * @param m_list
 *   An array of rte_mbuf pointers to be freed.
 * @param npkts
 *   Number of packets to free in m_list.
 */
static inline void __attribute__((always_inline))
rte_pktmbuf_free_bulk(struct rte_mbuf *m_list[], int16_t npkts)
{
    while(npkts--)
        rte_pktmbuf_free(*m_list++);
}

typedef enum { PACKET_CONSUMED = 0, UNKNOWN_PACKET = 0xEEEE, DROP_PACKET = 0xFFFE, FREE_PACKET = 0xFFFF } pktType_e;

enum {
	MAX_SCRN_ROWS			= 44,
	MAX_SCRN_COLS			= 132,

	COLUMN_WIDTH_0			= 18,
	COLUMN_WIDTH_1			= 19,

	// Row locations for start of data
	PORT_STATE_ROWS			= 1,
	LINK_STATE_ROWS			= 4,
	PKT_SIZE_ROWS			= 9,
	PKT_TOTALS_ROWS			= 7,
	IP_ADDR_ROWS			= 10,

	PORT_STATE_ROW			= 2,
	LINK_STATE_ROW			= (PORT_STATE_ROW + PORT_STATE_ROWS),
	PKT_SIZE_ROW			= (LINK_STATE_ROW + LINK_STATE_ROWS),
	PKT_TOTALS_ROW			= (PKT_SIZE_ROW + PKT_SIZE_ROWS),
	IP_ADDR_ROW				= (PKT_TOTALS_ROW + PKT_TOTALS_ROWS),

	DEFAULT_NETMASK			= 0xFFFFFF00,
	DEFAULT_IP_ADDR			= (192 << 24) | (168 << 16),
	DEFAULT_TX_COUNT		= 0,			// Forever
	DEFAULT_TX_RATE			= 100,
	DEFAULT_PRIME_COUNT		= 3,
	DEFAULT_SRC_PORT		= 1234,
	DEFAULT_DST_PORT		= 5678,
	DEFAULT_PKT_NUMBER		= 0x012345678,
	DEFAULT_ACK_NUMBER		= 0x012345690,
	DEFAULT_WND_SIZE		= 8192,
	MIN_VLAN_ID				= 1,
	MAX_VLAN_ID				= 4095,
	DEFAULT_VLAN_ID			= MIN_VLAN_ID,
	MAX_ETHER_TYPE_SIZE		= 0x600,
	OVERHEAD_FUDGE_VALUE	= 50,

	DEFAULT_PORTS_PER_PAGE	= 4,
	VLAN_TAG_SIZE			= 4,
	MAX_PRIME_COUNT			= 4,

	NUM_SEQ_PKTS			= 16,		// Number of buffers to support in sequence
	NUM_EXTRA_TX_PKTS		= 8,		// Number of extra TX packets

	FIRST_SEQ_PKT			= 0,
	SINGLE_PKT				= (FIRST_SEQ_PKT + NUM_SEQ_PKTS),		// 16
	PING_PKT				= (SINGLE_PKT + 1),						// 17
	RANGE_PKT				= (PING_PKT + 1),						// 18
	EXTRA_TX_PKT			= (RANGE_PKT + 1),						// 19
	NUM_TOTAL_PKTS			= (EXTRA_TX_PKT + NUM_EXTRA_TX_PKTS),

	INTER_FRAME_GAP			= 12,
	PKT_PREAMBLE_SIZE		= 8,
	FCS_SIZE				= 4,
	MIN_PKT_SIZE			= (ETHER_MIN_LEN - FCS_SIZE),
	MAX_PKT_SIZE			= (ETHER_MAX_LEN - FCS_SIZE),

	MAX_RX_QUEUES			= 16,	/**< RX Queues per port */
	MAX_TX_QUEUES			= 16,	/**< TX Queues per port */

	PCAP_PAGE_SIZE			= 25,	/**< Size of the PCAP display page */

	SOCKET0					= 0		/**< Socket ID value for allocation */
};

typedef struct rte_mbuf	rte_mbuf_t;

typedef union {
	struct ether_addr	addr;
	uint64_t			u64;
} ethaddr_t;

#define MAX_PORT_DESC_SIZE	132

/* Ethernet addresses of ports */
typedef struct pktgen_s {
	struct cmdline		  * cl;					/**< Command Line information pointer */
	char				  * cmd_filename;		/**< Command file path and name */
	void				  * L;					/**< Lua State pointer */
	char				  * hostname;			/**< GUI hostname */
	wr_scrn_t			  * scrn;				/**< Screen structure pointer */

	int32_t					socket_port;		/**< GUI port number */
	uint32_t				blinklist;			/**< Port list for blinking the led */
	uint32_t				flags;				/**< Flag values */
	uint16_t				ident;				/**< IPv4 ident value */
	uint16_t				last_row;			/**< last static row of the screen */
	uint16_t				nb_ports;			/**< Number of ports in the system */
	uint8_t					starting_port;		/**< Starting port to display */
	uint8_t					ending_port;		/**< Ending port to display */
	uint8_t					nb_ports_per_page;	/**< Number of ports to display per page */

	uint16_t				nb_rxd;				/**< Number of receive descriptors */
	uint16_t				nb_txd;				/**< Number of transmit descriptors */
	uint16_t				portNum;			/**< Current Port number */
	uint16_t				port_cnt;			/**< Number of ports used in total */
	uint64_t				hz;					/**< Number of events per seconds */

	int						(*callout)(void * callout_arg);
	void				  * callout_arg;

	struct rte_pci_addr		blacklist[RTE_MAX_ETHPORTS];
	struct rte_pci_addr		portlist[RTE_MAX_ETHPORTS];
	uint8_t				  * portdesc[RTE_MAX_ETHPORTS];
	uint32_t				portdesc_cnt;
	uint32_t				blacklist_cnt;

	// port to lcore mapping
	l2p_t				  * l2p;

	port_info_t				info[RTE_MAX_ETHPORTS];	/**< Port information */
	lc_info_t				core_info[RTE_MAX_LCORE];
	uint16_t				core_cnt;
	uint16_t				pad0;
	lscpu_t				  * lscpu;
	char				  * uname;
	eth_stats_t				cumm_rate_totals;	/**< port rates total values */

	pthread_t				thread;				/**< Thread structure for Lua server */

	uint64_t				counter;			/**< A debug counter */
	uint64_t				mem_used;			/**< Display memory used counters per ports */
	uint64_t				total_mem_used;		/**< Display memory used for all ports */
	int32_t					argc;				/**< Number of arguments */
	char				  * argv[64];			/**< Argument list */

	capture_t				capture[RTE_MAX_NUMA_NODES];	/**< Packet capture, 1 struct per socket */
} pktgen_t;

enum {	// Queue flags
	DO_TX_CLEANUP			= 0x00000001,		/**< Do a TX cleanup */
	CLEAR_FAST_ALLOC_FLAG	= 0x00000002,		/**< Clear the TX fast alloc flag */
	DO_TX_FLUSH				= 0x00000004		/**< Do a TX Flush by sending all of the pkts in the queue */
};

enum {		// Pktgen flags bits
	PRINT_LABELS_FLAG		= (1 << 0),			/**< Print constant labels on stats display */
	MAC_FROM_ARP_FLAG		= (1 << 1),			/**< Configure the SRC MAC from a ARP request */
	PROMISCUOUS_ON_FLAG		= (1 << 2),			/**< Enable promiscuous mode */
	NUMA_SUPPORT_FLAG		= (1 << 3),			/**< Enable NUMA support */
	IS_SERVER_FLAG			= (1 << 4),			/**< Pktgen is a Server */
	ENABLE_GUI_FLAG			= (1 << 5),			/**< GUI support is enabled */
	LUA_SHELL_FLAG			= (1 << 6),			/**< Enable Lua Shell */
	TX_DEBUG_FLAG			= (1 << 7),			/**< TX Debug output */
	FAKE_PORTS_FLAG			= (1 << 9),			/**< Fake ports enabled */
	BLINK_PORTS_FLAG		= (1 << 10),		/**< Blink the port leds */
	ENABLE_THEME_FLAG		= (1 << 11),		/**< Enable theme or color support */

	CONFIG_PAGE_FLAG		= (1 << 16),		/**< Display the configure page */
	SEQUENCE_PAGE_FLAG		= (1 << 17),		/**< Display the Packet sequence page */
	RANGE_PAGE_FLAG			= (1 << 18),		/**< Display the range page */
	PCAP_PAGE_FLAG			= (1 << 19),		/**< Display the PCAP page */
	CPU_PAGE_FLAG			= (1 << 20),		/**< Display the PCAP page */
	RND_BITFIELD_PAGE_FLAG	= (1 << 21),		/**< Display the random bitfield page */
	LOG_PAGE_FLAG			= (1 << 22)			/**< Display the message log page */
};

#define	PAGE_MASK_BITS		(CONFIG_PAGE_FLAG | SEQUENCE_PAGE_FLAG | RANGE_PAGE_FLAG | \
							 PCAP_PAGE_FLAG | CPU_PAGE_FLAG | RND_BITFIELD_PAGE_FLAG | \
							 LOG_PAGE_FLAG)

struct cmdline_etheraddr {
	uint8_t	mac[6];
};
typedef struct cmdline_etheraddr cmdline_etheraddr_t;

extern pktgen_t		pktgen;

extern void pktgen_page_display(__attribute__((unused)) struct rte_timer *tim, __attribute__((unused)) void *arg);

extern void pktgen_packet_ctor(port_info_t * info, int32_t seq_idx, int32_t type);
extern void pktgen_packet_rate(port_info_t * info);

extern void pktgen_send_mbuf(struct rte_mbuf *m, uint8_t pid, uint16_t qid);

extern pkt_seq_t * pktgen_find_matching_ipsrc( port_info_t * info, uint32_t addr );
extern pkt_seq_t * pktgen_find_matching_ipdst( port_info_t * info, uint32_t addr );

extern int pktgen_launch_one_lcore(__attribute__ ((unused)) void * arg);

extern void rte_timer_setup(void);

static __inline__ void
pktgen_set_port_flags(port_info_t * info, uint32_t flags) {
	uint32_t	val;

    do {
    	val = rte_atomic32_read(&info->port_flags);
    } while( rte_atomic32_cmpset((volatile uint32_t *)&info->port_flags.cnt, val, (val | flags)) == 0 );
}

static __inline__ void
pktgen_clr_port_flags(port_info_t * info, uint32_t flags) {
	uint32_t	val;

    do {
    	val = rte_atomic32_read(&info->port_flags);
    } while( rte_atomic32_cmpset((volatile uint32_t *)&info->port_flags.cnt, val, (val & ~flags)) == 0 );
}

static __inline__ void
pktgen_set_q_flags(port_info_t * info, uint8_t q, uint32_t flags) {
	uint32_t	val;

    do {
    	val = rte_atomic32_read(&info->q[q].flags);
    } while( rte_atomic32_cmpset((volatile uint32_t *)&info->q[q].flags.cnt, val, (val | flags)) == 0 );
}

static __inline__ void
pktgen_clr_q_flags(port_info_t * info, uint8_t q, uint32_t flags) {
	uint32_t	val;

    do {
    	val = rte_atomic32_read(&info->q[q].flags);
    } while( rte_atomic32_cmpset((volatile uint32_t *)&info->q[q].flags.cnt, val, (val & ~flags)) == 0 );
}

/**
 * Function returning string of version number: "- Version:x.y.x (DPDK-x.y.z)"
 * @return
 *     string
 */
static inline const char *
pktgen_version(void) {
	return "Ver:"PKTGEN_VERSION"(DPDK-"
			RTE_STR(RTE_VER_MAJOR)"."
			RTE_STR(RTE_VER_MINOR)"."
			RTE_STR(RTE_VER_PATCH_LEVEL)")";
}

static __inline__ char *
strdupf(char * str, const char * new) {
	if ( str ) free(str);
	return (new == NULL) ? NULL : strdup(new);
}

/**************************************************************************//**
*
* do_command - Internal function to execute a shell command and grab the output.
*
* DESCRIPTION
* Internal function to execute a shell command and grab the output from the command.
*
* RETURNS: Nubmer of lines read.
*
* SEE ALSO:
*/

static __inline__ int
do_command(const char * cmd, int (*display)(char *, int)) {
	FILE	  * f;
	int			i;
	char * line = NULL;
	size_t	line_size = 0;

	f = popen(cmd, "r");
	if ( f == NULL ) {
		pktgen_log_error("Unable to run '%s' command", cmd);
		return -1;
	}

	i = 0;
	while(getline(&line, &line_size, f) > 0)
		i = display(line, i);

	if ( f ) fclose(f);
	if ( line ) free(line);

	return i;
}

#ifndef MEMPOOL_F_DMA
#define MEMPOOL_F_DMA       0
#endif


#endif /* _PKTGEN_H_ */
