/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_RANGE_H_
#define _PKTGEN_RANGE_H_

/**
 * @file
 *
 * Range-mode packet field cycling for Pktgen.
 *
 * Defines the range_info_t structure that stores per-port start, min, max,
 * and increment values for every field that can sweep across a configurable
 * range during SEND_RANGE_PKTS traffic generation.
 */

#include <stdint.h>

#include "pktgen-seq.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct range_info_s {
    /** Source IP increment (per-burst step; IPv4 or IPv6 depending on mode) */
    union {
        uint32_t src_ip_inc;
        uint8_t src_ipv6_inc[PG_IN6ADDRSZ];
    };
    /** Destination IP increment (per-burst step; IPv4 or IPv6 depending on mode) */
    union {
        uint32_t dst_ip_inc;
        uint8_t dst_ipv6_inc[PG_IN6ADDRSZ];
    };
    uint16_t src_port_inc; /**< Source port increment */
    uint16_t dst_port_inc; /**< Destination port increment */
    uint16_t vlan_id_inc;  /**< VLAN id increment */
    uint16_t tcp_seq_inc;  /**< TCP sequence number increment */
    uint16_t tcp_ack_inc;  /**< TCP acknowledgement number increment */
    /** ToS / traffic-class increment (per-burst step) */
    union {
        uint16_t tos_inc;
        uint16_t traffic_class_inc;
    };
    uint16_t cos_inc;      /**< CoS / priority value increment */
    uint16_t pkt_size_inc; /**< Packet size increment */
    uint64_t src_mac_inc;  /**< Source MAC increment */
    uint64_t dst_mac_inc;  /**< Destination MAC increment */
    /** TTL / hop-limits increment (per-burst step) */
    union {
        uint8_t ttl_inc;
        uint8_t hop_limits_inc;
    };

    uint8_t pad0[3];

    /** Source starting IP address (IPv4 or IPv6 depending on mode) */
    union {
        uint32_t src_ip;
        uint8_t src_ipv6[PG_IN6ADDRSZ];
    };
    /** Source IP address minimum */
    union {
        uint32_t src_ip_min;
        uint8_t src_ipv6_min[PG_IN6ADDRSZ];
    };
    /** Source IP address maximum */
    union {
        uint32_t src_ip_max;
        uint8_t src_ipv6_max[PG_IN6ADDRSZ];
    };
    /** Destination starting IP address (IPv4 or IPv6 depending on mode) */
    union {
        uint32_t dst_ip;
        uint8_t dst_ipv6[PG_IN6ADDRSZ];
    };
    /** Destination IP address minimum */
    union {
        uint32_t dst_ip_min;
        uint8_t dst_ipv6_min[PG_IN6ADDRSZ];
    };
    /** Destination IP address maximum */
    union {
        uint32_t dst_ip_max;
        uint8_t dst_ipv6_max[PG_IN6ADDRSZ];
    };

    uint16_t ip_proto; /**< IP Protocol type TCP or UDP */

    uint16_t src_port;     /**< Source port starting value */
    uint16_t src_port_min; /**< Source port minimum */
    uint16_t src_port_max; /**< Source port maximum */

    uint16_t dst_port;     /**< Destination port starting value */
    uint16_t dst_port_min; /**< Destination port minimum */
    uint16_t dst_port_max; /**< Destination port maximum */

    uint8_t tcp_flags; /**< TCP flags value */

    uint32_t tcp_seq;     /**< TCP sequence number starting value */
    uint32_t tcp_seq_min; /**< TCP sequence number minimum */
    uint32_t tcp_seq_max; /**< TCP sequence number maximum */

    uint32_t tcp_ack;     /**< TCP acknowledgement number starting value */
    uint32_t tcp_ack_min; /**< TCP acknowledgement number minimum */
    uint32_t tcp_ack_max; /**< TCP acknowledgement number maximum */

    uint16_t vlan_id;     /**< VLAN id starting value */
    uint16_t vlan_id_min; /**< VLAN id minimum */
    uint16_t vlan_id_max; /**< VLAN id maximum */

    uint8_t cos;     /**< CoS / priority value starting */
    uint8_t cos_min; /**< CoS / priority value minimum */
    uint8_t cos_max; /**< CoS / priority value maximum */

    /** ToS / traffic-class starting value */
    union {
        uint8_t tos;
        uint8_t traffic_class;
    };
    /** ToS / traffic-class minimum */
    union {
        uint8_t tos_min;
        uint8_t traffic_class_min;
    };
    /** ToS / traffic-class maximum */
    union {
        uint8_t tos_max;
        uint8_t traffic_class_max;
    };

    uint16_t pkt_size;     /**< Packet size starting value (bytes) */
    uint16_t pkt_size_min; /**< Packet size minimum (bytes) */
    uint16_t pkt_size_max; /**< Packet size maximum (bytes) */

    uint64_t dst_mac;     /**< Destination starting MAC address */
    uint64_t dst_mac_min; /**< Destination minimum MAC address */
    uint64_t dst_mac_max; /**< Destination maximum MAC address */

    uint64_t src_mac;     /**< Source starting MAC address */
    uint64_t src_mac_min; /**< Source minimum MAC address */
    uint64_t src_mac_max; /**< Source maximum MAC address */

    /** TTL / hop-limits starting value */
    union {
        uint8_t ttl;
        uint8_t hop_limits;
    };
    /** TTL / hop-limits minimum */
    union {
        uint8_t ttl_min;
        uint8_t hop_limits_min;
    };
    /** TTL / hop-limits maximum */
    union {
        uint8_t ttl_max;
        uint8_t hop_limits_max;
    };

    uint32_t gtpu_teid;     /**< GTP-U TEID starting value */
    uint32_t gtpu_teid_inc; /**< GTP-U TEID increment */
    uint32_t gtpu_teid_min; /**< GTP-U TEID minimum */
    uint32_t gtpu_teid_max; /**< GTP-U TEID maximum */

    uint32_t vxlan_gid;     /**< VxLAN Group ID starting value */
    uint32_t vxlan_gid_inc; /**< VxLAN Group ID increment */
    uint32_t vxlan_gid_min; /**< VxLAN Group ID minimum */
    uint32_t vxlan_gid_max; /**< VxLAN Group ID maximum */

    uint32_t vxlan_vid;     /**< VxLAN VLAN ID starting value */
    uint32_t vxlan_vid_inc; /**< VxLAN VLAN ID increment */
    uint32_t vxlan_vid_min; /**< VxLAN VLAN ID minimum */
    uint32_t vxlan_vid_max; /**< VxLAN VLAN ID maximum */

    uint32_t vni_flags; /**< VxLAN VNI flags */
} range_info_t;

struct port_info_s;

/**
 * Populate a packet sequence template from a range_info_t configuration.
 *
 * @param range
 *   Pointer to the range configuration to apply.
 * @param pkt
 *   Packet sequence entry to update with current range field values.
 */
void pktgen_range_ctor(range_info_t *range, pkt_seq_t *pkt);

/**
 * Initialise range-mode state for a port.
 *
 * Copies the default single-packet template into the range packet slot and
 * sets up the initial range field values from the port's range_info_t.
 *
 * @param info
 *   Per-port state structure to initialise.
 */
void pktgen_range_setup(struct port_info_s *info);

/**
 * Render the range-mode display page to the terminal.
 */
void pktgen_page_range(void);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_RANGE_H_ */
