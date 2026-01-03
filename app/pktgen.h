/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
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
#include <rte_dev.h>
#include <rte_time.h>

#include <l2p.h>
#include <port_config.h>

#include <pg_inet.h>
#include <cksum.h>

#include <coreinfo.h>
#include <lscpu.h>
#include <utils.h>

#ifdef LUA_ENABLED
#include "lua_config.h"
#include "lauxlib.h"
#endif

#include "pktgen-port-cfg.h"
#include "pktgen-capture.h"
#include "pktgen-log.h"
#include "pktgen-latency.h"
#include "pktgen-random.h"
#include "pktgen-seq.h"
#include "pktgen-version.h"

#include <cli.h>

#ifdef LUA_ENABLED
#include <lua_config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MATRIX_ENTRIES 128
#define MAX_STRING         256
#define Million            (uint64_t)(1000000UL)
#define Billion            (uint64_t)(1000000000UL)

#define iBitsTotal(_x) (uint64_t)(((_x.ipackets * PKT_OVERHEAD_SIZE) + _x.ibytes) * 8)
#define oBitsTotal(_x) (uint64_t)(((_x.opackets * PKT_OVERHEAD_SIZE) + _x.obytes) * 8)

#define _do(_exp) \
    do {          \
        _exp;     \
    } while ((0))

#ifndef RTE_ETH_FOREACH_DEV
#define RTE_ETH_FOREACH_DEV(p) for (_p = 0; _p < pktgen.nb_ports; _p++)
#endif

#define forall_ports(_action)                \
    do {                                     \
        uint16_t pid;                        \
                                             \
        RTE_ETH_FOREACH_DEV(pid)             \
        {                                    \
            port_info_t *pinfo;              \
                                             \
            pinfo = l2p_get_port_pinfo(pid); \
            if (pinfo->seq_pkt == NULL)      \
                continue;                    \
            _action;                         \
        }                                    \
    } while ((0))

#define foreach_port(_portlist, _action)                  \
    do {                                                  \
        uint64_t *_pl = (uint64_t *)&_portlist;           \
        uint16_t pid, idx, bit;                           \
                                                          \
        RTE_ETH_FOREACH_DEV(pid)                          \
        {                                                 \
            port_info_t *pinfo;                           \
                                                          \
            idx = (pid / (sizeof(uint64_t) * 8));         \
            bit = (pid - (idx * (sizeof(uint64_t) * 8))); \
            if ((_pl[idx] & (1LL << bit)) == 0)           \
                continue;                                 \
            pinfo = l2p_get_port_pinfo(pid);              \
            if (pinfo->seq_pkt == NULL)                   \
                continue;                                 \
            _action;                                      \
        }                                                 \
    } while ((0))

typedef enum {
    PACKET_CONSUMED = 0,
    UNKNOWN_PACKET  = 0xEEEE,
    DROP_PACKET     = 0xFFFE,
    FREE_PACKET     = 0xFFFF
} pktType_e;

enum {
    MAX_SCRN_ROWS = 44,
    MAX_SCRN_COLS = 132,

    COLUMN_WIDTH_0 = 22,
    COLUMN_WIDTH_1 = 24,
    COLUMN_WIDTH_3 = 24,

    /* Row locations for start of data */
    PAGE_TITLE_ROWS = 1,
    PORT_FLAGS_ROWS = 1,
    LINK_STATE_ROWS = 1,
    PKT_RATE_ROWS   = 9,
    PKT_SIZE_ROWS   = 10,
    PKT_TOTALS_ROWS = 7,
    IP_ADDR_ROWS    = 12,

    PAGE_TITLE_ROW = 1,
    PORT_FLAGS_ROW = (PAGE_TITLE_ROW + PAGE_TITLE_ROWS),
    LINK_STATE_ROW = (PORT_FLAGS_ROW + PORT_FLAGS_ROWS),
    PKT_RATE_ROW   = (LINK_STATE_ROW + LINK_STATE_ROWS),
    PKT_SIZE_ROW   = (PKT_RATE_ROW + PKT_RATE_ROWS),
    PKT_TOTALS_ROW = (PKT_SIZE_ROW + PKT_SIZE_ROWS),
    IP_ADDR_ROW    = (PKT_TOTALS_ROW + PKT_TOTALS_ROWS),

    DEFAULT_NETMASK        = 0xFFFFFF00,
    DEFAULT_IP_ADDR        = (192 << 24) | (168 << 16),
    DEFAULT_TX_COUNT       = 0, /* Forever */
    DEFAULT_TX_RATE        = 100,
    DEFAULT_PRIME_COUNT    = 1,
    DEFAULT_SRC_PORT       = 1234,
    DEFAULT_DST_PORT       = 5678,
    DEFAULT_TTL            = 64,
    DEFAULT_TCP_SEQ_NUMBER = 0x12378,
    MAX_TCP_SEQ_NUMBER     = UINT32_MAX / 8,
    DEFAULT_TCP_ACK_NUMBER = 0x12390,
    MAX_TCP_ACK_NUMBER     = UINT32_MAX / 8,
    DEFAULT_TCP_FLAGS      = ACK_FLAG,
    DEFAULT_WND_SIZE       = 8192,
    MIN_VLAN_ID            = 1,
    MAX_VLAN_ID            = 4095,
    DEFAULT_VLAN_ID        = MIN_VLAN_ID,
    MIN_COS                = 0,
    MAX_COS                = 7,
    DEFAULT_COS            = MIN_COS,
    MIN_TOS                = 0,
    MAX_TOS                = 255,
    DEFAULT_TOS            = MIN_TOS,
    MAX_ETHER_TYPE_SIZE    = 0x600,
    OVERHEAD_FUDGE_VALUE   = 50,

    DEFAULT_PORTS_PER_PAGE = 4,
    VLAN_TAG_SIZE          = 4,
    MAX_PRIME_COUNT        = 4,

    NUM_SEQ_PKTS = 16, /* Number of buffers to support in sequence */

    FIRST_SEQ_PKT  = 0,
    SINGLE_PKT     = (FIRST_SEQ_PKT + NUM_SEQ_PKTS), /* 16 */
    SPECIAL_PKT    = (SINGLE_PKT + 1),               /* 17 */
    RANGE_PKT      = (SPECIAL_PKT + 1),              /* 18 */
    LATENCY_PKT    = (RANGE_PKT + 1),                /* 19 */
    NUM_TOTAL_PKTS = (LATENCY_PKT + 1),

    INTER_FRAME_GAP       = 12, /**< in bytes */
    START_FRAME_DELIMITER = 1,  /**< Starting frame delimiter bytes */
    PKT_PREAMBLE_SIZE     = 7,  /**< in bytes */
    /* total number of bytes in frame overhead, includes the FCS checksum byte count */
    PKT_OVERHEAD_SIZE =
        (INTER_FRAME_GAP + START_FRAME_DELIMITER + PKT_PREAMBLE_SIZE + RTE_ETHER_CRC_LEN),
    MIN_v6_PKT_SIZE = 78, /**< does include FCS bytes */

    MAX_RX_QUEUES  = 16, /**< RX Queues per port */
    MAX_TX_QUEUES  = 16, /**< TX Queues per port */
    PCAP_PAGE_SIZE = 25  /**< Size of the PCAP display page */
};

#define WIRE_SIZE(pkt_size, t) (t)(pkt_size + PKT_OVERHEAD_SIZE)

typedef struct rte_mbuf rte_mbuf_t;

typedef union {
    struct rte_ether_addr addr;
    uint64_t u64;
} ethaddr_t;

/* Ethernet addresses of ports */
typedef struct pktgen_s {
    int verbose;                           /**< Verbose flag */
    int32_t argc;                          /**< Number of arguments */
    uint32_t blinklist;                    /**< Port list for blinking the led */
    uint32_t flags;                        /**< Flag values */
    volatile int force_quit;               /**< Flag to force quit */
    struct cmdline *cl;                    /**< Command Line information pointer */
    char *argv[64];                        /**< Argument list */
    char *hostname;                        /**< hostname */
    int32_t socket_port;                   /**< port number */
    volatile uint8_t timer_running;        /**< flag to denote timer is running */
    uint16_t ident;                        /**< IPv4 ident value */
    uint16_t last_row;                     /**< last static row of the screen */
    uint16_t nb_ports;                     /**< Number of ports in the system */
    uint8_t starting_port;                 /**< Starting port to display */
    uint8_t ending_port;                   /**< Ending port to display */
    uint8_t nb_ports_per_page;             /**< Number of ports to display per page */
    uint16_t mbuf_dataroom;                /**< Size of data room in mbuf */
    uint16_t mbuf_buf_size;                /**< MBUF default buf size */
    uint16_t mbuf_headroom;                /**< Size of headroom in mbuf */
    uint16_t nb_rxd;                       /**< Number of receive descriptors */
    uint16_t nb_txd;                       /**< Number of transmit descriptors */
    uint16_t curr_port;                    /**< Current Port number */
    uint64_t hz;                           /**< Number of cycles per seconds */
    uint64_t page_timeout;                 /**< Timeout for page update */
    uint64_t stats_timeout;                /**< Timeout for stats update */
    uint64_t max_total_ipackets;           /**< Total Max seen input packet rate */
    uint64_t max_total_opackets;           /**< Total Max seen output packet rate */
    uint64_t counter;                      /**< A debug counter */
    uint64_t mem_used;                     /**< Display memory used counters per ports */
    uint64_t total_mem_used;               /**< Display memory used for all ports */
    struct rte_eth_stats cumm_rate_totals; /**< port rates total values */

#ifdef LUA_ENABLED
    luaData_t *ld;      /**< General Lua Data pointer */
    luaData_t *ld_sock; /**< Info for Lua Socket */
    pthread_t thread;   /**< Thread structure for Lua server */
#endif

    uint8_t *portdesc[RTE_MAX_ETHPORTS];   /**< Port descriptions from lspci */
    uint32_t portdesc_cnt;                 /**< Number of ports in portdesc array */
    lscpu_t *lscpu;                        /**< CPU information */
    capture_t capture[RTE_MAX_NUMA_NODES]; /**< Packet capture, 1 struct per socket */
} pktgen_t;

enum {                                    /* Pktgen flags bits */
       PRINT_LABELS_FLAG      = (1 << 0), /**< Print constant labels on stats display */
       MAC_FROM_ARP_FLAG      = (1 << 1), /**< Configure the SRC MAC from a ARP request */
       PROMISCUOUS_ON_FLAG    = (1 << 2), /**< Enable promiscuous mode */
       NUMA_SUPPORT_FLAG      = (1 << 3), /**< Enable NUMA support */
       IS_SERVER_FLAG         = (1 << 4), /**< Pktgen is a Server */
       RESERVED_05            = (1 << 5),
       LUA_SHELL_FLAG         = (1 << 6), /**< Enable Lua Shell */
       TX_DEBUG_FLAG          = (1 << 7), /**< TX Debug output */
       RESERVED_8             = (1 << 8),
       RESERVED_9             = (1 << 9),
       BLINK_PORTS_FLAG       = (1 << 10), /**< Blink the port leds */
       ENABLE_THEME_FLAG      = (1 << 11), /**< Enable theme or color support */
       CLOCK_GETTIME_FLAG     = (1 << 12), /**< Enable clock_gettime() instead of rdtsc() */
       JUMBO_PKTS_FLAG        = (1 << 13), /**< Enable Jumbo frames */
       RESERVED_14            = (1 << 14),
       MAIN_PAGE_FLAG         = (1 << 15), /**< Display the main page */
       CPU_PAGE_FLAG          = (1 << 16), /**< Display the CPU page */
       SEQUENCE_PAGE_FLAG     = (1 << 17), /**< Display the Packet sequence page */
       RANGE_PAGE_FLAG        = (1 << 18), /**< Display the range page */
       RESERVED_19            = (1 << 19),
       SYSTEM_PAGE_FLAG       = (1 << 20), /**< Display the System page */
       RND_BITFIELD_PAGE_FLAG = (1 << 21), /**< Display the random bitfield page */
       LOG_PAGE_FLAG          = (1 << 22), /**< Display the message log page */
       LATENCY_PAGE_FLAG      = (1 << 23), /**< Display latency page */
       QSTATS_PAGE_FLAG       = (1 << 24), /**< Display the port queue stats */
       XSTATS_PAGE_FLAG       = (1 << 25), /**< Display the physical port stats */
       RESERVED_27            = (1 << 27),
       RESERVED_28            = (1 << 28),
       RESERVED_29            = (1 << 29),
       RESERVED_30            = (1 << 30),
       UPDATE_DISPLAY_FLAG    = (1 << 31)
};

#define UPDATE_DISPLAY_TICK_INTERVAL 4 /* check stats rate per second */
#define UPDATE_DISPLAY_TICK_RATE     (pktgen.hz / UPDATE_DISPLAY_TICK_INTERVAL)

#define PAGE_MASK_BITS                                                                          \
    (MAIN_PAGE_FLAG | CPU_PAGE_FLAG | SEQUENCE_PAGE_FLAG | RANGE_PAGE_FLAG | SYSTEM_PAGE_FLAG | \
     RND_BITFIELD_PAGE_FLAG | LOG_PAGE_FLAG | LATENCY_PAGE_FLAG | XSTATS_PAGE_FLAG |            \
     QSTATS_PAGE_FLAG)

extern pktgen_t pktgen;

void pktgen_page_display(void);

void pktgen_packet_ctor(port_info_t *pinfo, int32_t seq_idx, int32_t type);
void pktgen_packet_rate(port_info_t *pinfo);

int pktgen_find_matching_ipsrc(port_info_t *pinfo, uint32_t addr);
int pktgen_find_matching_ipdst(port_info_t *pinfo, uint32_t addr);

int pktgen_launch_one_lcore(void *arg);
void pktgen_input_start(void);
void stat_timer_dump(void);
void stat_timer_clear(void);
void pktgen_timer_setup(void);
double next_poisson_time(double rateParameter);

void pktgen_setup_packets(uint16_t pid);

static inline uint64_t
pktgen_get_time(void)
{
    if (pktgen.flags & CLOCK_GETTIME_FLAG) {
        struct timespec tp;

        if (clock_gettime(CLOCK_REALTIME, &tp) < 0)
            return rte_rdtsc();

        return rte_timespec_to_ns(&tp);
    } else
        return rte_rdtsc();
}

static inline uint64_t
pktgen_get_timer_hz(void)
{
    if (pktgen.flags & CLOCK_GETTIME_FLAG) {
        struct timespec tp = {.tv_nsec = 0, .tv_sec = 1};
        return rte_timespec_to_ns(&tp);
    } else
        return rte_get_timer_hz();
}

typedef struct {
    uint32_t magic;
    uint32_t index;
    uint64_t timestamp;
} tstamp_t;

#define TSTAMP_MAGIC 0xf00dcafe

static __inline__ void
pktgen_set_port_flags(port_info_t *pinfo, uint64_t flags)
{
    uint64_t val;

    do {
        val = rte_atomic64_read(&pinfo->port_flags);
    } while (!rte_atomic64_cmpset((volatile uint64_t *)&pinfo->port_flags.cnt, val, (val | flags)));
}

static __inline__ void
pktgen_clr_port_flags(port_info_t *pinfo, uint64_t flags)
{
    uint64_t val;

    do {
        val = rte_atomic64_read(&pinfo->port_flags);
    } while (
        !rte_atomic64_cmpset((volatile uint64_t *)&pinfo->port_flags.cnt, val, (val & ~flags)));
}

static __inline__ int
pktgen_tst_port_flags(port_info_t *pinfo, uint64_t flags)
{
    return ((rte_atomic64_read(&pinfo->port_flags) & flags) ? 1 : 0);
}

/* onOff values */
enum { DISABLE_STATE = 0, ENABLE_STATE = 1 };

static __inline__ uint32_t
estate(const char *state)
{
    return (!strcasecmp(state, "on") || !strcasecmp(state, "enable") || !strcasecmp(state, "start"))
               ? ENABLE_STATE
               : DISABLE_STATE;
}

/* LatSampler types */
enum { LATSAMPLER_UNSPEC, LATSAMPLER_SIMPLE, LATSAMPLER_POISSON };

/**
 * Function returning string of version number: "- Version:x.y.x (DPDK-x.y.z)"
 * @return
 *     string
 */
static inline const char *
pktgen_version(void)
{
    static char pkt_version[64];

    if (pkt_version[0] != 0)
        return pkt_version;

    snprintf(pkt_version, sizeof(pkt_version), "%s", PKTGEN_VERSION);
    return pkt_version;
}

static __inline__ char *
strdupf(char *str, const char *new)
{
    if (str)
        free(str);
    return (new == NULL) ? NULL : strdup(new);
}

/**
 *
 * do_command - Internal function to execute a shell command and grab the output.
 *
 * DESCRIPTION
 * Internal function to execute a shell command and grab the output from the
 * command.
 *
 * RETURNS: Number of lines read.
 *
 * SEE ALSO:
 */

static __inline__ int
do_command(const char *cmd, int (*display)(char *, int))
{
    FILE *f;
    int i;
    char *line       = NULL;
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
        pclose(f);
    if (line)
        free(line);

    return i;
}

#ifndef MEMPOOL_F_DMA
#define MEMPOOL_F_DMA 0
#endif

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_H_ */
