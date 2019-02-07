/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_SEQ_H_
#define _PKTGEN_SEQ_H_

#include <rte_ether.h>
#include <cmdline_parse.h>
#include <cmdline_parse_ipaddr.h>
#include <pg_inet.h>

#include "pktgen-constants.h"

#ifdef __cplusplus
extern "C" {
#endif

__extension__
typedef void    *MARKER[0];   /**< generic marker for a point in a structure */

typedef struct pkt_seq_s {
	/* Packet type and information */
	struct ether_addr eth_dst_addr;	/**< Destination Ethernet address */
	struct ether_addr eth_src_addr;	/**< Source Ethernet address */

	struct cmdline_ipaddr ip_src_addr;	/**< Source IPv4 address also used for IPv6 */
	struct cmdline_ipaddr ip_dst_addr;	/**< Destination IPv4 address */
	uint32_t ip_mask;			/**< IPv4 Netmask value */

	uint16_t sport;		/**< Source port value */
	uint16_t dport;		/**< Destination port value */
	uint16_t ethType;	/**< IPv4 or IPv6 */
	uint16_t ipProto;	/**< TCP or UDP or ICMP */
	uint16_t vlanid;	/**< VLAN ID value if used */
	uint8_t cos;		/**< 802.1p cos value if used */
	uint8_t tos;		/**< tos value if used */
	uint16_t ether_hdr_size;/**< Size of Ethernet header in packet for VLAN ID */

	uint32_t mpls_entry;	/**< MPLS entry if used */
	uint16_t qinq_outerid;	/**< Outer VLAN ID if Q-in-Q */
	uint16_t qinq_innerid;	/**< Inner VLAN ID if Q-in-Q */
	uint32_t gre_key;	/**< GRE key if used */

	uint16_t pktSize;	/**< Size of packet in bytes not counting FCS */
	uint8_t seq_enabled;	/**< Enable or disable this sequence through GUI */
	uint8_t pad0;
	uint32_t gtpu_teid;	/**< GTP-U TEID, if UDP dport=2152 */

        RTE_STD_C11
        union {
                uint64_t vxlan;         	/**< VxLAN 64 bit word */
                struct {
                        uint16_t vni_flags;     /**< VxLAN Flags */
                        uint16_t group_id;      /**< VxLAN Group Policy ID */
                        uint32_t vxlan_id;	/**< VxLAN VNI */
                };
        };
	uint64_t ol_flags;	/**< offload flags */

	pkt_hdr_t hdr __rte_cache_aligned;	/**< Packet header data */
	uint8_t pad[DEFAULT_MBUF_SIZE - sizeof(pkt_hdr_t)];
} pkt_seq_t __rte_cache_aligned;

struct port_info_s;

void pktgen_send_seq_pkt(struct port_info_s *info, uint32_t seq_idx);

void pktgen_page_seq(uint32_t pid);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_SEQ_H_ */
