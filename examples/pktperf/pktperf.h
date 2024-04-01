/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2023 Intel Corporation
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>

#include <fgen_common.h>
#include <fgen.h>

#define PRINT(format, args...)  \
    do {                        \
        printf(format, ##args); \
        fflush(stdout);         \
    } while (0)

#define INFO_PRINT(format, args...)                                \
    do {                                                           \
        char buf[64];                                              \
        snprintf(buf, sizeof(buf), "%s(%'d)", __func__, __LINE__); \
        printf("INFO>%-24s:" format, buf, ##args);                 \
        fflush(stdout);                                            \
    } while (0)

#define ERR_PRINT(format, args...)                                 \
    do {                                                           \
        char buf[64];                                              \
        snprintf(buf, sizeof(buf), "%s(%'d)", __func__, __LINE__); \
        printf("ERROR>%-24s:" format, buf, ##args);                \
        fflush(stdout);                                            \
    } while (0)

#define DBG_PRINT(format, args...)                                     \
    do {                                                               \
        if (info->verbose) {                                           \
            char buf[64];                                              \
            snprintf(buf, sizeof(buf), "%s(%'d)", __func__, __LINE__); \
            printf("DEBUG>%-24s:" format, buf, ##args);                \
            fflush(stdout);                                            \
        }                                                              \
    } while (0)

enum {
    DEFAULT_PKT_SIZE         = 64,           /* Default packet size */
    DEFAULT_TX_RATE          = 100,          /* Default TX rate */
    DEFAULT_RX_DESC          = 1024,         /* RX descriptors by default */
    DEFAULT_TX_DESC          = 1024,         /* TX descriptors by default */
    DEFAULT_BURST_COUNT      = 32,           /* default RX/TX burst count */
    DEFAULT_TX_DRAIN_US      = 100,          /* default TX drain every ~100us */
    DEFAULT_TIMEOUT_PERIOD   = 1,            /* 1 seconds default */
    DEFAULT_PROMISCUOUS_MODE = 1,            /* default to enabled */
    DEFAULT_MBUF_COUNT       = (8 * 1024),   /* default to 16K mbufs */
    MAX_MBUF_COUNT           = (128 * 1024), /* max to 128K mbufs */
    MIN_RX_DESC              = 512,          /* Minimum number of RX descriptors */
    MIN_TX_DESC              = 512,          /* Minimum number of TX descriptors */
    MAX_RX_DESC              = 4096,         /* Maximum number of RX descriptors */
    MAX_TX_DESC              = 4096,         /* Maximum number of TX descriptors */
    MAX_QUEUES_PER_PORT      = 16,           /* Max number of queues per port */
    MAX_MAPPINGS             = 32,           /* Max number of mappings */
    MAX_TX_RATE              = 100,          /* Max TX rate percentage */
    MAX_PKT_SIZE             = 1518,         /* Maximum packet size */
    MAX_ALLOCA_SIZE          = 1024,         /* Maximum size of an allocation */
    MAX_BURST_COUNT          = 512,          /* max burst count */
    MAX_CHECK_TIME           = 40,           /* (40 * CHECK_INTERVAL) is 10s */

    RANDOM_SEED           = 0x19560630,                     /* Random seed */
    MEMPOOL_CACHE_SIZE    = RTE_MEMPOOL_CACHE_MAX_SIZE / 2, /* Size of mempool cache */
    PKT_BUFF_SIZE         = 2048,                           /* Size of packet buffers */
    MAX_TIMEOUT_PERIOD    = (60 * 60),                      /* 1 hour max */
    CHECK_INTERVAL        = 250,                            /* Link status check interval 250ms */
    INTER_FRAME_GAP       = 12,                             /* inter-frame gap in bytes */
    START_FRAME_DELIMITER = 1,                              /* Start Frame Delimiter in bytes*/
    PKT_PREAMBLE_SIZE     = 7,                              /* Packet preamble in bytes */
    PKT_OVERHEAD_SIZE =
        (INTER_FRAME_GAP + START_FRAME_DELIMITER + PKT_PREAMBLE_SIZE + RTE_ETHER_CRC_LEN),
};
#define Million (uint64_t)(1000000UL)
#define Billion (uint64_t)(1000000000UL)

enum { LCORE_MODE_UNKNOWN = 0, LCORE_MODE_RX = 1, LCORE_MODE_TX = 2, LCORE_MODE_BOTH = 3 };

typedef struct qstats_s {
    uint64_t q_ipackets[MAX_QUEUES_PER_PORT]; /** queue Rx packets. */
    uint64_t q_ibytes[MAX_QUEUES_PER_PORT];   /** successfully received queue bytes */

    uint64_t q_opackets[MAX_QUEUES_PER_PORT]; /** queue Tx packets */
    uint64_t q_obytes[MAX_QUEUES_PER_PORT];   /** successfully transmitted queue bytes */

    uint64_t q_rx_time[MAX_QUEUES_PER_PORT];    /* Cycles to receive a burst of packets */
    uint64_t q_tx_drops[MAX_QUEUES_PER_PORT];   /* Tx dropped packets per queue */
    uint64_t q_tx_time[MAX_QUEUES_PER_PORT];    /* Cycles to transmit a burst of packets */
    uint64_t q_no_txmbufs[MAX_QUEUES_PER_PORT]; /* Number of times no mbufs were allocated */
} qstats_t __rte_cache_aligned;

typedef struct pq_s { /* Port/Queue structure */
    qstats_t curr;    /* Current statistics */
    qstats_t prev;    /* Previous statistics */
    qstats_t rate;    /* Rate statistics */
} pq_t;

typedef struct l2p_port_s {
    rte_atomic16_t inited;          /* Port initialized flag */
    pthread_spinlock_t tx_lock;     /* Tx port lock */
    volatile uint16_t tx_inited;    /* Tx port initialized flag */
    uint16_t pid;                   /* Port ID attached to lcore */
    uint16_t num_rx_qids;           /* Number of Rx queues */
    uint16_t num_tx_qids;           /* Number of Tx queues */
    uint16_t mtu_size;              /* MTU size */
    uint16_t max_pkt_size;          /* Max packet size */
    uint64_t tx_cycles;             /* Tx cycles */
    uint64_t wire_size;             /* Port wire size */
    uint64_t pps;                   /* Packets per second */
    struct rte_mempool *rx_mp;      /* Rx pktmbuf mempool per queue */
    struct rte_mempool *tx_mp;      /* Tx pktmbuf mempool per queue */
    struct rte_eth_link link;       /* Port link status */
    struct rte_ether_addr mac_addr; /* MAC addresses of Port */
    struct rte_eth_stats stats;     /* Port statistics */
    struct rte_eth_stats pstats;    /* Previous port statistics */
    pq_t pq[MAX_QUEUES_PER_PORT];   /* port/queue information */
} l2p_port_t;

typedef struct l2p_lport_s { /* Each lcore has one port/queue attached */
    uint16_t mode;           /* TXPKTS_MODE_RX or TXPKTS_MODE_TX or BOTH */
    uint16_t lid;            /* Lcore ID */
    uint16_t rx_qid;         /* Queue ID attached to Rx lcore */
    uint16_t tx_qid;         /* Queue ID attached to Tx lcore */
    l2p_port_t *port;        /* Port structure */
} l2p_lport_t;

typedef struct {
    volatile bool force_quit; /* force quit flag */
    bool verbose;             /* verbose flag */
    uint16_t num_lcores;      /* number total of lcores */
    uint16_t num_ports;       /* number total of ports */
    uint16_t num_mappings;    /* number total of mappings */

    l2p_lport_t
        *lports[RTE_MAX_LCORE] __rte_cache_aligned; /* Array of lcore/port structure pointers */
    l2p_port_t ports[RTE_MAX_ETHPORTS] __rte_cache_aligned; /* Array of port structures */
    char *mappings[MAX_MAPPINGS]; /* Array of string port/queue mappings */

    /* Configuration values from command line options */
    uint32_t mbuf_count;     /* Number of mbufs to allocate per port. */
    uint16_t tx_rate;        /* packet TX rate percentage */
    uint16_t promiscuous_on; /* Ports set in promiscuous mode off by default. */
    uint16_t burst_count;    /* Burst size for RX and TX */
    uint16_t pkt_size;       /* Packet size with FCS */
    uint16_t nb_rxd;         /* number of RX descriptors */
    uint16_t nb_txd;         /* number of TX descriptors */
    uint16_t timeout_secs;   /* Statistics print timeout */
    fgen_t *fgen;            /* Packet generator */
    const char *fgen_file;   /* File to use for packet generator */
} txpkts_info_t;

extern txpkts_info_t *info;

int parse_args(int argc, char **argv);
void packet_rate(l2p_port_t *port);
void print_stats(void);
int port_setup(l2p_port_t *port);
void packet_constructor(l2p_lport_t *lport, uint8_t *pkt);
void usage(const char *prgname);

#ifdef __cplusplus
}
#endif
