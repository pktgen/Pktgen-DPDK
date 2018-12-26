/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_RANGE_H_
#define _PKTGEN_RANGE_H_

#include <stdint.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct range_info_s {
	uint32_t src_ip_inc;	/**< Source IP increment */
	uint32_t dst_ip_inc;	/**< Destination increment IP address */
	uint16_t src_port_inc;	/**< Source port increment */
	uint16_t dst_port_inc;	/**< Destination port increment */
	uint16_t vlan_id_inc;	/**< VLAN id increment */
	uint16_t tos_inc;		/**< tos val increment */
	uint16_t cos_inc;		/**< prio val increment */
	uint16_t pkt_size_inc;	/**< PKT size increment */
	uint64_t src_mac_inc;	/**< Source MAC increment */
	uint64_t dst_mac_inc;	/**< Destination MAC increment */

	uint32_t src_ip;	/**< Source starting IP address */
	uint32_t src_ip_min;	/**< Source IP minimum */
	uint32_t src_ip_max;	/**< Source IP maximum */

	uint32_t dst_ip;	/**< Destination starting IP address */
	uint32_t dst_ip_min;	/**< Destination minimum IP address */
	uint32_t dst_ip_max;	/**< Destination maximum IP address */

	uint16_t ip_proto;	/**< IP Protocol type TCP or UDP */

	uint16_t src_port;	/**< Source port starting */
	uint16_t src_port_min;	/**< Source port minimum */
	uint16_t src_port_max;	/**< Source port maximum */

	uint16_t dst_port;	/**< Destination port starting */
	uint16_t dst_port_min;	/**< Destination port minimum */
	uint16_t dst_port_max;	/**< Destination port maximum */

	uint16_t vlan_id;	/**< VLAN id starting */
	uint16_t vlan_id_min;	/**< VLAN id minimum */
	uint16_t vlan_id_max;	/**< VLAN id maximum */

	uint8_t cos;		/**< prio val starting */
	uint8_t cos_min;	/**< prio val minimum */
	uint8_t cos_max;	/**< prio val maximum */

	uint8_t tos;		/**< tos val starting */
	uint8_t tos_min;	/**< tos val minimum */
	uint8_t tos_max;	/**< tos val maximum */

	uint16_t pkt_size;	/**< PKT Size starting */
	uint16_t pkt_size_min;	/**< PKT Size minimum */
	uint16_t pkt_size_max;	/**< PKT Size maximum */

	uint64_t dst_mac;	/**< Destination starting MAC address */
	uint64_t dst_mac_min;	/**< Destination minimum MAC address */
	uint64_t dst_mac_max;	/**< Destination maximum MAC address */

	uint64_t src_mac;	/**< Source starting MAC address */
	uint64_t src_mac_min;	/**< Source minimum MAC address */
	uint64_t src_mac_max;	/**< Source maximum MAC address */

	uint32_t gtpu_teid;	/**< GTP-U TEID starting */
	uint32_t gtpu_teid_inc;	/**< GTP-U TEID inc */
	uint32_t gtpu_teid_min;	/**< GTP-U TEID minimum */
	uint32_t gtpu_teid_max;	/**< GTP-U TEID maximum */

	uint32_t vxlan_gid;	/**< VxLAN Group ID */
	uint32_t vxlan_gid_inc;
	uint32_t vxlan_gid_min;
	uint32_t vxlan_gid_max;

	uint32_t vxlan_vid;	/**< VxLAN VLAN ID */
	uint32_t vxlan_vid_inc;
	uint32_t vxlan_vid_min;
	uint32_t vxlan_vid_max;

	uint32_t vni_flags;	/**< VxLAN Flags */
} range_info_t;

struct port_info_s;

void pktgen_range_ctor(range_info_t *range, pkt_seq_t *pkt);
void pktgen_range_setup(struct port_info_s *info);
void pktgen_page_range(void);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_RANGE_H_ */
