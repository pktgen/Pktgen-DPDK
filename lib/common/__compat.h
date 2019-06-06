/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/**
 * @file
 *
 * Compat file for pktgen
 */

#ifndef __COMPAT_H_
#define __COMPAT_H_

#include <rte_version.h>

#ifdef __cplusplus
extern "C" {
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(19,5,0,0)
#define __ether_addr		rte_ether_addr
#define __ether_hdr		rte_ether_hdr
#define __ipv4_hdr		rte_ipv4_hdr
#define __ipv6_hdr		rte_ipv6_hdr
#define __tcp_hdr		rte_tcp_hdr
#define __udp_hdr		rte_udp_hdr
#define __icmp_hdr		rte_icmp_hdr
#define __arp_hdr		rte_arp_hdr
#define __vlan_hdr		rte_vlan_hdr
#define arp_hrd		arp_hardware
#define arp_pro		arp_protocol
#define arp_hln		arp_hlen
#define arp_pln		arp_plen
#define arp_op		arp_opcode
#define __ETHER_ADDR_LEN	RTE_ETHER_ADDR_LEN
#define __ETHER_CRC_LEN		RTE_ETHER_CRC_LEN
#define __ETHER_MIN_LEN		RTE_ETHER_MIN_LEN
#define __ETHER_MAX_LEN		RTE_ETHER_MAX_LEN
#define __ETHER_TYPE_IPv4	RTE_ETHER_TYPE_IPv4
#define __ETHER_TYPE_IPv6	RTE_ETHER_TYPE_IPv6
#define __ETHER_TYPE_VLAN	RTE_ETHER_TYPE_VLAN
#define __ETHER_TYPE_ARP	RTE_ETHER_TYPE_ARP
#define __ether_format_addr	rte_ether_format_addr
#define __ether_addr_copy	rte_ether_addr_copy
#else
#define __ether_addr		ether_addr
#define __ether_hdr		ether_hdr
#define __ipv4_hdr		ipv4_hdr
#define __ipv6_hdr		ipv6_hdr
#define __tcp_hdr		tcp_hdr
#define __udp_hdr		udp_hdr
#define __icmp_hdr		icmp_hdr
#define __arp_hdr		arp_hdr
#define __vlan_hdr		vlan_hdr
#define __ETHER_ADDR_LEN	ETHER_ADDR_LEN
#define __ETHER_CRC_LEN		ETHER_CRC_LEN
#define __ETHER_MIN_LEN		ETHER_MIN_LEN
#define __ETHER_MAX_LEN		ETHER_MAX_LEN
#define __ETHER_TYPE_IPv4	ETHER_TYPE_IPv4
#define __ETHER_TYPE_IPv6	ETHER_TYPE_IPv6
#define __ETHER_TYPE_VLAN	ETHER_TYPE_VLAN
#define __ETHER_TYPE_ARP	ETHER_TYPE_ARP
#define __ether_format_addr	ether_format_addr
#define __ether_addr_copy	ether_addr_copy
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
#define __eth_dev_count_avail	rte_eth_dev_count_avail
#define __eth_dev_count_total	rte_eth_dev_count_total
#else
#define __eth_dev_count_avail rte_eth_dev_count
#define __eth_dev_count_total rte_eth_dev_count
#endif

#ifdef __cplusplus
}
#endif

#endif /* __COMPAT_H_ */
