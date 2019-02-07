/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
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
#include <rte_spinlock.h>
#include <rte_pci.h>

#undef BPF_MAJOR_VERSION
#include <pcap/pcap.h>

#include "pktgen-seq.h"
#include "pktgen-range.h"
#include "pktgen-stats.h"
#include "pktgen-pcap.h"
#include "pktgen-dump.h"
#include "pktgen-ether.h"
#include "pktgen-random.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PORT_DESC_SIZE  132
#define USER_PATTERN_SIZE   16

typedef struct port_sizes_s {
	uint64_t _64;		/**< Number of 64 byte packets */
	uint64_t _65_127;	/**< Number of 65-127 byte packets */
	uint64_t _128_255;	/**< Number of 128-255 byte packets */
	uint64_t _256_511;	/**< Number of 256-511 byte packets */
	uint64_t _512_1023;	/**< Number of 512-1023 byte packets */
	uint64_t _1024_1518;	/**< Number of 1024-1518 byte packets */
	uint64_t broadcast;	/**< Number of broadcast packets */
	uint64_t multicast;	/**< Number of multicast packets */
	uint64_t jumbo;		/**< Number of Jumbo frames */
	uint64_t runt;		/**< Number of Runt frames */
	uint64_t unknown;	/**< Number of unknown sizes */
} port_sizes_t;

struct mbuf_table {
	uint16_t len;
	struct rte_mbuf *m_table[DEFAULT_PKT_BURST];
};

enum {						/* Per port flag bits */
	SEND_ARP_REQUEST        = 0x00000001,	/**< Send a ARP request */
	SEND_GRATUITOUS_ARP     = 0x00000002,	/**< Send a Gratuitous ARP */
	ICMP_ECHO_ENABLE_FLAG   = 0x00000004,	/**< Enable ICMP Echo support */
	SEND_PCAP_PKTS          = 0x00000008,	/**< Send a pcap file of packets */
	SEND_RANGE_PKTS         = 0x00000010,	/**< Send a range of packets */
	SEND_SEQ_PKTS           = 0x00000020,	/**< Send a sequence of packets */
	PROCESS_INPUT_PKTS      = 0x00000040,	/**< Process input packets */
	SEND_PING4_REQUEST      = 0x00000080,	/**< Send a IPv4 Ping request */
	SEND_PING6_REQUEST      = 0x00000100,	/**< Send a IPv6 Ping request */
	PROCESS_RX_TAP_PKTS     = 0x00000200,	/**< Handle RX TAP interface packets */
	PROCESS_TX_TAP_PKTS     = 0x00000400,	/**< Handle TX TAP interface packets */
	SEND_VLAN_ID            = 0x00000800,	/**< Send packets with VLAN ID */
	PROCESS_GARP_PKTS       = 0x00001000,	/**< Process GARP packets and update the dst MAC address */
	CAPTURE_PKTS            = 0x00002000,	/**< Capture received packets */
	SEND_MPLS_LABEL         = 0x00004000,	/**< Send MPLS label */
	SEND_Q_IN_Q_IDS         = 0x00008000,	/**< Send packets with Q-in-Q */
	SEND_GRE_IPv4_HEADER    = 0x00010000,	/**< Encapsulate IPv4 in GRE */
	SEND_RANDOM_PKTS        = 0x00020000,	/**< Send random bitfields in packets */
	SEND_GRE_ETHER_HEADER   = 0x00040000,	/**< Encapsulate Ethernet frame in GRE */
	SEND_LATENCY_PKTS       = 0x00080000,	/**< Send latency packets */
	BONDING_TX_PACKETS	= 0x00100000,	/**< Bonding driver send zero pkts */
	SEND_SHORT_PACKETS	= 0x00200000,	/**< Allow port to send short packets */
	SEND_VXLAN_PACKETS	= 0x00400000,	/**< Send VxLAN Packets */
	SENDING_PACKETS         = 0x40000000,	/**< sending packets on this port */
	SEND_FOREVER            = 0x80000000,	/**< Send packets forever */
	SEND_ARP_PING_REQUESTS  =
		(SEND_ARP_REQUEST | SEND_GRATUITOUS_ARP | SEND_PING4_REQUEST |
		 SEND_PING6_REQUEST)
};

#define RTE_PMD_PARAM_UNSET -1

/*
 * Configurable values of RX and TX ring threshold registers.
 */
typedef struct ring_conf_s {
	int8_t rx_pthresh;
	int8_t rx_hthresh;
	int8_t rx_wthresh;

	int8_t tx_pthresh;
	int8_t tx_hthresh;
	int8_t tx_wthresh;

	/*
	 * Configurable value of RX free threshold.
	 */
	int16_t rx_free_thresh;

	/*
	 * Configurable value of RX drop enable.
	 */
	int8_t rx_drop_en;

	/*
	 * Configurable value of TX free threshold.
	 */
	int16_t tx_free_thresh;

	/*
	 * Configurable value of TX RS bit threshold.
	 */
	int16_t tx_rs_thresh;

#if RTE_VERSION <= RTE_VERSION_NUM(18, 5, 0, 0)
	/*
	 * Configurable value of TX queue flags.
	 */
	int32_t txq_flags;
#endif

	/*
	 * Receive Side Scaling (RSS) configuration.
	 */
	uint64_t rss_hf;
} ring_conf_t;

typedef enum {
	ZERO_FILL_PATTERN = 1,
	ABC_FILL_PATTERN,
	USER_FILL_PATTERN,
	NO_FILL_PATTERN,
} fill_t;

typedef void (*tx_func_t)(struct port_info_s *info, uint16_t qid);

typedef struct port_info_s {
	uint16_t pid;		/**< Port ID value */
	uint16_t tx_burst;	/**< Number of TX burst packets */
	double tx_rate;		/**< Percentage rate for tx packets with fractions */
	rte_atomic32_t port_flags;	/**< Special send flags for ARP and other */

	rte_atomic64_t transmit_count;	/**< Packets to transmit loaded into current_tx_count */
	rte_atomic64_t current_tx_count;/**< Current number of packets to send */
	uint64_t tx_cycles;	/**< Number cycles between TX bursts */
	uint64_t tx_pps;	/**< Transmit packets per seconds */
	uint64_t delta;		/**< Delta value for latency testing */
	uint64_t tx_count;	/**< Total count of tx attempts */

	/* Packet buffer space for traffic generator, shared for all packets per port */
	uint16_t seqIdx;	/**< Current Packet sequence index 0 to NUM_SEQ_PKTS */
	uint16_t seqCnt;	/**< Current packet sequence max count */
	uint16_t prime_cnt;	/**< Set the number of packets to send in a prime command */
	uint16_t vlanid;	/**< Set the port VLAN ID value */
	uint8_t cos;		/**< Set the port 802.1p cos value */
	uint8_t tos;		/**< Set the port tos value */
	rte_spinlock_t port_lock;/**< Used to sync up packet constructor between cores */
	pkt_seq_t *seq_pkt;	/**< Sequence of packets seq_pkt[NUM_SEQ_PKTS]=default packet */
	range_info_t range;	/**< Range Information */

	uint32_t mpls_entry;	/**< Set the port MPLS entry */
	uint32_t gre_key;	/**< GRE key if used */

	uint16_t nb_mbufs;	/**< Number of mbufs in the system */
	uint64_t max_latency;	/**< TX Latency sequence */
	uint64_t avg_latency;	/**< Latency delta in clock ticks */
	uint64_t min_latency;	/**< RX Latency sequence */

	RTE_STD_C11
	union {
		uint64_t vxlan;		/**< VxLAN 64 bit word */
		struct {
			uint16_t vni_flags;	/**< VxLAN Flags */
			uint16_t group_id;	/**< VxLAN Group Policy ID */
			uint32_t vxlan_id;	/**< VxLAN VNI */
		};
	};

	uint32_t magic_errors;
	uint32_t latency_nb_pkts;
	uint64_t jitter_threshold;
	uint64_t jitter_threshold_clks;
	uint64_t jitter_count;
	uint64_t prev_latency;

	pkt_stats_t stats;	/**< Statistics for a number of stats */
	port_sizes_t sizes;	/**< Stats for the different packets sizes */

	eth_stats_t curr_stats;	/**< current port statistics */
	eth_stats_t prev_stats;	/**< previous port statistics */
	eth_stats_t rate_stats;	/**< current packet rate statistics */
	uint64_t max_ipackets;	/**< Max seen input packet rate */
	uint64_t max_opackets;	/**< Max seen output packet rate */
	uint64_t max_missed;	/**< Max missed packets seen */

	struct rte_eth_link link;	/**< Link Information like speed and duplex */

	struct q_info {
		rte_atomic32_t flags;		/**< Special send flags for ARP and other */
		struct mbuf_table tx_mbufs;	/**< mbuf holder for transmit packets */
		struct rte_mempool *rx_mp;	/**< Pool pointer for port RX mbufs */
		struct rte_mempool *tx_mp;	/**< Pool pointer for default TX mbufs */
		struct rte_mempool *range_mp;	/**< Pool pointer for port Range TX mbufs */
		struct rte_mempool *seq_mp;	/**< Pool pointer for port Sequence TX mbufs */
		struct rte_mempool *pcap_mp;	/**< Pool pointer for port PCAP TX mbufs */
		struct rte_mempool *special_mp;	/**< Pool pointer for special TX mbufs */
	} q[NUM_Q];

	int32_t rx_tapfd;	/**< Rx Tap file descriptor */
	int32_t tx_tapfd;	/**< Tx Tap file descriptor */
	pcap_info_t *pcap;	/**< PCAP information header */
	pcap_info_t *pcaps[NUM_Q];	/**< Per Tx queue PCAP information headers */
	uint64_t pcap_cycles;	/**< number of cycles for pcap sending */

	int32_t pcap_result;	/**< PCAP result of filter compile */
	struct bpf_program pcap_program;/**< PCAP filter program structure */

	/* Packet dump related */
	struct packet {
		void *data;	/**< Packet data */
		uint32_t len;	/**< Length of data */
	} dump_list[MAX_DUMP_PACKETS];
	uint8_t dump_head;	/**< Index of last packet written to screen */
	uint8_t dump_tail;	/**< Index of last valid packet in dump_list */
	uint8_t dump_count;	/**< Number of packets the user requested */

	struct rnd_bits_s     *rnd_bitfields;	/**< Random bitfield settings */

	struct rte_eth_dev_info dev_info;	/**< PCI info + driver name */
	char user_pattern[USER_PATTERN_SIZE];	/**< User set pattern values */
	fill_t fill_pattern_type;		/**< Type of pattern to fill with */
} port_info_t;

struct vxlan {
	uint16_t vni_flags;	/**< VxLAN Flags */
	uint16_t group_id;	/**< VxLAN Group Policy ID */
	uint32_t vxlan_id;	/**< VxLAN VNI */
};

void pktgen_config_ports(void);

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
		tmp2 = likely(tmp1 > burst) ? burst : tmp1;
		success = rte_atomic64_cmpset((volatile uint64_t *)&v->cnt,
					      tmp1,
					      tmp1 - tmp2);
	} while (success == 0);

	return tmp2;
}

static inline void
rte_eth_rxconf_dump(FILE *f, struct rte_eth_rxconf *rx)
{
	fprintf(f, "  RX Conf:\n");
	fprintf(f,
		"     pthresh        :%5" PRIu16 " hthresh          :%5" PRIu16 " wthresh        :%5" PRIu16 "\n",
		rx->rx_thresh.pthresh,
		rx->rx_thresh.hthresh,
		rx->rx_thresh.wthresh);
	fprintf(f,
		"     Free Thresh    :%5" PRIu16 " Drop Enable      :%5" PRIu16 " Deferred Start :%5" PRIu16 "\n",
		rx->rx_free_thresh,
		rx->rx_drop_en,
		rx->rx_deferred_start);
	fprintf(f,
		"     offloads       :%016" PRIx64 "\n",
		rx->offloads);
}

static inline void
rte_eth_txconf_dump(FILE *f, struct rte_eth_txconf *tx)
{
	fprintf(f, "  TX Conf:\n");
	fprintf(f,
		"     pthresh        :%5" PRIu16 " hthresh          :%5" PRIu16 " wthresh        :%5" PRIu16 "\n",
		tx->tx_thresh.pthresh,
		tx->tx_thresh.hthresh,
		tx->tx_thresh.wthresh);
#if RTE_VERSION <= RTE_VERSION_NUM(18, 5, 0, 0)
	fprintf(f,
		"     Free Thresh    :%5" PRIu16 " RS Thresh        :%5" PRIu16 " Deferred Start :%5" PRIu16 "  TXQ Flags: %08x\n",
		tx->tx_free_thresh,
		tx->tx_rs_thresh,
		tx->tx_deferred_start,
		tx->txq_flags);
#else
	fprintf(f,
		"     Free Thresh    :%5" PRIu16 " RS Thresh        :%5" PRIu16 " Deferred Start :%5" PRIu16 "\n",
		tx->tx_free_thresh,
		tx->tx_rs_thresh,
		tx->tx_deferred_start);
#endif
	fprintf(f,
		"     offloads       :%016" PRIx64 "\n",
		tx->offloads);
}

static inline void
rte_eth_desc_lim_dump(FILE *f, struct rte_eth_desc_lim *lim, int tx_flag)
{
	fprintf(f, "  %s: descriptor Limits\n", tx_flag ? "Tx" : "Rx");

	fprintf(f,
		"     nb_max         :%5" PRIu16 "  nb_min          :%5" PRIu16 "  nb_align      :%5" PRIu16 "\n",
		lim->nb_max, lim->nb_min, lim->nb_align);
	fprintf(f,
		"     nb_seg_max     :%5" PRIu16 "  nb_mtu_seg_max  :%5" PRIu16 "\n",
		lim->nb_seg_max, lim->nb_mtu_seg_max);
}

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
static inline void
rte_eth_dev_portconf_dump(FILE *f, struct rte_eth_dev_portconf *conf, int tx_flag)
{
	fprintf(f, "  %s: Port Config\n", tx_flag ? "Tx" : "Rx");

	fprintf(f,
		"     burst_size     :%5" PRIu16 "  ring_size       :%5" PRIu16 "  nb_queues     :%5" PRIu16 "\n",
		conf->burst_size, conf->ring_size, conf->nb_queues);
}
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
static inline void
rte_eth_switch_info_dump(FILE *f, struct rte_eth_switch_info *sw)
{
	fprintf(f, "  Switch Info: %s\n", sw->name);

	fprintf(f,
		"     domain_id      :%5" PRIu16 "  port_id         :%5" PRIu16 "\n",
		sw->domain_id, sw->port_id);
}
#endif

static inline int
rte_get_rx_capa_list(uint64_t rx_capa, char *buf, size_t len)
{
	uint32_t i;
	int ret;
#define _(x)	#x
	struct {
		uint64_t flag;
		const char *name;
	} rx_flags[] = {
	{ DEV_RX_OFFLOAD_VLAN_STRIP,		_(VLAN_STRIP) },
	{ DEV_RX_OFFLOAD_IPV4_CKSUM,		_(IPV4_CKSUM) },
	{ DEV_RX_OFFLOAD_UDP_CKSUM,		_(UDP_CKSUM) },
	{ DEV_RX_OFFLOAD_TCP_CKSUM,		_(TCP_CKSUM) },
	{ DEV_RX_OFFLOAD_TCP_LRO,		_(TCP_LRO) },
	{ DEV_RX_OFFLOAD_QINQ_STRIP,		_(QINQ_STRIP) },
	{ DEV_RX_OFFLOAD_OUTER_IPV4_CKSUM,	_(OUTER_IPV4_CKSUM) },
	{ DEV_RX_OFFLOAD_MACSEC_STRIP,		_(MACSEC_STRIP) },
	{ DEV_RX_OFFLOAD_HEADER_SPLIT,		_(HEADER_SPLIT) },
	{ DEV_RX_OFFLOAD_VLAN_FILTER,		_(VLAN_FILTER) },
	{ DEV_RX_OFFLOAD_VLAN_EXTEND,		_(VLAN_EXTEND) },
	{ DEV_RX_OFFLOAD_JUMBO_FRAME,		_(JUMBO_FRAME) },
	{ DEV_RX_OFFLOAD_SCATTER,		_(SCATTER) },
	{ DEV_RX_OFFLOAD_TIMESTAMP,		_(TIMESTAMP) },
	{ DEV_RX_OFFLOAD_SECURITY,		_(SECURITY) },
	{ DEV_RX_OFFLOAD_KEEP_CRC,		_(KEEP_CRC) },
	{ DEV_RX_OFFLOAD_SCTP_CKSUM,		_(SCTP_CKSUM) },
	{ DEV_RX_OFFLOAD_OUTER_UDP_CKSUM, 	_(OUTER_UDP_CKSUM) }
	};
#undef _

	if (len == 0)
		return -1;

	buf[0] = '\0';
	for(i = 0; i < RTE_DIM(rx_flags); i++) {
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
#define _(x)	#x
	struct {
		uint64_t flag;
		const char *name;
	} tx_flags[] = {
	{ DEV_TX_OFFLOAD_VLAN_INSERT,		_(VLAN_INSERT) },
	{ DEV_TX_OFFLOAD_IPV4_CKSUM, 		_(IPV4_CKSUM) },
	{ DEV_TX_OFFLOAD_UDP_CKSUM, 		_(UDP_CKSUM) },
	{ DEV_TX_OFFLOAD_TCP_CKSUM, 		_(TCP_CKSUM) },
	{ DEV_TX_OFFLOAD_SCTP_CKSUM, 		_(SCTP_CKSUM) },
	{ DEV_TX_OFFLOAD_TCP_TSO, 		_(TCP_TSO) },
	{ DEV_TX_OFFLOAD_UDP_TSO, 		_(UDP_TSO) },
	{ DEV_TX_OFFLOAD_OUTER_IPV4_CKSUM,	_(OUTER_IPV4_CKSUM) },
	{ DEV_TX_OFFLOAD_QINQ_INSERT, 		_(QINQ_INSERT) },
	{ DEV_TX_OFFLOAD_VXLAN_TNL_TSO, 	_(VXLAN_TNL_TSO) },
	{ DEV_TX_OFFLOAD_GRE_TNL_TSO, 		_(GRE_TNL_TSO) },
	{ DEV_TX_OFFLOAD_IPIP_TNL_TSO, 		_(IPIP_TNL_TSO) },
	{ DEV_TX_OFFLOAD_GENEVE_TNL_TSO, 	_(GENEVE_TNL_TSO) },
	{ DEV_TX_OFFLOAD_MACSEC_INSERT, 	_(MACSEC_INSERT) },
	{ DEV_TX_OFFLOAD_MT_LOCKFREE, 		_(MT_LOCKFREE) },
	{ DEV_TX_OFFLOAD_MULTI_SEGS, 		_(MULTI_SEGS) },
	{ DEV_TX_OFFLOAD_MBUF_FAST_FREE,	_(MBUF_FAST_FREE) },
	{ DEV_TX_OFFLOAD_SECURITY, 		_(SECURITY) },
	{ DEV_TX_OFFLOAD_UDP_TNL_TSO, 		_(UDP_TNL_TSO) },
	{ DEV_TX_OFFLOAD_IP_TNL_TSO, 		_(IP_TNL_TSO) },
	{ DEV_TX_OFFLOAD_OUTER_UDP_CKSUM,	_(OUTER_UDP_CKSUM) },
	{ DEV_TX_OFFLOAD_MATCH_METADATA,	_(MATCH_METADATA) },
	};
#undef _

	if (len == 0)
		return -1;

	buf[0] = '\0';
	for(i = 0; i < RTE_DIM(tx_flags); i++) {
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

	rte_eth_dev_info_get(pid, &dev_info);

	if (!f)
		f = stderr;

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
	fprintf(f, "** Device Info (%s, if_index:%" PRId32 ", flags %08" PRIu32 ") **\n",
		rte_eth_devices[pid].data->name, di->if_index, *di->dev_flags);
#else
	fprintf(f, "** Device Info (%s, if_index:%" PRId32 ") **\n",
		rte_eth_devices[pid].data->name, di->if_index);
#endif

	fprintf(f,
		"   min_rx_bufsize :%5" PRIu32 "  max_rx_pktlen     :%5" PRIu32 "  hash_key_size :%5" PRIu8 "\n",
		di->min_rx_bufsize, di->max_rx_pktlen, di->hash_key_size);
	fprintf(f,
		"   max_rx_queues  :%5" PRIu16 "  max_tx_queues     :%5" PRIu16 "  max_vfs       :%5" PRIu16 "\n",
		di->max_rx_queues, di->max_tx_queues, di->max_vfs);
	fprintf(f,
		"   max_mac_addrs  :%5" PRIu32 "  max_hash_mac_addrs:%5" PRIu32 "  max_vmdq_pools:%5" PRIu16 "\n",
		di->max_mac_addrs, di->max_hash_mac_addrs, di->max_vmdq_pools);
	fprintf(f,
		"   vmdq_queue_base:%5" PRIu16 "  vmdq_queue_num    :%5" PRIu16 "  vmdq_pool_base:%5" PRIu16 "\n",
		di->vmdq_queue_base, di->vmdq_queue_num, di->vmdq_pool_base);
	fprintf(f,
		"   nb_rx_queues   :%5" PRIu16 "  nb_tx_queues      :%5" PRIu16 "  speed_capa    : %08" PRIx32 "\n",
		di->nb_rx_queues, di->nb_tx_queues, di->speed_capa);
	fprintf(f, "\n");
	fprintf(f,
		"   flow_type_rss_offloads:%016" PRIx64 "  reta_size             :%5" PRIu16 "\n",
		di->flow_type_rss_offloads, di->reta_size);
	rte_get_rx_capa_list(di->rx_offload_capa, buf, sizeof(buf));
	fprintf(f,
		"   rx_offload_capa       :%s\n", buf);
	rte_get_tx_capa_list(di->tx_offload_capa, buf, sizeof(buf));
	fprintf(f,
		"   tx_offload_capa       :%s\n", buf);
	fprintf(f,
		"   rx_queue_offload_capa :%016" PRIx64,
		di->rx_queue_offload_capa);
	fprintf(f,
		"  tx_queue_offload_capa :%016" PRIx64 "\n",
		di->tx_queue_offload_capa);
#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
	fprintf(f,
		"   dev_capa              :%016" PRIx64 "\n",
		di->dev_capa);
#endif
	fprintf(f, "\n");

	rte_eth_rxconf_dump(f, &di->default_rxconf);
	rte_eth_txconf_dump(f, &di->default_txconf);

	rte_eth_desc_lim_dump(f, &di->rx_desc_lim, 0);
	rte_eth_desc_lim_dump(f, &di->tx_desc_lim, 1);

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
	rte_eth_dev_portconf_dump(f, &di->default_rxportconf, 0);
	rte_eth_dev_portconf_dump(f, &di->default_txportconf, 1);

	rte_eth_switch_info_dump(f, &di->switch_info);
#endif
	fprintf(f, "\n");
}

void pktgen_set_hw_strip_crc(uint8_t val);
int pktgen_get_hw_strip_crc(void);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_PORT_CFG_H_ */
