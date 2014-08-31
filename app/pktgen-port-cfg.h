/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *	 notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *	 notice, this list of conditions and the following disclaimer in
 *	 the documentation and/or other materials provided with the
 *	 distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *	 contributors may be used to endorse or promote products derived
 *	 from this software without specific prior written permission.
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
 *	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ windriver.com */

#ifndef _PKTGEN_PORT_CFG_H_
#define _PKTGEN_PORT_CFG_H_

#include <rte_atomic.h>

#undef BPF_MAJOR_VERSION
#include <pcap/pcap.h>

#include "pktgen-seq.h"
#include "pktgen-range.h"
#include "pktgen-stats.h"
#include "pktgen-pcap.h"
#include "pktgen-dump.h"
#include "pktgen-ether.h"
#include "pktgen-random.h"


#define MAX_PORT_DESC_SIZE	132


typedef struct port_sizes_s {
	uint64_t			_64;				/**< Number of 64 byte packets */
	uint64_t			_65_127;			/**< Number of 65-127 byte packets */
	uint64_t			_128_255;			/**< Number of 128-255 byte packets */
	uint64_t			_256_511;			/**< Number of 256-511 byte packets */
	uint64_t			_512_1023;			/**< Number of 512-1023 byte packets */
	uint64_t			_1024_1518;			/**< Number of 1024-1518 byte packets */
	uint64_t			broadcast;			/**< Number of broadcast packets */
	uint64_t			multicast;			/**< Number of multicast packets */
	uint64_t			jumbo;				/**< Number of Jumbo frames */
	uint64_t			runt;				/**< Number of Runt frames */
} port_sizes_t;


struct mbuf_table {
	unsigned len;
	struct rte_mbuf *m_table[DEFAULT_PKT_BURST+1];
};


enum {		// Per port flag bits
	SEND_ARP_REQUEST		= 0x00000001,		/**< Send a ARP request */
	SEND_GRATUITOUS_ARP		= 0x00000002,		/**< Send a Gratuitous ARP */
	ICMP_ECHO_ENABLE_FLAG	= 0x00000004,		/**< Enable ICMP Echo support */
	SEND_PCAP_PKTS			= 0x00000008,		/**< Send a pcap file of packets */
	SEND_RANGE_PKTS			= 0x00000010,		/**< Send a range of packets */
	SEND_SEQ_PKTS			= 0x00000020,		/**< Send a sequence of packets */
	PROCESS_INPUT_PKTS		= 0x00000040,		/**< Process input packets */
	SEND_PING4_REQUEST		= 0x00000080,		/**< Send a IPv4 Ping request */
	SEND_PING6_REQUEST		= 0x00000100,		/**< Send a IPv6 Ping request */
	SEND_ARP_PING_REQUESTS	= (SEND_ARP_REQUEST | SEND_GRATUITOUS_ARP | SEND_PING4_REQUEST | SEND_PING6_REQUEST),
	PROCESS_RX_TAP_PKTS		= 0x00000200,		/**< Handle RX TAP interface packets */
	PROCESS_TX_TAP_PKTS		= 0x00000400,		/**< Handle TX TAP interface packets */
	SEND_VLAN_ID			= 0x00000800,		/**< Send packets with VLAN ID */
	PROCESS_GARP_PKTS		= 0x00001000,		/**< Process GARP packets and update the dst MAC address */
	CAPTURE_PKTS			= 0x00002000,		/**< Capture received packets */
	SEND_MPLS_LABEL			= 0x00004000,		/**< Send MPLS label */
	SEND_Q_IN_Q_IDS			= 0x00008000,		/**< Send packets with Q-in-Q */
	SEND_GRE_IPv4_HEADER	= 0x00010000,		/**< Encapsulate IPv4 in GRE */
	SEND_RANDOM_PKTS		= 0x00020000,		/**< Send random bitfields in packets */
	SEND_GRE_ETHER_HEADER	= 0x00040000,		/**< Encapsulate Ethernet frame in GRE */
	SENDING_PACKETS			= 0x80000000		/**< sending packets on this port */
};

typedef struct port_info_s {
	uint16_t				pid;				/**< Port ID value */
	uint16_t				tx_burst;			/**< Number of TX burst packets */
	uint8_t					pad0;
	uint8_t					tx_rate;			/**< Percentage rate for tx packets */
	rte_atomic32_t			port_flags;			/**< Special send flags for ARP and other */

	rte_atomic64_t			transmit_count;		/**< Packets to transmit loaded into current_tx_count */
	rte_atomic64_t			current_tx_count;	/**< Current number of packets to send */
	uint64_t				tx_cycles;			/**< Number cycles between TX bursts */
	uint64_t				tx_pps;				/**< Transmit packets per seconds */
	uint64_t				delta;				/**< Delta value for latency testing */
	uint64_t				tx_count;			/**< Total count of tx attempts */

	// Packet buffer space for traffic generator, shared for all packets per port
	uint16_t				seqIdx;				/**< Current Packet sequence index 0 to NUM_SEQ_PKTS */
	uint16_t				seqCnt;				/**< Current packet sequence max count */
	uint16_t				prime_cnt;			/**< Set the number of packets to send in a prime command */
	uint16_t				vlanid;				/**< Set the port VLAN ID value */
	pkt_seq_t			  * seq_pkt;			/**< Sequence of packets seq_pkt[NUM_SEQ_PKTS]=default packet */
	range_info_t			range;				/**< Range Information */

	uint32_t				mpls_entry;			/**< Set the port MPLS entry */
	uint16_t				qinq_outerid;		/**< Set the port outer VLAN ID value for Q-in-Q */
	uint16_t				qinq_innerid;		/**< Set the port inner VLAN ID value for Q-in-Q */
	uint32_t				gre_key;			/**< GRE key if used */

	uint16_t				nb_mbufs;			/**< Number of mbufs in the system */
	uint16_t				pad1;

	pkt_stats_t				stats;				/**< Statistics for a number of stats */
	port_sizes_t			sizes;				/**< Stats for the different packets sizes */

	eth_stats_t				init_stats;			/**< Initial packet statistics */
	eth_stats_t				port_stats;			/**< current port statistics */
	eth_stats_t				rate_stats;			/**< current packet rate statistics */

	struct rte_eth_link		link;				/**< Link Information like speed and duplex */

	struct q_info {
		rte_atomic32_t		 flags;				/**< Special send flags for ARP and other */
		struct mbuf_table	 tx_mbufs;			/**< mbuf holder for transmit packets */
		struct rte_mempool * rx_mp;				/**< Pool pointer for port RX mbufs */
		struct rte_mempool * tx_mp;				/**< Pool pointer for default TX mbufs */
		struct rte_mempool * range_mp;			/**< Pool pointer for port Range TX mbufs */
		struct rte_mempool * seq_mp;			/**< Pool pointer for port Sequence TX mbufs */
		struct rte_mempool * pcap_mp;			/**< Pool pointer for port PCAP TX mbufs */
		struct rte_mempool * special_mp;		/**< Pool pointer for special TX mbufs */
	} q[NUM_Q];

	int32_t					rx_tapfd;			/**< Rx Tap file descriptor */
	int32_t					tx_tapfd;			/**< Tx Tap file descriptor */
	pcap_info_t			  * pcap;				/**< PCAP information header */
	uint64_t				pcap_cycles;		/**< number of cycles for pcap sending */

	int32_t					pcap_result;		/**< PCAP result of filter compile */
	struct bpf_program		pcap_program;		/**< PCAP filter program structure */

	// Packet dump related
	struct packet {
		void *		data;						/**< Packet data */
		uint32_t	len;						/**< Length of data */
	} dump_list[MAX_DUMP_PACKETS];
	uint8_t					dump_head;			/**< Index of last packet written to screen */
	uint8_t					dump_tail;			/**< Index of last valid packet in dump_list */
	uint8_t					dump_count;			/**< Number of packets the user requested */

	struct rnd_bits_s	  * rnd_bitfields;		/**< Random bitfield settings */
} port_info_t;


extern void pktgen_config_ports(void);

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
	int success = 0;
	int64_t tmp1, tmp2;

	while (success == 0) {
		tmp1 = v->cnt;
		tmp2 = (tmp1 > burst) ? burst : tmp1;
		success = rte_atomic64_cmpset((volatile uint64_t *)&v->cnt,
		                              tmp1, tmp1 - tmp2);
	}
	return tmp2;
}


#endif	// _PKTGEN_PORT_CFG_H_
