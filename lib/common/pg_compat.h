/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2020 Intel Corporation.
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

#define PG_JUMBO_FRAME_LEN (9600 + RTE_ETHER_CRC_LEN + RTE_ETHER_HDR_LEN)

#ifndef RTE_JUMBO_ETHER_MTU
#define RTE_JUMBO_ETHER_MTU \
    (PG_JUMBO_FRAME_LEN - RTE_ETHER_HDR_LEN - RTE_ETHER_CRC_LEN) /**< Ethernet MTU. */
#endif

#ifndef RTE_ETHER_MTU
#define RTE_ETHER_MTU
#endif

#define __RTE_VERSION RTE_VERSION_NUM(RTE_VER_YEAR, RTE_VER_MONTH, RTE_VER_MINOR, RTE_VER_RELEASE)

#define pg_eth_dev_count_avail rte_eth_dev_count_avail
#define pg_eth_dev_count_total rte_eth_dev_count_total

#if __RTE_VERSION >= RTE_VERSION_NUM(21, 5, 0, 0)
#define pg_eth_bond_workers_get        rte_eth_bond_slaves_get
#define pg_eth_bond_active_workers_get rte_eth_bond_active_slaves_get
#define pg_eth_bond_8023ad_worker_info rte_eth_bond_8023ad_slave_info
#define PG_SKIP_MAIN                   SKIP_MAIN
#define pg_get_initial_lcore           rte_get_main_lcore
#define PG_DEVTYPE_BLOCKED             RTE_DEVTYPE_BLOCKED
#define PG_DEVTYPE_ALLOWED             RTE_DEVTYPE_ALLOWED
#define pg_ether_addr                  rte_ether_addr
#define pg_ether_hdr                   rte_ether_hdr
#define pg_ipv4_hdr                    rte_ipv4_hdr
#define pg_ipv6_hdr                    rte_ipv6_hdr
#define pg_tcp_hdr                     rte_tcp_hdr
#define pg_udp_hdr                     rte_udp_hdr
#define pg_icmp_hdr                    rte_icmp_hdr
#define pg_arp_hdr                     rte_arp_hdr
#define pg_vlan_hdr                    rte_vlan_hdr
#define arp_hrd                        arp_hardware
#define arp_pro                        arp_protocol
#define arp_hln                        arp_hlen
#define arp_pln                        arp_plen
#define arp_op                         arp_opcode
#define PG_ETHER_ADDR_LEN              RTE_ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN               RTE_ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN               RTE_ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN               RTE_ETHER_MAX_LEN
#define PG_ETHER_MTU                   RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN   PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU             RTE_JUMBO_ETHER_MTU
#define PG_ETHER_TYPE_IPv4             RTE_ETHER_TYPE_IPV4
#define PG_ETHER_TYPE_IPv6             RTE_ETHER_TYPE_IPV6
#define PG_ETHER_TYPE_VLAN             RTE_ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP              RTE_ETHER_TYPE_ARP
#define pg_ether_format_addr           rte_ether_format_addr
#define pg_ether_addr_copy             rte_ether_addr_copy

#elif __RTE_VERSION >= RTE_VERSION_NUM(20, 11, 0, 0)
#define pg_eth_bond_workers_get        rte_eth_bond_slaves_get
#define pg_eth_bond_active_workers_get rte_eth_bond_active_slaves_get
#define pg_eth_bond_8023ad_worker_info rte_eth_bond_8023ad_slave_info
#define PG_SKIP_MAIN                   SKIP_MAIN
#define pg_get_initial_lcore           rte_get_main_lcore
#define PG_DEVTYPE_BLOCKED             RTE_DEVTYPE_BLOCKED
#define PG_DEVTYPE_ALLOWED             RTE_DEVTYPE_ALLOWED
#define pg_ether_addr                  rte_ether_addr
#define pg_ether_hdr                   rte_ether_hdr
#define pg_ipv4_hdr                    rte_ipv4_hdr
#define pg_ipv6_hdr                    rte_ipv6_hdr
#define pg_tcp_hdr                     rte_tcp_hdr
#define pg_udp_hdr                     rte_udp_hdr
#define pg_icmp_hdr                    rte_icmp_hdr
#define pg_arp_hdr                     rte_arp_hdr
#define pg_vlan_hdr                    rte_vlan_hdr
#define arp_hrd                        arp_hardware
#define arp_pro                        arp_protocol
#define arp_hln                        arp_hlen
#define arp_pln                        arp_plen
#define arp_op                         arp_opcode
#define PG_ETHER_ADDR_LEN              RTE_ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN               RTE_ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN               RTE_ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN               RTE_ETHER_MAX_LEN
#define PG_ETHER_MTU                   RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN   PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU             RTE_JUMBO_ETHER_MTU
#define PG_ETHER_TYPE_VLAN             RTE_ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP              RTE_ETHER_TYPE_ARP
#define PG_ETHER_TYPE_IPv4             RTE_ETHER_TYPE_IPV4
#define PG_ETHER_TYPE_IPv6             RTE_ETHER_TYPE_IPV6
#define pg_ether_format_addr           rte_ether_format_addr
#define pg_ether_addr_copy             rte_ether_addr_copy

#elif __RTE_VERSION >= RTE_VERSION_NUM(20, 8, 0, 0)
#define PG_SKIP_MAIN                 SKIP_MASTER
#define pg_get_initial_lcore         rte_get_master_lcore
#define PG_DEVTYPE_BLOCKED           RTE_DEVTYPE_BLACKLISTED_PCI
#define PG_DEVTYPE_ALLOWED           RTE_DEVTYPE_WHITELISTED_PCI
#define pg_ether_addr                rte_ether_addr
#define pg_ether_hdr                 rte_ether_hdr
#define pg_ipv4_hdr                  rte_ipv4_hdr
#define pg_ipv6_hdr                  rte_ipv6_hdr
#define pg_tcp_hdr                   rte_tcp_hdr
#define pg_udp_hdr                   rte_udp_hdr
#define pg_icmp_hdr                  rte_icmp_hdr
#define pg_arp_hdr                   rte_arp_hdr
#define pg_vlan_hdr                  rte_vlan_hdr
#define arp_hrd                      arp_hardware
#define arp_pro                      arp_protocol
#define arp_hln                      arp_hlen
#define arp_pln                      arp_plen
#define arp_op                       arp_opcode
#define PG_ETHER_ADDR_LEN            RTE_ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN             RTE_ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN             RTE_ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN             RTE_ETHER_MAX_LEN
#define PG_ETHER_MTU                 RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU           RTE_JUMBO_ETHER_MTU
#define PG_ETHER_TYPE_VLAN           RTE_ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP            RTE_ETHER_TYPE_ARP
#define PG_ETHER_TYPE_IPv4           RTE_ETHER_TYPE_IPv4
#define PG_ETHER_TYPE_IPv6           RTE_ETHER_TYPE_IPv6
#define pg_ether_format_addr         rte_ether_format_addr
#define pg_ether_addr_copy           rte_ether_addr_copy

#elif __RTE_VERSION >= RTE_VERSION_NUM(20, 5, 0, 0)
#define PG_SKIP_MAIN                 SKIP_MASTER
#define pg_get_initial_lcore         rte_get_master_lcore
#define PG_DEVTYPE_BLOCKED           RTE_DEVTYPE_BLACKLISTED_PCI
#define PG_DEVTYPE_ALLOWED           RTE_DEVTYPE_WHITELISTED_PCI
#define pg_ether_addr                rte_ether_addr
#define pg_ether_hdr                 rte_ether_hdr
#define pg_ipv4_hdr                  rte_ipv4_hdr
#define pg_ipv6_hdr                  rte_ipv6_hdr
#define pg_tcp_hdr                   rte_tcp_hdr
#define pg_udp_hdr                   rte_udp_hdr
#define pg_icmp_hdr                  rte_icmp_hdr
#define pg_arp_hdr                   rte_arp_hdr
#define pg_vlan_hdr                  rte_vlan_hdr
#define arp_hrd                      arp_hardware
#define arp_pro                      arp_protocol
#define arp_hln                      arp_hlen
#define arp_pln                      arp_plen
#define arp_op                       arp_opcode
#define PG_ETHER_ADDR_LEN            RTE_ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN             RTE_ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN             RTE_ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN             RTE_ETHER_MAX_LEN
#define PG_ETHER_MTU                 RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU           RTE_JUMBO_ETHER_MTU
#define PG_ETHER_TYPE_VLAN           RTE_ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP            RTE_ETHER_TYPE_ARP
#define PG_ETHER_TYPE_IPv4           ETHER_TYPE_IPv4
#define PG_ETHER_TYPE_IPv6           ETHER_TYPE_IPv6
#define pg_ether_format_addr         rte_ether_format_addr
#define pg_ether_addr_copy           rte_ether_addr_copy

#else   /* Anything older then 20.05 requires an older version of Ubuntu 20.10 */
#define PG_SKIP_MAIN                   SKIP_MASTER
#define pg_get_initial_lcore           rte_get_master_lcore
#define PG_DEVTYPE_BLOCKED             RTE_DEVTYPE_BLACKLISTED_PCI
#define PG_DEVTYPE_ALLOWED             RTE_DEVTYPE_WHITELISTED_PCI
#define pg_ether_addr                  ether_addr
#define pg_ether_hdr                   ether_hdr
#define pg_ipv4_hdr                    ipv4_hdr
#define pg_ipv6_hdr                    ipv6_hdr
#define pg_tcp_hdr                     tcp_hdr
#define pg_udp_hdr                     udp_hdr
#define pg_icmp_hdr                    icmp_hdr
#define pg_arp_hdr                     arp_hdr
#define pg_vlan_hdr                    vlan_hdr
#define pg_eth_bond_workers_get        rte_eth_bond_slaves_get
#define pg_eth_bond_active_workers_get rte_eth_bond_active_slaves_get
#define pg_eth_bond_8023ad_worker_info rte_eth_bond_8023ad_slave_info
#define PG_ETHER_ADDR_LEN              ETHER_ADDR_LEN
#define PG_ETHER_CRC_LEN               ETHER_CRC_LEN
#define PG_ETHER_MIN_LEN               ETHER_MIN_LEN
#define PG_ETHER_MAX_LEN               ETHER_MAX_LEN
#define PG_ETHER_MTU                   RTE_ETHER_MTU
#define PG_ETHER_MAX_JUMBO_FRAME_LEN   PG_JUMBO_FRAME_LEN
#define PG_JUMBO_ETHER_MTU             RTE_JUMBO_ETHER_MTU
#define PG_ETHER_TYPE_IPv4             ETHER_TYPE_IPv4
#define PG_ETHER_TYPE_IPv6             ETHER_TYPE_IPv6
#define PG_ETHER_TYPE_VLAN             ETHER_TYPE_VLAN
#define PG_ETHER_TYPE_ARP              ETHER_TYPE_ARP
#define pg_ether_format_addr           ether_format_addr
#define pg_ether_addr_copy             ether_addr_copy
#endif

#ifdef PG_cplusplus
}
#endif

#endif /* PG_COMPAT_H_ */
