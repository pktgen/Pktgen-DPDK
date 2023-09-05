/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CMDS_H_
#define _PKTGEN_CMDS_H_

#include <inttypes.h>
#include <rte_version.h>

#include <rte_net.h>

#include "pktgen.h"
#include <rte_string_fns.h>
#include <portlist.h>

#ifdef __cplusplus
"C"
{
#endif

    /* Internal APIs */
    char *pktgen_flags_string(port_info_t * info);
    char *pktgen_transmit_count_rate(int port, char *buff, int len);
    void pktgen_update_display(void);
    void pktgen_update(void);
    char *pktgen_link_state(int port, char *buff, int len);
    char *pktgen_transmit_count(int port, char *buff, int len);
    char *pktgen_transmit_rate(int port, char *buff, int len);
    int pktgen_pkt_stats(int port, pkt_stats_t *pstats);
    int pktgen_port_stats(int port, const char *name, eth_stats_t *pstats);
    int pktgen_pkt_sizes(int port, pkt_sizes_t *psizes);

    /* Global commands */
    void pktgen_send_arp_requests(port_info_t * info, uint32_t type);
    void pktgen_start_transmitting(port_info_t * info);
    void pktgen_stop_transmitting(port_info_t * info);
    int pktgen_port_transmitting(int port);
    void pktgen_set_page(char *str);
    void pktgen_screen(int state);
    void pktgen_force_update(void);
    void pktgen_update_display(void);
    void pktgen_clear_display(void);
    void pktgen_start_stop_latency_sampler(port_info_t * info, uint32_t state);
    void pktgen_start_latency_sampler(port_info_t * info);
    void pktgen_stop_latency_sampler(port_info_t * info);

    int pktgen_save(char *path);
    void pktgen_cls(void);
    void pktgen_ping4(port_info_t * info);
#ifdef INCLUDE_PING6
    void pktgen_ping6(port_info_t * info);
#endif
    void pktgen_clear_stats(port_info_t * info);
    void pktgen_reset(port_info_t * info);
    void pktgen_port_restart(port_info_t * info);
    void pktgen_mac_from_arp(int state);
    void pktgen_prime_ports(port_info_t * info);
    void pktgen_quit(void);
    void pktgen_set_page_size(uint32_t page_size);
    void pktgen_set_port_number(uint16_t port_number);
    void pktgen_set_port_prime(port_info_t * info, uint32_t cnt);
    void pktgen_port_defaults(uint32_t pid, uint8_t seq);

    struct pg_ipaddr;

    /* Single */
    void single_set_ipaddr(port_info_t * info, char type, struct pg_ipaddr *ip, int ip_ver);
    void single_set_proto(port_info_t * info, char *type);
    void single_set_tcp_seq(port_info_t * info, uint32_t seq);
    void single_set_tcp_ack(port_info_t * info, uint32_t ack);
    void single_set_tcp_flag_set(port_info_t * info, const char *which);
    void single_set_tcp_flag_clr(port_info_t * info, const char *which);
    void single_set_vlan_id(port_info_t * info, uint16_t vlanid);
    void single_set_cos(port_info_t * info, uint8_t cos);
    void single_set_tos(port_info_t * info, uint8_t tos);
    void single_set_mac(port_info_t * info, const char *which, struct rte_ether_addr *mac);
    void single_set_dst_mac(port_info_t * info, struct rte_ether_addr * mac);
    void single_set_src_mac(port_info_t * info, struct rte_ether_addr * mac);
    void single_set_pkt_type(port_info_t * info, const char *type);
    void single_set_tx_count(port_info_t * info, uint32_t cnt);
    void single_set_tx_burst(port_info_t * info, uint32_t burst);
    void single_set_rx_burst(port_info_t * info, uint32_t burst);
    void single_set_pkt_size(port_info_t * info, uint16_t size);
    void single_set_tx_rate(port_info_t * info, const char *rate);
    void single_set_jitter(port_info_t * info, uint64_t threshold);
    void single_set_ttl_value(port_info_t * info, uint8_t ttl);
    void single_set_port_value(port_info_t * info, char type, uint32_t portValue);
    void single_set_qinqids(port_info_t * info, uint16_t outerid, uint16_t innerid);
    void single_set_vxlan(port_info_t * info, uint16_t flags, uint16_t group_id, uint32_t vxlan_id);
    void single_set_latsampler_params(port_info_t * info, char *type, uint32_t num_samples,
                                      uint32_t sampling_rate, char outfile[]);

    /* Rate */
    char *rate_transmit_count_rate(int port, char *buff, int len);
    void rate_set_tx_count(port_info_t * info, uint32_t cnt);
    void rate_set_ipaddr(port_info_t * info, char type, struct pg_ipaddr *ip, int ip_ver);
    void rate_set_proto(port_info_t * info, char *type);
    void rate_set_dst_mac(port_info_t * info, struct rte_ether_addr * mac);
    void rate_set_src_mac(port_info_t * info, struct rte_ether_addr * mac);
    void rate_set_pkt_type(port_info_t * info, const char *type);
    void rate_set_pkt_size(port_info_t * info, uint16_t size);
    void rate_set_ttl_value(port_info_t * info, uint8_t ttl);
    void rate_set_port_value(port_info_t * info, char type, uint32_t portValue);
    void rate_set_tcp_seq(port_info_t * info, uint32_t seq);
    void rate_set_tcp_ack(port_info_t * info, uint32_t ack);
    void rate_set_tcp_flag_set(port_info_t * info, const char *which);
    void rate_set_tcp_flag_clr(port_info_t * info, const char *which);
    void rate_set_tx_burst(port_info_t * info, uint32_t burst);
    void rate_set_rx_burst(port_info_t * info, uint32_t burst);

    /* Debug */
    void debug_dump(port_info_t * info, char *str);
    void debug_blink(port_info_t * info, uint32_t state);
    void debug_pdump(port_info_t * info);
    void debug_set_tx_cycles(port_info_t * info, uint32_t cycles);
    void debug_set_rx_cycles(port_info_t * info, uint32_t cycles);
    void debug_matrix_dump(void);
    void debug_mempool_dump(port_info_t * info, char *name);
    void debug_set_port_dump(port_info_t * info, uint32_t cnt);
    void debug_tx_rate(port_info_t * info);

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
    void show_bonding_mode(port_info_t * info);
#endif

    /* Enable or toggle types */
    void enable_rx_tap(port_info_t * info, uint32_t state);
    void enable_tx_tap(port_info_t * info, uint32_t state);
    void enable_vlan(port_info_t * info, uint32_t state);
    void enable_vxlan(port_info_t * info, uint32_t state);
    void enable_qinq(port_info_t * info, uint32_t state);
    void enable_mpls(port_info_t * info, uint32_t state);
    void enable_gre(port_info_t * info, uint32_t state);
    void enable_gre_eth(port_info_t * info, uint32_t state);
    void enable_icmp_echo(port_info_t * info, uint32_t state);
    void enable_random(port_info_t * info, uint32_t state);
    void enable_latency(port_info_t * info, uint32_t state);
    void enable_mac_from_arp(uint32_t state);
    void enable_clock_gettime(uint32_t state);
    void enable_process(port_info_t * info, int state);
    void enable_capture(port_info_t * info, uint32_t state);
    void enable_range(port_info_t * info, uint32_t state);
    void enable_pcap(port_info_t * info, uint32_t state);
    void enable_rate(port_info_t * info, uint32_t state);

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
    void enable_bonding(port_info_t * info, uint32_t state);
#endif

    /* PCAP */
    void pcap_filter(port_info_t * info, char *str);

    /* Range commands */
    void range_set_dest_mac(port_info_t * info, const char *what, struct rte_ether_addr *mac);
    void range_set_src_mac(port_info_t * info, const char *what, struct rte_ether_addr *mac);
    void range_set_src_ip(port_info_t * info, char *what, struct pg_ipaddr *ip);
    void range_set_dst_ip(port_info_t * info, char *what, struct pg_ipaddr *ip);
    void range_set_src_port(port_info_t * info, char *what, uint16_t port);
    void range_set_dst_port(port_info_t * info, char *what, uint16_t port);
    void range_set_proto(port_info_t * info, const char *type);
    void range_set_pkt_type(port_info_t * info, const char *type);
    void range_set_tcp_flag_set(port_info_t * info, const char *which);
    void range_set_tcp_flag_clr(port_info_t * info, const char *which);
    void range_set_tcp_seq(port_info_t * info, char *what, uint32_t seq);
    void range_set_tcp_ack(port_info_t * info, char *what, uint32_t ack);
    void range_set_pkt_size(port_info_t * info, char *what, uint16_t size);
    void range_set_gtpu_teid(port_info_t * info, char *what, uint32_t teid);
    void range_set_vlan_id(port_info_t * info, char *what, uint16_t id);
    void range_set_tos_id(port_info_t * info, char *what, uint8_t id);
    void range_set_cos_id(port_info_t * info, char *what, uint8_t id);
    void range_set_mpls_entry(port_info_t * info, uint32_t mpls_entry);
    void range_set_qinqids(port_info_t * info, uint16_t outerid, uint16_t innerid);
    void range_set_gre_key(port_info_t * info, uint32_t gre_key);
    void range_set_ttl(port_info_t * info, char *what, uint8_t ttl);
    void range_set_hop_limits(port_info_t * info, char *what, uint8_t hop_limits);
    void range_set_traffic_class(port_info_t * info, char *what, uint8_t traffic_class);

    /* Sequence */
    void pktgen_set_port_seqCnt(port_info_t * info, uint32_t cnt);
    void pktgen_set_seq(port_info_t * info, uint32_t seqnum, struct rte_ether_addr * daddr,
                        struct rte_ether_addr * saddr, struct pg_ipaddr * ip_daddr,
                        struct pg_ipaddr * ip_saddr, uint32_t sport, uint32_t dport, char ip,
                        char proto, uint16_t vlanid, uint32_t pktsize, uint32_t gtpu_teid);
    void pktgen_set_cos_tos_seq(port_info_t * info, uint32_t seqnum, uint32_t cos, uint32_t tos);
    void pktgen_set_vxlan_seq(port_info_t * info, uint32_t seqnum, uint32_t flag, uint32_t gid,
                              uint32_t vid);

    /* Pattern */
    void pattern_set_type(port_info_t * info, char *str);
    void pattern_set_user_pattern(port_info_t * info, char *str);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CMDS_H_ */
