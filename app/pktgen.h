/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_H_
#define _PKTGEN_H_

/**
 * @file
 *
 * Core pktgen header: global state, port-flag manipulation, and master control macros.
 *
 * Includes all DPDK and pktgen sub-headers, declares the global pktgen_t struct,
 * defines the per-port flag bits, packet-slot indices, display enumerations, and
 * provides inline helpers for atomic port-flag operations and time acquisition.
 */

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

#define MAX_MATRIX_ENTRIES 128 /**< Maximum number of lcore/port mapping entries */
#define MAX_STRING         256 /**< Maximum length of a mapping string */
#define Million            (uint64_t)(1000000UL)    /**< One million (10^6) */
#define Billion            (uint64_t)(1000000000UL) /**< One billion (10^9) */

/** Compute total input bits including Ethernet physical overhead. */
#define iBitsTotal(_x) (uint64_t)(((_x.ipackets * PKT_OVERHEAD_SIZE) + _x.ibytes) * 8)
/** Compute total output bits including Ethernet physical overhead. */
#define oBitsTotal(_x) (uint64_t)(((_x.opackets * PKT_OVERHEAD_SIZE) + _x.obytes) * 8)

/** Execute @p _exp in a do-while(0) to allow use in if-else without braces. */
#define _do(_exp) \
    do {          \
        _exp;     \
    } while ((0))

#ifndef RTE_ETH_FOREACH_DEV
#define RTE_ETH_FOREACH_DEV(p) for (_p = 0; _p < pktgen.nb_ports; _p++)
#endif

/**
 * Iterate over all active ports and execute @p _action for each.
 *
 * Declares local variables @c pid and @c pinfo; @p _action may reference
 * both.  Ports whose seq_pkt array is NULL are skipped.
 */
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

/**
 * Iterate over ports selected by a portlist bitmask and execute @p _action.
 *
 * Declares local variables @c pid and @c pinfo; @p _action may reference
 * both.  Ports not in @p _portlist or with a NULL seq_pkt are skipped.
 *
 * @param _portlist  Portlist bitmask (uint64_t array).
 * @param _action    Statement to execute for each selected port.
 */
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

/** Packet processing outcome code returned by the RX handler. */
typedef enum {
    PACKET_CONSUMED = 0,      /**< Packet was handled internally by pktgen */
    UNKNOWN_PACKET  = 0xEEEE, /**< Packet type was not recognised */
    DROP_PACKET     = 0xFFFE, /**< Packet should be dropped */
    FREE_PACKET     = 0xFFFF  /**< Packet buffer should be freed */
} pktType_e;

enum {
    MAX_SCRN_ROWS = 44,  /**< Maximum number of terminal rows used */
    MAX_SCRN_COLS = 132, /**< Maximum number of terminal columns used */

    COLUMN_WIDTH_0 = 22, /**< Width of display column 0 */
    COLUMN_WIDTH_1 = 24, /**< Width of display column 1 */
    COLUMN_WIDTH_3 = 24, /**< Width of display column 3 */

    DEFAULT_MAX_TX_LAG = 20000, /**< Max TX lag cycles; ideally make configurable */

    /* Row locations for start of data */
    PAGE_TITLE_ROWS = 1,  /**< Number of rows for the page title */
    PORT_FLAGS_ROWS = 1,  /**< Number of rows for port flags */
    LINK_STATE_ROWS = 1,  /**< Number of rows for link state */
    PKT_RATE_ROWS   = 7,  /**< Number of rows for packet rate stats */
    PKT_SIZE_ROWS   = 10, /**< Number of rows for packet size stats */
    PKT_TOTALS_ROWS = 7,  /**< Number of rows for packet total counters */
    IP_ADDR_ROWS    = 12, /**< Number of rows for IP address display */

    PAGE_TITLE_ROW = 1,                                  /**< Row for page title */
    PORT_FLAGS_ROW = (PAGE_TITLE_ROW + PAGE_TITLE_ROWS), /**< Row for port flags */
    LINK_STATE_ROW = (PORT_FLAGS_ROW + PORT_FLAGS_ROWS), /**< Row for link state */
    PKT_RATE_ROW   = (LINK_STATE_ROW + LINK_STATE_ROWS), /**< Row for packet rates */
    PKT_SIZE_ROW   = (PKT_RATE_ROW + PKT_RATE_ROWS),     /**< Row for packet sizes */
    PKT_TOTALS_ROW = (PKT_SIZE_ROW + PKT_SIZE_ROWS),     /**< Row for packet totals */
    IP_ADDR_ROW    = (PKT_TOTALS_ROW + PKT_TOTALS_ROWS), /**< Row for IP addresses */

    DEFAULT_NETMASK        = 0xFFFFFF00,                /**< Default network mask (/24) */
    DEFAULT_IP_ADDR        = (192 << 24) | (168 << 16), /**< Default source IP (192.168.0.0) */
    DEFAULT_TX_COUNT       = 0,                         /**< Default TX count (0 = forever) */
    DEFAULT_TX_RATE        = 100,                       /**< Default TX rate (100%) */
    DEFAULT_PRIME_COUNT    = 1,                         /**< Default prime burst count */
    DEFAULT_SRC_PORT       = 1234,                      /**< Default source L4 port */
    DEFAULT_DST_PORT       = 5678,                      /**< Default destination L4 port */
    DEFAULT_TTL            = 64,                        /**< Default IP TTL */
    DEFAULT_TCP_SEQ_NUMBER = 0x12378,                   /**< Default TCP initial sequence number */
    MAX_TCP_SEQ_NUMBER     = UINT32_MAX / 8,            /**< Maximum TCP sequence number */
    DEFAULT_TCP_ACK_NUMBER = 0x12390,                   /**< Default TCP initial ack number */
    MAX_TCP_ACK_NUMBER     = UINT32_MAX / 8,            /**< Maximum TCP acknowledgement number */
    DEFAULT_TCP_FLAGS      = ACK_FLAG,                  /**< Default TCP flags (ACK) */
    DEFAULT_WND_SIZE       = 8192,                      /**< Default TCP window size */
    MIN_VLAN_ID            = 1,                         /**< Minimum valid VLAN ID */
    MAX_VLAN_ID            = 4095,                      /**< Maximum valid VLAN ID */
    DEFAULT_VLAN_ID        = MIN_VLAN_ID,               /**< Default VLAN ID */
    MIN_COS                = 0,                         /**< Minimum CoS value */
    MAX_COS                = 7,                         /**< Maximum CoS value */
    DEFAULT_COS            = MIN_COS,                   /**< Default CoS value */
    MIN_TOS                = 0,                         /**< Minimum ToS value */
    MAX_TOS                = 255,                       /**< Maximum ToS value */
    DEFAULT_TOS            = MIN_TOS,                   /**< Default ToS value */
    MAX_ETHER_TYPE_SIZE    = 0x600,                     /**< Maximum Ethernet type field value */
    OVERHEAD_FUDGE_VALUE   = 50,                        /**< Fudge factor for overhead estimates */

    DEFAULT_PORTS_PER_PAGE = 4, /**< Default number of ports shown per display page */
    VLAN_TAG_SIZE          = 4, /**< Size of an 802.1Q VLAN tag in bytes */
    MAX_PRIME_COUNT        = 4, /**< Maximum prime-burst packet count */

    NUM_SEQ_PKTS = 16, /**< Number of sequence packet slots per port */

    FIRST_SEQ_PKT  = 0,                              /**< First sequence slot index */
    SINGLE_PKT     = (FIRST_SEQ_PKT + NUM_SEQ_PKTS), /**< Single-packet template (slot 16) */
    SPECIAL_PKT    = (SINGLE_PKT + 1),               /**< Scratch/special packet (slot 17) */
    RANGE_PKT      = (SPECIAL_PKT + 1),              /**< Range-mode template (slot 18) */
    LATENCY_PKT    = (RANGE_PKT + 1),                /**< Latency probe packet (slot 19) */
    NUM_TOTAL_PKTS = (LATENCY_PKT + 1),              /**< Total per-port packet slots */

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

/** Compute the wire size of a packet including all physical overhead. */
#define WIRE_SIZE(pkt_size, t) (t)(pkt_size + PKT_OVERHEAD_SIZE)

/** Convenience typedef for struct rte_mbuf. */
typedef struct rte_mbuf rte_mbuf_t;

/** Union allowing an Ethernet address to be accessed as a 64-bit integer. */
typedef union {
    struct rte_ether_addr addr; /**< Ethernet address structure */
    uint64_t u64;               /**< Raw 64-bit representation */
} ethaddr_t;

/** Global Pktgen application state: settings, port references, and display parameters. */
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

enum {                                     /* Pktgen flags bits */
       PRINT_LABELS_FLAG      = (1 << 0),  /**< Print constant labels on stats display */
       MAC_FROM_ARP_FLAG      = (1 << 1),  /**< Configure the SRC MAC from a ARP request */
       PROMISCUOUS_ON_FLAG    = (1 << 2),  /**< Enable promiscuous mode */
       NUMA_SUPPORT_FLAG      = (1 << 3),  /**< Enable NUMA support */
       IS_SERVER_FLAG         = (1 << 4),  /**< Pktgen is a Server */
       RESERVED_05            = (1 << 5),  /**< Reserved */
       LUA_SHELL_FLAG         = (1 << 6),  /**< Enable Lua Shell */
       TX_DEBUG_FLAG          = (1 << 7),  /**< TX Debug output */
       RESERVED_8             = (1 << 8),  /**< Reserved */
       RESERVED_9             = (1 << 9),  /**< Reserved */
       BLINK_PORTS_FLAG       = (1 << 10), /**< Blink the port leds */
       ENABLE_THEME_FLAG      = (1 << 11), /**< Enable theme or color support */
       CLOCK_GETTIME_FLAG     = (1 << 12), /**< Enable clock_gettime() instead of rdtsc() */
       JUMBO_PKTS_FLAG        = (1 << 13), /**< Enable Jumbo frames */
       RESERVED_14            = (1 << 14), /**< Reserved */
       MAIN_PAGE_FLAG         = (1 << 15), /**< Display the main page */
       CPU_PAGE_FLAG          = (1 << 16), /**< Display the CPU page */
       SEQUENCE_PAGE_FLAG     = (1 << 17), /**< Display the Packet sequence page */
       RANGE_PAGE_FLAG        = (1 << 18), /**< Display the range page */
       RESERVED_19            = (1 << 19), /**< Reserved */
       SYSTEM_PAGE_FLAG       = (1 << 20), /**< Display the System page */
       RND_BITFIELD_PAGE_FLAG = (1 << 21), /**< Display the random bitfield page */
       LOG_PAGE_FLAG          = (1 << 22), /**< Display the message log page */
       LATENCY_PAGE_FLAG      = (1 << 23), /**< Display latency page */
       QSTATS_PAGE_FLAG       = (1 << 24), /**< Display the port queue stats */
       XSTATS_PAGE_FLAG       = (1 << 25), /**< Display the physical port stats */
       RESERVED_27            = (1 << 27), /**< Reserved */
       RESERVED_28            = (1 << 28), /**< Reserved */
       RESERVED_29            = (1 << 29), /**< Reserved */
       RESERVED_30            = (1 << 30), /**< Reserved */
       UPDATE_DISPLAY_FLAG    = (1 << 31)  /**< Trigger a display refresh */
};

#define UPDATE_DISPLAY_TICK_INTERVAL 4 /**< Display stat refresh checks per second */
#define UPDATE_DISPLAY_TICK_RATE     (pktgen.hz / UPDATE_DISPLAY_TICK_INTERVAL) /**< Tick rate */

/** Bitmask of all page-select flags used to clear the current page before switching. */
#define PAGE_MASK_BITS                                                                          \
    (MAIN_PAGE_FLAG | CPU_PAGE_FLAG | SEQUENCE_PAGE_FLAG | RANGE_PAGE_FLAG | SYSTEM_PAGE_FLAG | \
     RND_BITFIELD_PAGE_FLAG | LOG_PAGE_FLAG | LATENCY_PAGE_FLAG | XSTATS_PAGE_FLAG |            \
     QSTATS_PAGE_FLAG)

/** The global pktgen application state singleton. */
extern pktgen_t pktgen;

/** Redraw the currently active display page. */
void pktgen_page_display(void);

/**
 * Construct a packet template in a sequence slot.
 *
 * @param pinfo    Per-port state.
 * @param seq_idx  Packet slot index (0 .. NUM_TOTAL_PKTS-1).
 * @param type     Protocol type override (-1 to use existing).
 */
void pktgen_packet_ctor(port_info_t *pinfo, int32_t seq_idx, int32_t type);

/**
 * Recalculate the inter-burst TX cycle count for a port's target rate.
 *
 * @param pinfo  Per-port state.
 */
void pktgen_packet_rate(port_info_t *pinfo);

/**
 * Test whether an IPv4 address matches the port's source IP.
 *
 * @return  Non-zero if @p addr matches.
 */
int pktgen_find_matching_ipsrc(port_info_t *pinfo, uint32_t addr);

/**
 * Test whether an IPv4 address matches the port's destination IP.
 *
 * @return  Non-zero if @p addr matches.
 */
int pktgen_find_matching_ipdst(port_info_t *pinfo, uint32_t addr);

/**
 * Worker function launched on each lcore.
 *
 * @param arg  Unused (required by rte_eal_remote_launch prototype).
 * @return     0 on clean exit.
 */
int pktgen_launch_one_lcore(void *arg);

/** Start the interactive CLI input loop on the main lcore. */
void pktgen_input_start(void);

/** Dump the per-port timer statistics to the log. */
void stat_timer_dump(void);

/** Clear the accumulated per-port timer statistics. */
void stat_timer_clear(void);

/** Set up the periodic stat-update and display-refresh timers. */
void pktgen_timer_setup(void);

/**
 * Rebuild packet templates for all queues on a port.
 *
 * @param pid  Port ID.
 */
void pktgen_setup_packets(uint16_t pid);

/**
 * Return the current time value using the configured time source.
 *
 * Returns nanoseconds via clock_gettime(CLOCK_REALTIME) when
 * CLOCK_GETTIME_FLAG is set, otherwise returns rte_rdtsc() cycles.
 *
 * @return
 *   Current time in nanoseconds or TSC cycles.
 */
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

/**
 * Return the timer frequency for the configured time source.
 *
 * @return
 *   1,000,000,000 when using clock_gettime(), otherwise rte_get_timer_hz().
 */
static inline uint64_t
pktgen_get_timer_hz(void)
{
    if (pktgen.flags & CLOCK_GETTIME_FLAG) {
        struct timespec tp = {.tv_nsec = 0, .tv_sec = 1};
        return rte_timespec_to_ns(&tp);
    } else
        return rte_get_timer_hz();
}

/** Latency probe packet payload header. */
typedef struct {
    uint32_t magic;     /**< Magic value (TSTAMP_MAGIC) for probe identification */
    uint32_t index;     /**< Sequence index used to match TX/RX probes */
    uint64_t timestamp; /**< TSC cycle count at transmit time */
} tstamp_t;

#define TSTAMP_MAGIC 0xf00dcafe /**< Magic value identifying a latency probe packet */

/**
 * Atomically OR a set of flags into a port's port_flags atomic.
 *
 * @param pinfo  Per-port state.
 * @param flags  Bitmask of SEND_* / port-flag bits to set.
 */
static __inline__ void
pktgen_set_port_flags(port_info_t *pinfo, uint64_t flags)
{
    uint64_t val;

    do {
        val = rte_atomic64_read(&pinfo->port_flags);
    } while (!rte_atomic64_cmpset((volatile uint64_t *)&pinfo->port_flags.cnt, val, (val | flags)));
}

/**
 * Atomically clear a set of flags from a port's port_flags atomic.
 *
 * @param pinfo  Per-port state.
 * @param flags  Bitmask of SEND_* / port-flag bits to clear.
 */
static __inline__ void
pktgen_clr_port_flags(port_info_t *pinfo, uint64_t flags)
{
    uint64_t val;

    do {
        val = rte_atomic64_read(&pinfo->port_flags);
    } while (
        !rte_atomic64_cmpset((volatile uint64_t *)&pinfo->port_flags.cnt, val, (val & ~flags)));
}

/**
 * Test whether any of the given flag bits are set in a port's port_flags.
 *
 * @param pinfo  Per-port state.
 * @param flags  Bitmask to test.
 * @return
 *   Non-zero if any bit in @p flags is currently set.
 */
static __inline__ int
pktgen_tst_port_flags(port_info_t *pinfo, uint64_t flags)
{
    return ((rte_atomic64_read(&pinfo->port_flags) & flags) ? 1 : 0);
}

/** Enable/disable state values for CLI and command arguments. */
enum { DISABLE_STATE = 0, /**< Disabled */ ENABLE_STATE = 1 /**< Enabled */ };

/**
 * Parse an on/off/enable/disable/start state string.
 *
 * @param state  "on", "enable", or "start" → ENABLE_STATE; else → DISABLE_STATE.
 * @return
 *   ENABLE_STATE or DISABLE_STATE.
 */
static __inline__ uint32_t
estate(const char *state)
{
    return (!strcasecmp(state, "on") || !strcasecmp(state, "enable") || !strcasecmp(state, "start"))
               ? ENABLE_STATE
               : DISABLE_STATE;
}

/** Latency sampler algorithm selection. */
enum {
    LATSAMPLER_UNSPEC, /**< Unspecified (use default) */
    LATSAMPLER_SIMPLE, /**< Uniform random sampling */
    LATSAMPLER_POISSON /**< Poisson-rate sampling */
};

/**
 * Return the pktgen version string.
 *
 * @return
 *   Pointer to a static buffer containing the version string.
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

/**
 * Free an existing string and duplicate a new one.
 *
 * @param str  Existing string to free (may be NULL).
 * @param new  String to duplicate (may be NULL, returns NULL in that case).
 * @return
 *   Newly duplicated string, or NULL if @p new is NULL.
 */
static __inline__ char *
strdupf(char *str, const char *new)
{
    if (str)
        free(str);
    return (new == NULL) ? NULL : strdup(new);
}

/**
 * Execute a shell command and display its output line-by-line.
 *
 * @param cmd
 *   Shell command string to execute via popen().
 * @param display
 *   Callback invoked for each output line; receives the line and an
 *   accumulating counter, returns the updated counter.
 * @return
 *   Final value returned by @p display, or -1 on popen() failure.
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
