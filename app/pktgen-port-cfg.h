/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_PORT_CFG_H_
#define _PKTGEN_PORT_CFG_H_

#include <stdio.h>
#include <string.h>
#include <rte_version.h>
#include <rte_atomic.h>
#include <rte_pci.h>
#ifdef TX_DEBUG_PKT_DUMP
#include "rte_hexdump.h"
#endif

#undef BPF_MAJOR_VERSION
#include <pcap/pcap.h>

#include "pktgen-seq.h"
#include "pktgen-range.h"
#include "pktgen-stats.h"
#include "pktgen-pcap.h"
#include "pktgen-dump.h"
#include "pktgen-ether.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USER_PATTERN_SIZE 16
#define MAX_LATENCY_ENTRIES \
    50108        // Max 101000?, limited by max allowed size of latsamp_stats_t.data[]
#define MAX_LATENCY_QUEUES 10

// clang-format off
enum { /* Per port flag bits */
       /* Supported packet modes non-exclusive */
       SEND_ARP_REQUEST         = (1ULL << 0), /**< Send a ARP request */
       SEND_GRATUITOUS_ARP      = (1ULL << 1), /**< Send a Gratuitous ARP */
       ICMP_ECHO_ENABLE_FLAG    = (1ULL << 2), /**< Enable ICMP Echo support */
       BONDING_TX_PACKETS       = (1ULL << 3), /**< Bonding driver send zero pkts */

       /* Receive packet modes */
       PROCESS_INPUT_PKTS       = (1ULL << 4), /**< Process input packets */
       CAPTURE_PKTS             = (1ULL << 5), /**< Capture received packets */
       SAMPLING_LATENCIES       = (1ULL << 6), /**< Sampling latency measurements */

       SEND_PING4_REQUEST       = (1ULL << 8), /**< Send a IPv4 Ping request */
       SEND_PING6_REQUEST       = (1ULL << 9), /**< Send a IPv6 Ping request */

       /* Exclusive Packet sending modes */
       SEND_SINGLE_PKTS         = (1ULL << 12), /**< Send single packets */
       SEND_PCAP_PKTS           = (1ULL << 13), /**< Send a pcap file of packets */
       SEND_RANGE_PKTS          = (1ULL << 14), /**< Send range of packets */
       SEND_SEQ_PKTS            = (1ULL << 15), /**< Send sequence of packets */

       /* Exclusive Packet type modes */
       SEND_RANDOM_PKTS         = (1ULL << 16), /**< Send random bitfields in packets */
       SEND_VLAN_ID             = (1ULL << 17), /**< Send packets with VLAN ID */
       SEND_MPLS_LABEL          = (1ULL << 18), /**< Send MPLS label */
       SEND_Q_IN_Q_IDS          = (1ULL << 19), /**< Send packets with Q-in-Q */
       
       SEND_GRE_IPv4_HEADER     = (1ULL << 20), /**< Encapsulate IPv4 in GRE */
       SEND_GRE_ETHER_HEADER    = (1ULL << 21), /**< Encapsulate Ethernet frame in GRE */
       SEND_VXLAN_PACKETS       = (1ULL << 22), /**< Send VxLAN Packets */
       SEND_LATENCY_PKTS        = (1ULL << 23), /**< Send latency packets in any mode */

       /* Sending flags */
       SETUP_TRANSMIT_PKTS      = (1ULL << 28), /**< Need to setup transmit packets */
       STOP_RECEIVING_PACKETS   = (1ULL << 29), /**< Stop receiving packet */
       SENDING_PACKETS          = (1ULL << 30), /**< sending packets on this port */
       SEND_FOREVER             = (1ULL << 31), /**< Send packets forever */

       SEND_ARP_PING_REQUESTS   =
           (SEND_ARP_REQUEST | SEND_GRATUITOUS_ARP | SEND_PING4_REQUEST | SEND_PING6_REQUEST)
};
#define RANDOMIZE_SRC_IP (1ULL << 32) /**< Set the source IP address as random */
#define RANDOMIZE_SRC_PT (1ULL << 33) /**< Set the source port as random */
// clang-format on

#define EXCLUSIVE_MODES (SEND_SINGLE_PKTS | SEND_PCAP_PKTS | SEND_RANGE_PKTS | SEND_SEQ_PKTS)

#define EXCLUSIVE_PKT_MODES                                                                       \
    (SEND_RANDOM_PKTS | SEND_VLAN_ID | SEND_MPLS_LABEL | SEND_Q_IN_Q_IDS | SEND_GRE_IPv4_HEADER | \
     SEND_GRE_ETHER_HEADER | SEND_VXLAN_PACKETS | SEND_LATENCY_PKTS)

#define RTE_PMD_PARAM_UNSET -1

typedef enum {
    ZERO_FILL_PATTERN = 1,
    ABC_FILL_PATTERN,
    USER_FILL_PATTERN,
    NO_FILL_PATTERN,
} fill_t;

typedef void (*tx_func_t)(struct port_info_s *info, uint16_t qid);

typedef struct {
    uint64_t data[MAX_LATENCY_ENTRIES]; /** Record for latencies */
    uint32_t idx;                       /**< Index to the latencies array */
    uint64_t next;                      /**< Next latency entry */
    uint64_t pkt_counter;               /**< Pkt counter */
    uint32_t num_samples;               /**< Number of latency samples */
} latsamp_stats_t __rte_cache_aligned;

typedef struct {
    uint64_t latency_rate_us;         /**< number micro-seconds between injecting packets */
    uint64_t jitter_threshold_us;     /**< Jitter threshold in micro-seconds */
    uint64_t jitter_threshold_cycles; /**< Jitter threshold cycles */
    uint64_t latency_rate_cycles;     /**< Number of cycles between injections */
    uint64_t latency_timo_cycles;     /**< Number of cycles to next latency injection */
    uint16_t latency_entropy;         /**< Entropy value to be used to increment sport */
    MARKER stats;                     /* starting Marker to clear stats */
    uint64_t jitter_count;            /**< Number of jitter stats */
    uint64_t num_latency_pkts;        /**< Total number of latency packets */
    uint64_t num_latency_tx_pkts;     /**< Total number of TX latency packets */
    uint64_t num_skipped;             /**< Number of skipped latency packets */
    uint64_t running_cycles;          /**< Running, Number of cycles per latency packet */
    uint64_t prev_cycles;             /**< previous cycles cyles time from last latency packet */
    uint64_t min_cycles;              /**< minimum cycles per latency packet */
    uint64_t avg_cycles;              /**< average cycles per latency packet */
    uint64_t max_cycles;              /**< maximum cycles per latency packet */
    uint32_t next_index;              /**< Next index to use for sending latency packets */
    uint32_t expect_index;            /**< Expected index for received latency packets */
    MARKER end_stats;
} latency_t;

typedef struct port_info_s {
    rte_atomic64_t port_flags;        /**< Special send flags for ARP and other */
    rte_atomic64_t transmit_count;    /**< Packets to transmit loaded into current_tx_count */
    rte_atomic64_t current_tx_count;  /**< Current number of packets to send */
    volatile uint64_t tx_cycles;      /**< Number cycles between TX bursts */
    uint16_t pid;                     /**< Port ID value */
    uint16_t tx_burst;                /**< Number of TX burst packets */
    uint16_t lsc_enabled;             /**< Enable link state change */
    uint16_t rx_burst;                /**< RX burst size */
    uint64_t tx_pps;                  /**< Transmit packets per seconds */
    uint64_t tx_count;                /**< Total count of tx attempts */
    uint64_t delta;                   /**< Delta value for latency testing */
    double tx_rate;                   /**< Percentage rate for tx packets with fractions */
    struct rte_eth_link link;         /**< Link Information like speed and duplex */
    struct rte_eth_dev_info dev_info; /**< PCI info + driver name */
    struct rte_ether_addr src_mac;    /**< Source MAC address of the port */

    /* Packet buffer space for traffic generator, shared for all packets per port */
    uint16_t seqIdx;     /**< Current Packet sequence index 0 to NUM_SEQ_PKTS */
    uint16_t seqCnt;     /**< Current packet sequence max count */
    uint16_t prime_cnt;  /**< Set the number of packets to send in a prime command */
    uint16_t vlanid;     /**< Set the port VLAN ID value */
    uint8_t cos;         /**< Set the port 802.1p cos value */
    uint8_t tos;         /**< Set the port tos value */
    pkt_seq_t *seq_pkt;  /**< Sequence of packets seq_pkt[NUM_SEQ_PKTS]=default packet */
    range_info_t range;  /**< Range Information */
    uint32_t mpls_entry; /**< Set the port MPLS entry */
    uint32_t gre_key;    /**< GRE key if used */

    struct rnd_bits_s *rnd_bitfields;     /**< Random bitfield settings */
    char user_pattern[USER_PATTERN_SIZE]; /**< User set pattern values */
    fill_t fill_pattern_type;             /**< Type of pattern to fill with */

    /** Whether the pseudo-header is required when calculating the checksum.
     *  Depends on the original NIC driver (e.g., ixgbe NICs expect the pseudo-header)
     *  See Table 1.133: https://doc.dpdk.org/guides/nics/overview.html */
    bool cksum_requires_phdr;

    union {
        uint64_t vxlan; /**< VxLAN 64 bit word */
        struct {
            uint16_t vni_flags; /**< VxLAN Flags */
            uint16_t group_id;  /**< VxLAN Group Policy ID */
            uint32_t vxlan_id;  /**< VxLAN VNI */
        };
    };

    struct rte_eth_stats curr_stats;  /**< current port statistics */
    struct rte_eth_stats queue_stats; /**< current port queue statistics */
    struct rte_eth_stats rate_stats;  /**< current packet rate statistics */
    struct rte_eth_stats prev_stats;  /**< previous port statistics */
    struct rte_eth_stats base_stats;  /**< base port statistics */
    pkt_stats_t pkt_stats;            /**< Statistics for a number of stats */
    pkt_sizes_t pkt_sizes;            /**< Stats for the different packet sizes */
    uint64_t max_ipackets;            /**< Max seen input packet rate */
    uint64_t max_opackets;            /**< Max seen output packet rate */
    uint64_t max_missed;              /**< Max missed packets seen */
    uint64_t qcnt[RTE_ETHDEV_QUEUE_STAT_CNTRS];
    uint64_t prev_qcnt[RTE_ETHDEV_QUEUE_STAT_CNTRS];

    /* Packet dump related */
    struct packet {
        void *data;   /**< Packet data */
        uint32_t len; /**< Length of data */
    } dump_list[MAX_DUMP_PACKETS];
    uint8_t dump_head;  /**< Index of last packet written to screen */
    uint8_t dump_tail;  /**< Index of last valid packet in dump_list */
    uint8_t dump_count; /**< Number of packets the user requested */

    /* Latency sampling data */
    /* Depending on MAX_LATENCY_ENTRIES, this could blow up static array memory usage
     * over the limit allowed by x86_64 architecture */
    latency_t latency;                                 /**< Latency information */
    latsamp_stats_t latsamp_stats[MAX_LATENCY_QUEUES]; /**< Per core stats */
    uint32_t latsamp_type;                             /**< Type of lat sampler  */
    uint32_t latsamp_rate;        /**< Sampling rate i.e., samples per second  */
    uint32_t latsamp_num_samples; /**< Number of samples to collect  */
    char latsamp_outfile[256];    /**< Path to file for dumping latency samples */
    FILE *pcap_file;              /**< PCAP file handle */
} port_info_t;

struct vxlan {
    uint16_t vni_flags; /**< VxLAN Flags */
    uint16_t group_id;  /**< VxLAN Group Policy ID */
    uint32_t vxlan_id;  /**< VxLAN VNI */
};

void pktgen_config_ports(void);
void tx_send_packets(port_info_t *pinfo, uint16_t qid, struct rte_mbuf **pkts, uint16_t nb_pkts);

/**
 * Atomically subtract a 64-bit value from the tx counter.
 *
 * @param v
 *   A pointer to the atomic tx counter.
 * @param burst
 *   The value to be subtracted from the counter for tx burst size.
 * @return
 *   The number of packets to burst out
 */
static inline uint64_t
pkt_atomic64_tx_count(rte_atomic64_t *v, int64_t burst)
{
    int success;
    int64_t tmp2;

    do {
        int64_t tmp1 = v->cnt;

        if (tmp1 == 0)
            return 0;
        tmp2    = likely(tmp1 > burst) ? burst : tmp1;
        success = rte_atomic64_cmpset((volatile uint64_t *)&v->cnt, tmp1, tmp1 - tmp2);
    } while (success == 0);

    return tmp2;
}

static inline void
rte_eth_rxconf_dump(FILE *f, struct rte_eth_rxconf *rx)
{
    fprintf(f, "  RX Conf:\n");
    fprintf(f,
            "     pthresh        :%5" PRIu16 " hthresh          :%5" PRIu16
            " wthresh        :%5" PRIu16 "\n",
            rx->rx_thresh.pthresh, rx->rx_thresh.hthresh, rx->rx_thresh.wthresh);
    fprintf(f,
            "     Free Thresh    :%5" PRIu16 " Drop Enable      :%5" PRIu16
            " Deferred Start :%5" PRIu16 "\n",
            rx->rx_free_thresh, rx->rx_drop_en, rx->rx_deferred_start);
    fprintf(f, "     offloads       :%016" PRIx64 "\n", rx->offloads);
}

static inline void
rte_eth_txconf_dump(FILE *f, struct rte_eth_txconf *tx)
{
    fprintf(f, "  TX Conf:\n");
    fprintf(f,
            "     pthresh        :%5" PRIu16 " hthresh          :%5" PRIu16
            " wthresh        :%5" PRIu16 "\n",
            tx->tx_thresh.pthresh, tx->tx_thresh.hthresh, tx->tx_thresh.wthresh);
    fprintf(f,
            "     Free Thresh    :%5" PRIu16 " RS Thresh        :%5" PRIu16
            " Deferred Start :%5" PRIu16 "\n",
            tx->tx_free_thresh, tx->tx_rs_thresh, tx->tx_deferred_start);
    fprintf(f, "     offloads       :%016" PRIx64 "\n", tx->offloads);
}

static inline void
rte_eth_desc_lim_dump(FILE *f, struct rte_eth_desc_lim *lim, int tx_flag)
{
    fprintf(f, "  %s: descriptor Limits\n", tx_flag ? "Tx" : "Rx");

    fprintf(f,
            "     nb_max         :%5" PRIu16 "  nb_min          :%5" PRIu16
            "  nb_align      :%5" PRIu16 "\n",
            lim->nb_max, lim->nb_min, lim->nb_align);
    fprintf(f, "     nb_seg_max     :%5" PRIu16 "  nb_mtu_seg_max  :%5" PRIu16 "\n",
            lim->nb_seg_max, lim->nb_mtu_seg_max);
}

static inline void
rte_eth_dev_portconf_dump(FILE *f, struct rte_eth_dev_portconf *conf, int tx_flag)
{
    fprintf(f, "  %s: Port Config\n", tx_flag ? "Tx" : "Rx");

    fprintf(f,
            "     burst_size     :%5" PRIu16 "  ring_size       :%5" PRIu16
            "  nb_queues     :%5" PRIu16 "\n",
            conf->burst_size, conf->ring_size, conf->nb_queues);
}

static inline void
rte_eth_switch_info_dump(FILE *f, struct rte_eth_switch_info *sw)
{
    fprintf(f, "  Switch Info: %s\n", sw->name);

    fprintf(f, "     domain_id      :%5" PRIu16 "  port_id         :%5" PRIu16 "\n", sw->domain_id,
            sw->port_id);
}

static inline int
rte_get_rx_capa_list(uint64_t rx_capa, char *buf, size_t len)
{
    uint32_t i;
    int ret;
#define _(x) #x
    struct {
        uint64_t flag;
        const char *name;
    } rx_flags[] = {{RTE_ETH_RX_OFFLOAD_VLAN_STRIP, _(VLAN_STRIP)},
                    {RTE_ETH_RX_OFFLOAD_IPV4_CKSUM, _(IPV4_CKSUM)},
                    {RTE_ETH_RX_OFFLOAD_UDP_CKSUM, _(UDP_CKSUM)},
                    {RTE_ETH_RX_OFFLOAD_TCP_CKSUM, _(TCP_CKSUM)},
                    {RTE_ETH_RX_OFFLOAD_TCP_LRO, _(TCP_LRO)},
                    {RTE_ETH_RX_OFFLOAD_QINQ_STRIP, _(QINQ_STRIP)},
                    {RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM, _(OUTER_IPV4_CKSUM)},
                    {RTE_ETH_RX_OFFLOAD_MACSEC_STRIP, _(MACSEC_STRIP)},
                    {RTE_ETH_RX_OFFLOAD_VLAN_FILTER, _(VLAN_FILTER)},
                    {RTE_ETH_RX_OFFLOAD_VLAN_EXTEND, _(VLAN_EXTEND)},
                    {RTE_ETH_RX_OFFLOAD_SCATTER, _(SCATTER)},
                    {RTE_ETH_RX_OFFLOAD_TIMESTAMP, _(TIMESTAMP)},
                    {RTE_ETH_RX_OFFLOAD_SECURITY, _(SECURITY)},
                    {RTE_ETH_RX_OFFLOAD_KEEP_CRC, _(KEEP_CRC)},
                    {RTE_ETH_RX_OFFLOAD_SCTP_CKSUM, _(SCTP_CKSUM)},
                    {RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM, _(OUTER_UDP_CKSUM)},
                    {RTE_ETH_RX_OFFLOAD_RSS_HASH, _(RSS_HASH)},
                    {RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT, _(BUFFER_SPLIT)}};
#undef _

    if (len == 0)
        return -1;

    buf[0] = '\0';
    for (i = 0; i < RTE_DIM(rx_flags); i++) {
        if ((rx_capa & rx_flags[i].flag) != rx_flags[i].flag)
            continue;

        ret = snprintf(buf, len, "%s ", rx_flags[i].name);
        if (ret < 0)
            return -1;
        if ((size_t)ret >= len)
            return -1;
        buf += ret;
        len -= ret;
    }
    return 0;
}

static inline int
rte_get_tx_capa_list(uint64_t tx_capa, char *buf, size_t len)
{
    uint32_t i;
    int ret;
#define _(x) #x
    struct {
        uint64_t flag;
        const char *name;
    } tx_flags[] = {
        {RTE_ETH_TX_OFFLOAD_VLAN_INSERT, _(VLAN_INSERT)},
        {RTE_ETH_TX_OFFLOAD_IPV4_CKSUM, _(IPV4_CKSUM)},
        {RTE_ETH_TX_OFFLOAD_UDP_CKSUM, _(UDP_CKSUM)},
        {RTE_ETH_TX_OFFLOAD_TCP_CKSUM, _(TCP_CKSUM)},
        {RTE_ETH_TX_OFFLOAD_SCTP_CKSUM, _(SCTP_CKSUM)},
        {RTE_ETH_TX_OFFLOAD_TCP_TSO, _(TCP_TSO)},
        {RTE_ETH_TX_OFFLOAD_UDP_TSO, _(UDP_TSO)},
        {RTE_ETH_TX_OFFLOAD_OUTER_IPV4_CKSUM, _(OUTER_IPV4_CKSUM)},
        {RTE_ETH_TX_OFFLOAD_QINQ_INSERT, _(QINQ_INSERT)},
        {RTE_ETH_TX_OFFLOAD_VXLAN_TNL_TSO, _(VXLAN_TNL_TSO)},
        {RTE_ETH_TX_OFFLOAD_GRE_TNL_TSO, _(GRE_TNL_TSO)},
        {RTE_ETH_TX_OFFLOAD_IPIP_TNL_TSO, _(IPIP_TNL_TSO)},
        {RTE_ETH_TX_OFFLOAD_GENEVE_TNL_TSO, _(GENEVE_TNL_TSO)},
        {RTE_ETH_TX_OFFLOAD_MACSEC_INSERT, _(MACSEC_INSERT)},
        {RTE_ETH_TX_OFFLOAD_MT_LOCKFREE, _(MT_LOCKFREE)},
        {RTE_ETH_TX_OFFLOAD_MULTI_SEGS, _(MULTI_SEGS)},
        {RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE, _(MBUF_FAST_FREE)},
        {RTE_ETH_TX_OFFLOAD_SECURITY, _(SECURITY)},
        {RTE_ETH_TX_OFFLOAD_UDP_TNL_TSO, _(UDP_TNL_TSO)},
        {RTE_ETH_TX_OFFLOAD_IP_TNL_TSO, _(IP_TNL_TSO)},
        {RTE_ETH_TX_OFFLOAD_OUTER_UDP_CKSUM, _(OUTER_UDP_CKSUM)},
    };
#undef _

    if (len == 0)
        return -1;

    buf[0] = '\0';
    for (i = 0; i < RTE_DIM(tx_flags); i++) {
        if ((tx_capa & tx_flags[i].flag) != tx_flags[i].flag)
            continue;

        ret = snprintf(buf, len, "%s ", tx_flags[i].name);
        if (ret < 0)
            return -1;
        if ((size_t)ret >= len)
            return -1;
        buf += ret;
        len -= ret;
    }
    return 0;
}

static inline void
rte_eth_dev_info_dump(FILE *f, uint16_t pid)
{
    struct rte_eth_dev_info dev_info;
    struct rte_eth_dev_info *di = &dev_info;
    char buf[512];

    if (rte_eth_dev_info_get(pid, &dev_info) < 0) {
        fprintf(f, "Failed to get eth dev info for port %u\n", pid);
        return;
    }

    if (!f)
        f = stderr;

    char dev_name[64];

    rte_eth_dev_get_name_by_port(pid, dev_name);
    fprintf(f, "** Device Info (%s, if_index:%" PRId32 ", flags %08" PRIu32 ") **\n", dev_name,
            di->if_index, *di->dev_flags);

    fprintf(f,
            "   min_rx_bufsize :%5" PRIu32 "  max_rx_pktlen     :%5" PRIu32
            "  hash_key_size :%5" PRIu8 "\n",
            di->min_rx_bufsize, di->max_rx_pktlen, di->hash_key_size);
    fprintf(f,
            "   max_rx_queues  :%5" PRIu16 "  max_tx_queues     :%5" PRIu16
            "  max_vfs       :%5" PRIu16 "\n",
            di->max_rx_queues, di->max_tx_queues, di->max_vfs);
    fprintf(f,
            "   max_mac_addrs  :%5" PRIu32 "  max_hash_mac_addrs:%5" PRIu32
            "  max_vmdq_pools:%5" PRIu16 "\n",
            di->max_mac_addrs, di->max_hash_mac_addrs, di->max_vmdq_pools);
    fprintf(f,
            "   vmdq_queue_base:%5" PRIu16 "  vmdq_queue_num    :%5" PRIu16
            "  vmdq_pool_base:%5" PRIu16 "\n",
            di->vmdq_queue_base, di->vmdq_queue_num, di->vmdq_pool_base);
    fprintf(f,
            "   nb_rx_queues   :%5" PRIu16 "  nb_tx_queues      :%5" PRIu16
            "  speed_capa    : %08" PRIx32 "\n",
            di->nb_rx_queues, di->nb_tx_queues, di->speed_capa);
    fprintf(f, "\n");
    fprintf(f, "   flow_type_rss_offloads:%016" PRIx64 "  reta_size             :%5" PRIu16 "\n",
            di->flow_type_rss_offloads, di->reta_size);
    rte_get_rx_capa_list(di->rx_offload_capa, buf, sizeof(buf));
    fprintf(f, "   rx_offload_capa       :%s\n", buf);
    rte_get_tx_capa_list(di->tx_offload_capa, buf, sizeof(buf));
    fprintf(f, "   tx_offload_capa       :%s\n", buf);
    fprintf(f, "   rx_queue_offload_capa :%016" PRIx64, di->rx_queue_offload_capa);
    fprintf(f, "  tx_queue_offload_capa :%016" PRIx64 "\n", di->tx_queue_offload_capa);
    fprintf(f, "   dev_capa              :%016" PRIx64 "\n", di->dev_capa);
    fprintf(f, "\n");

    rte_eth_rxconf_dump(f, &di->default_rxconf);
    rte_eth_txconf_dump(f, &di->default_txconf);

    rte_eth_desc_lim_dump(f, &di->rx_desc_lim, 0);
    rte_eth_desc_lim_dump(f, &di->tx_desc_lim, 1);

    rte_eth_dev_portconf_dump(f, &di->default_rxportconf, 0);
    rte_eth_dev_portconf_dump(f, &di->default_txportconf, 1);

    rte_eth_switch_info_dump(f, &di->switch_info);

    fprintf(f, "\n");
}

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_PORT_CFG_H_ */
