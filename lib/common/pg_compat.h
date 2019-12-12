/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/**
 * @file
 *
 * Compat file for pktgen
 */

#ifndef PG_COMPAT_H_
#define PG_COMPAT_H_

#include <rte_version.h>
#include <rte_ethdev.h>
#include <rte_ether.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PG_JUMBO_FRAME_LEN	(9600 + RTE_ETHER_CRC_LEN + RTE_ETHER_HDR_LEN)

#ifndef RTE_JUMBO_ETHER_MTU
#define RTE_JUMBO_ETHER_MTU       \
        (PG_JUMBO_FRAME_LEN - RTE_ETHER_HDR_LEN - \
                RTE_ETHER_CRC_LEN) /**< Ethernet MTU. */
#endif

#ifndef RTE_ETHER_MTU
#define RTE_ETHER_MTU
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(19,8,0,0)
#define pg_ether_addr			rte_ether_addr
#define pg_ether_hdr			rte_ether_hdr
#define pg_ipv4_hdr			rte_ipv4_hdr
#define pg_ipv6_hdr			rte_ipv6_hdr
#define pg_tcp_hdr			rte_tcp_hdr
#define pg_udp_hdr			rte_udp_hdr
#define pg_icmp_hdr			rte_icmp_hdr
#define pg_arp_hdr			rte_arp_hdr
#define pg_vlan_hdr			rte_vlan_hdr
#define arp_hrd				arp_hardware
#define arp_pro				arp_protocol
#define arp_hln				arp_hlen
#define arp_pln				arp_plen
#define arp_op				arp_opcode
#define PG_ETHER_ADDR_LEN		RTE_ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN		RTE_ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN		RTE_ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN		RTE_ETHER_MAX_LEN
#define PG_ETHER_MTU			RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU      RTE_JUMBO_ETHER_MTU
#if RTE_VERSION == RTE_VERSION_NUM(19,5,0,0)
#define PG_ETHER_TYPE_IPv4		RTE_ETHER_TYPE_IPv4
#define PG_ETHER_TYPE_IPv6		RTE_ETHER_TYPE_IPv6
#else
#define PG_ETHER_TYPE_IPv4		RTE_ETHER_TYPE_IPV4
#define PG_ETHER_TYPE_IPv6		RTE_ETHER_TYPE_IPV6
#endif
#define PG_ETHER_TYPE_VLAN		RTE_ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP		RTE_ETHER_TYPE_ARP
#define pg_ether_format_addr		rte_ether_format_addr
#define pg_ether_addr_copy		rte_ether_addr_copy
#else
#define pg_ether_addr			ether_addr
#define pg_ether_hdr			ether_hdr
#define pg_ipv4_hdr			ipv4_hdr
#define pg_ipv6_hdr			ipv6_hdr
#define pg_tcp_hdr			tcp_hdr
#define pg_udp_hdr			udp_hdr
#define pg_icmp_hdr			icmp_hdr
#define pg_arp_hdr			arp_hdr
#define pg_vlan_hdr			vlan_hdr
#define PG_ETHER_ADDR_LEN		ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN		ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN		ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN		ETHER_MAX_LEN
#define PG_ETHER_MTU			RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU      RTE_JUMBO_ETHER_MTU
#define PG_ETHER_TYPE_IPv4		ETHER_TYPE_IPv4
#define PG_ETHER_TYPE_IPv6		ETHER_TYPE_IPv6
#define PG_ETHER_TYPE_VLAN		ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP		ETHER_TYPE_ARP
#define pg_ether_format_addr		ether_format_addr
#define pg_ether_addr_copy		ether_addr_copy
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
#define pg_eth_dev_count_avail	rte_eth_dev_count_avail
#define pg_eth_dev_count_total	rte_eth_dev_count_total
#else
#define pg_eth_dev_count_avail rte_eth_dev_count
#define pg_eth_dev_count_total rte_eth_dev_count
#endif

#if RTE_VERSION < RTE_VERSION_NUM(18, 8, 0, 0)
#define RTE_PRIORITY_LOG 101
#define RTE_PRIORITY_BUS 110
#define RTE_PRIORITY_CLASS 120
#define RTE_PRIORITY_LAST 65535

#define RTE_PRIO(prio) \
        RTE_PRIORITY_ ## prio

/**
 * Run after main() with low priority.
 *
 * @param func
 *   Destructor function name.
 * @param prio
 *   Priority number must be above 100.
 *   Lowest number is the last to run.
 */
#ifndef RTE_FINI_PRIO /* Allow to override from EAL */
#define RTE_FINI_PRIO(func, prio) \
static void __attribute__((destructor(RTE_PRIO(prio)), used)) func(void)
#endif

/**
 * Run after main() with high priority.
 *
 * The destructor will be run *before* prioritized destructors.
 *
 * @param func
 *   Destructor function name.
 */
#define RTE_FINI(func) \
        RTE_FINI_PRIO(func, LAST)
#endif

#ifdef PG_cplusplus
}
#endif

#endif /* PG_COMPAT_H_ */
