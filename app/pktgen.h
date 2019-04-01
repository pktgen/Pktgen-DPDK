/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
#include <time.h>

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
#include <rte_branch_prediction.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_timer.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_tcp.h>

#include <copyright_info.h>
#include <l2p.h>
#include <port_config.h>
#include <core_info.h>

#include <_pcap.h>
#include <pg_inet.h>
#include <cksum.h>

#include <mbuf.h>
#include <coremap.h>
#include <lscpu.h>
#include <utils.h>

#include "pktgen-port-cfg.h"
#include "pktgen-capture.h"
#include "pktgen-log.h"
#include "pktgen-latency.h"
#include "pktgen-seq.h"

#include <cli.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PKTGEN_VERSION          "3.6.6"
#define PKTGEN_APP_NAME         "Pktgen"
#define PKTGEN_CREATED_BY       "Keith Wiles"

#define MAX_MATRIX_ENTRIES      128
#define MAX_STRING              256
#define Million                 (uint64_t)(1000000ULL)

#define iBitsTotal(_x) \
	(uint64_t)(((_x.ipackets * PKT_OVERHEAD_SIZE) + _x.ibytes) * 8)
#define oBitsTotal(_x) \
	(uint64_t)(((_x.opackets * PKT_OVERHEAD_SIZE) + _x.obytes) * 8)

#define _do(_exp)       do { _exp; } while ((0))

#ifndef RTE_ETH_FOREACH_DEV
#define RTE_ETH_FOREACH_DEV(p)	for(_p = 0; _p < pktgen.nb_ports; _p++)
#endif

#define forall_ports(_action)					\
	do {							\
		uint16_t pid;					\
								\
		RTE_ETH_FOREACH_DEV(pid) {			\
			port_info_t *info;			\
								\
			info = &pktgen.info[pid];		\
			if (info->seq_pkt == NULL)		\
				continue;			\
			_action;				\
		}						\
	} while ((0))

#define foreach_port(_portlist, _action)				\
	do {								\
		uint64_t *_pl = (uint64_t *)&_portlist;			\
		uint16_t pid, idx, bit;					\
									\
		RTE_ETH_FOREACH_DEV(pid) {				\
			port_info_t   *info;				\
									\
			idx = (pid / (sizeof(uint64_t) * 8));		\
			bit = (pid - (idx * (sizeof(uint64_t) * 8)));	\
			if ( (_pl[idx] & (1LL<< bit)) == 0)		\
				continue;				\
			info = &pktgen.info[pid];			\
			if (info->seq_pkt == NULL)			\
				continue;				\
			_action;					\
		}							\
	} while ((0))

/**
 * Free a list of packet mbufs back into its original mempool.
 *
 * Free a list of mbufs by calling rte_pktmbuf_free() in a loop as a
 * wrapper function.
 *
 * @param m_list
 *   An array of rte_mbuf pointers to be freed.
 * @param npkts
 *   Number of packets to free in m_list.
 */
static inline void __attribute__((always_inline))
rte_pktmbuf_free_bulk(struct rte_mbuf *m_list[], int16_t npkts)
{
	struct rte_mbuf *m;
	while (npkts--) {
		m = *m_list++;
		rte_pktmbuf_free(m);
		m->next = 0;
	}
}

typedef enum { PACKET_CONSUMED = 0, UNKNOWN_PACKET = 0xEEEE,
	       DROP_PACKET = 0xFFFE, FREE_PACKET = 0xFFFF } pktType_e;

enum {
	MAX_SCRN_ROWS           = 44,
	MAX_SCRN_COLS           = 132,

	COLUMN_WIDTH_0          = 22,
	COLUMN_WIDTH_1          = 20,
	COLUMN_WIDTH_3          = 22,

	/* Row locations for start of data */
	PORT_STATE_ROWS         = 1,
	LINK_STATE_ROWS         = 4,
	PKT_SIZE_ROWS           = 10,
	PKT_TOTALS_ROWS         = 7,
	IP_ADDR_ROWS            = 12,

	PORT_STATE_ROW          = 2,
	LINK_STATE_ROW          = (PORT_STATE_ROW + PORT_STATE_ROWS),
	PKT_SIZE_ROW            = (LINK_STATE_ROW + LINK_STATE_ROWS),
	PKT_TOTALS_ROW          = (PKT_SIZE_ROW + PKT_SIZE_ROWS),
	IP_ADDR_ROW             = (PKT_TOTALS_ROW + PKT_TOTALS_ROWS),

	DEFAULT_NETMASK         = 0xFFFFFF00,
	DEFAULT_IP_ADDR         = (192 << 24) | (168 << 16),
	DEFAULT_TX_COUNT        = 0,	/* Forever */
	DEFAULT_TX_RATE         = 100,
	DEFAULT_PRIME_COUNT     = 1,
	DEFAULT_SRC_PORT        = 1234,
	DEFAULT_DST_PORT        = 5678,
	DEFAULT_PKT_NUMBER      = 0x012345678,
	DEFAULT_ACK_NUMBER      = 0x012345690,
	DEFAULT_WND_SIZE        = 8192,
	MIN_VLAN_ID             = 1,
	MAX_VLAN_ID             = 4095,
	DEFAULT_VLAN_ID         = MIN_VLAN_ID,
	MIN_COS             	= 0,
	MAX_COS 	            = 7,
	DEFAULT_COS	         	= MIN_COS,
	MIN_TOS             	= 0,
	MAX_TOS 	            = 255,
	DEFAULT_TOS		        = MIN_TOS,
	MAX_ETHER_TYPE_SIZE     = 0x600,
	OVERHEAD_FUDGE_VALUE    = 50,

	DEFAULT_PORTS_PER_PAGE  = 4,
	VLAN_TAG_SIZE           = 4,
	MAX_PRIME_COUNT         = 4,

	NUM_SEQ_PKTS            = 16,	/* Number of buffers to support in sequence */
	NUM_EXTRA_TX_PKTS       = 8,	/* Number of extra TX packets */

	FIRST_SEQ_PKT           = 0,
	SINGLE_PKT              = (FIRST_SEQ_PKT + NUM_SEQ_PKTS),	/* 16 */
	PING_PKT                = (SINGLE_PKT + 1),			/* 17 */
	RANGE_PKT               = (PING_PKT + 1),			/* 18 */
	DUMP_PKT                = (RANGE_PKT + 1),			/* 19 */
	EXTRA_TX_PKT            = (DUMP_PKT + 1),			/* 20 */
	NUM_TOTAL_PKTS          = (EXTRA_TX_PKT + NUM_EXTRA_TX_PKTS),

	INTER_FRAME_GAP         = 12,	/**< in bytes */
	START_FRAME_DELIMITER	= 1,
	PKT_PREAMBLE_SIZE       = 7,	/**< in bytes */
	PKT_OVERHEAD_SIZE	= (INTER_FRAME_GAP + START_FRAME_DELIMITER +
				   PKT_PREAMBLE_SIZE + ETHER_CRC_LEN),

	MIN_PKT_SIZE            = (ETHER_MIN_LEN - ETHER_CRC_LEN),
	MAX_PKT_SIZE            = (ETHER_MAX_LEN - ETHER_CRC_LEN),
	MIN_v6_PKT_SIZE         = (78 - ETHER_CRC_LEN),

	MAX_RX_QUEUES           = 16,	/**< RX Queues per port */
	MAX_TX_QUEUES           = 16,	/**< TX Queues per port */

	PCAP_PAGE_SIZE          = 25,	/**< Size of the PCAP display page */

	SOCKET0                 = 0	/**< Socket ID value for allocation */
};

typedef struct rte_mbuf rte_mbuf_t;

typedef union {
	struct ether_addr addr;
	uint64_t u64;
} ethaddr_t;

#define MAX_PORT_DESC_SIZE  132
/* Ethernet addresses of ports */
typedef struct pktgen_s {
	struct cmdline *cl;	/**< Command Line information pointer */
	luaData_t *ld;		/**< General Lua Data pointer */
	luaData_t *ld_sock;	/**< Info for Lua Socket */
	char *hostname;		/**< GUI hostname */
	int verbose;		/**< Verbose flag */

	int32_t socket_port;		/**< GUI port number */
	uint32_t blinklist;		/**< Port list for blinking the led */
	uint32_t flags;			/**< Flag values */
	uint16_t ident;			/**< IPv4 ident value */
	uint16_t last_row;		/**< last static row of the screen */
	uint16_t nb_ports;		/**< Number of ports in the system */
	uint8_t starting_port;		/**< Starting port to display */
	uint8_t ending_port;		/**< Ending port to display */
	uint8_t nb_ports_per_page;	/**< Number of ports to display per page */

	uint16_t nb_rxd;	/**< Number of receive descriptors */
	uint16_t nb_txd;	/**< Number of transmit descriptors */
	uint16_t portNum;	/**< Current Port number */
	uint16_t port_cnt;	/**< Number of ports used in total */
	uint64_t hz;		/**< Number of events per seconds */

	int (*callout)(void *callout_arg);
	void *callout_arg;

	struct rte_pci_addr blacklist[RTE_MAX_ETHPORTS];
	struct rte_pci_addr portlist[RTE_MAX_ETHPORTS];
	uint8_t *portdesc[RTE_MAX_ETHPORTS];
	uint32_t portdesc_cnt;
	uint32_t blacklist_cnt;

	/* port to lcore mapping */
	l2p_t *l2p;

	port_info_t info[RTE_MAX_ETHPORTS];	/**< Port information */
	lc_info_t core_info[RTE_MAX_LCORE];
	uint16_t core_cnt;
	uint16_t pad0;
	lscpu_t *lscpu;
	char *uname;
	eth_stats_t cumm_rate_totals;	/**< port rates total values */
	uint64_t max_total_ipackets;	/**< Total Max seen input packet rate */
	uint64_t max_total_opackets;	/**< Total Max seen output packet rate */

	pthread_t thread;	/**< Thread structure for Lua server */

	uint64_t counter;	/**< A debug counter */
	uint64_t mem_used;	/**< Display memory used counters per ports */
	uint64_t total_mem_used;/**< Display memory used for all ports */
	int32_t argc;	/**< Number of arguments */
	char *argv[64];	/**< Argument list */

	capture_t capture[RTE_MAX_NUMA_NODES];	/**< Packet capture, 1 struct per socket */
	uint8_t is_gui_running;
	volatile uint8_t timer_running;
} pktgen_t;

enum {						/* Queue flags */
	CLEAR_FAST_ALLOC_FLAG   = 0x00000001,	/**< Clear the TX fast alloc flag */
	DO_TX_FLUSH             = 0x00000002	/**< Do a TX Flush by sending all of the pkts in the queue */
};

enum {						/* Pktgen flags bits */
	PRINT_LABELS_FLAG       = (1 << 0),	/**< Print constant labels on stats display */
	MAC_FROM_ARP_FLAG       = (1 << 1),	/**< Configure the SRC MAC from a ARP request */
	PROMISCUOUS_ON_FLAG     = (1 << 2),	/**< Enable promiscuous mode */
	NUMA_SUPPORT_FLAG       = (1 << 3),	/**< Enable NUMA support */
	IS_SERVER_FLAG          = (1 << 4),	/**< Pktgen is a Server */
	ENABLE_GUI_FLAG         = (1 << 5),	/**< GUI support is enabled */
	LUA_SHELL_FLAG          = (1 << 6),	/**< Enable Lua Shell */
	TX_DEBUG_FLAG           = (1 << 7),	/**< TX Debug output */
	Not_USED                = (1 << 8),	/**< Not Used */
	FAKE_PORTS_FLAG         = (1 << 9),	/**< Fake ports enabled */
	BLINK_PORTS_FLAG        = (1 << 10),	/**< Blink the port leds */
	ENABLE_THEME_FLAG       = (1 << 11),	/**< Enable theme or color support */

	CONFIG_PAGE_FLAG        = (1 << 16),	/**< Display the configure page */
	SEQUENCE_PAGE_FLAG      = (1 << 17),	/**< Display the Packet sequence page */
	RANGE_PAGE_FLAG         = (1 << 18),	/**< Display the range page */
	PCAP_PAGE_FLAG          = (1 << 19),	/**< Display the PCAP page */
	CPU_PAGE_FLAG           = (1 << 20),	/**< Display the PCAP page */
	RND_BITFIELD_PAGE_FLAG  = (1 << 21),	/**< Display the random bitfield page */
	LOG_PAGE_FLAG           = (1 << 22),	/**< Display the message log page */
	LATENCY_PAGE_FLAG       = (1 << 23),	/**< Display latency page */
	STATS_PAGE_FLAG         = (1 << 24),	/**< Display the physical port stats */
	XSTATS_PAGE_FLAG        = (1 << 25),	/**< Display the physical port stats */

	UPDATE_DISPLAY_FLAG     = (1 << 31)
};

#define UPDATE_DISPLAY_RATE             1	/* one second */
#define UPDATE_DISPLAY_TICK_INTERVAL    8
#define UPDATE_DISPLAY_TICK_RATE	\
	((pktgen.hz * UPDATE_DISPLAY_RATE) / UPDATE_DISPLAY_TICK_INTERVAL)

#define PAGE_MASK_BITS  (CONFIG_PAGE_FLAG | SEQUENCE_PAGE_FLAG | \
			 RANGE_PAGE_FLAG | \
			 PCAP_PAGE_FLAG | CPU_PAGE_FLAG | \
			 RND_BITFIELD_PAGE_FLAG | \
			 LOG_PAGE_FLAG | LATENCY_PAGE_FLAG | \
			 XSTATS_PAGE_FLAG | STATS_PAGE_FLAG)

extern pktgen_t pktgen;

void pktgen_page_display(struct rte_timer *tim, void *arg);

void pktgen_packet_ctor(port_info_t *info, int32_t seq_idx,
			       int32_t type);
void pktgen_packet_rate(port_info_t *info);

void pktgen_send_mbuf(struct rte_mbuf *m, uint8_t pid, uint16_t qid);

pkt_seq_t *pktgen_find_matching_ipsrc(port_info_t *info, uint32_t addr);
pkt_seq_t *pktgen_find_matching_ipdst(port_info_t *info, uint32_t addr);

int pktgen_launch_one_lcore(void *arg);
uint64_t pktgen_wire_size(port_info_t *info);
void pktgen_input_start(void);

void rte_timer_setup(void);

static __inline__ void
pktgen_set_port_flags(port_info_t *info, uint32_t flags) {
	uint32_t val;

	do
		val = rte_atomic32_read(&info->port_flags);
	while (rte_atomic32_cmpset((volatile uint32_t *)&info->port_flags.cnt,
				   val, (val | flags)) == 0);
}

static __inline__ void
pktgen_clr_port_flags(port_info_t *info, uint32_t flags) {
	uint32_t val;

	do
		val = rte_atomic32_read(&info->port_flags);
	while (rte_atomic32_cmpset((volatile uint32_t *)&info->port_flags.cnt,
				   val, (val & ~flags)) == 0);
}

static __inline__ void
pktgen_set_q_flags(port_info_t *info, uint8_t q, uint32_t flags) {
	uint32_t val;

	do
		val = rte_atomic32_read(&info->q[q].flags);
	while (rte_atomic32_cmpset((volatile uint32_t *)&info->q[q].flags.cnt,
				   val, (val | flags)) == 0);
}

static __inline__ void
pktgen_clr_q_flags(port_info_t *info, uint8_t q, uint32_t flags) {
	uint32_t val;

	do
		val = rte_atomic32_read(&info->q[q].flags);
	while (rte_atomic32_cmpset((volatile uint32_t *)&info->q[q].flags.cnt,
				   val, (val & ~flags)) == 0);
}

/* onOff values */
enum { DISABLE_STATE = 0, ENABLE_STATE = 1 };

static __inline__ uint32_t
estate(const char *state) {
	return ( !strcasecmp(state, "on") ||
		 !strcasecmp(state, "enable") ||
		 !strcasecmp(state, "start") ) ?
	       ENABLE_STATE : DISABLE_STATE;
}

/**
 * Function returning string of version number: "- Version:x.y.x (DPDK-x.y.z)"
 * @return
 *     string
 */
static inline const char *
pktgen_version(void) {
	static char pkt_version[128];

	snprintf(pkt_version, sizeof(pkt_version),
		 "Ver: %s (%s)", PKTGEN_VERSION, rte_version());
	return pkt_version;
}

static __inline__ char *
strdupf(char *str, const char *new) {
	if (str)
		free(str);
	return (new == NULL) ? NULL : strdup(new);
}

/**************************************************************************//**
 *
 * do_command - Internal function to execute a shell command and grab the output.
 *
 * DESCRIPTION
 * Internal function to execute a shell command and grab the output from the
 * command.
 *
 * RETURNS: Nubmer of lines read.
 *
 * SEE ALSO:
 */

static __inline__ int
do_command(const char *cmd, int (*display)(char *, int)) {
	FILE      *f;
	int i;
	char *line = NULL;
	size_t line_size = 0;

	f = popen(cmd, "r");
	if (f == NULL) {
		pktgen_log_error("Unable to run '%s' command", cmd);
		return -1;
	}

	i = 0;
	while (getline(&line, &line_size, f) > 0)
		i = display(line, i);

	if (f)
		fclose(f);
	if (line)
		free(line);

	return i;
}

#ifndef MEMPOOL_F_DMA
#define MEMPOOL_F_DMA       0
#endif

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_H_ */
