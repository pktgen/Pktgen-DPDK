/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CMDS_H_
#define _PKTGEN_CMDS_H_

/**
 * @file
 *
 * Pktgen command API: all functions invoked by the CLI and Lua scripting layer.
 *
 * Covers internal display helpers, global port control, single/range/sequence
 * packet configuration, debug utilities, enable/disable toggles, PCAP handling,
 * and TCP flag parsing helpers.
 */

#include <inttypes.h>
#include <rte_version.h>

#include <rte_net.h>

#include "pktgen.h"
#include <rte_string_fns.h>
#include <portlist.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Internal APIs */

/** Format the active port flags as a human-readable string. */
char *pktgen_flags_string(port_info_t *pinfo);

/** Format the TX count and rate for a port into @p buff. */
char *pktgen_transmit_count_rate(int port, char *buff, int len);

/** Schedule a display refresh on the next timer tick. */
void pktgen_update_display(void);

/** Refresh port statistics and redraw the active display page. */
void pktgen_update(void);

/** Format the link state for a port into @p buff. */
char *pktgen_link_state(int port, char *buff, int len);

/** Format the TX packet count for a port into @p buff. */
char *pktgen_transmit_count(int port, char *buff, int len);

/** Format the TX rate percentage for a port into @p buff. */
char *pktgen_transmit_rate(int port, char *buff, int len);

/**
 * Copy the current statistics for a port into a caller-supplied struct.
 *
 * @return
 *   0 on success, negative on invalid port.
 */
int pktgen_port_stats(int port, port_stats_t *pstats);

/* Global commands */

/**
 * Send an ARP request of the given type on a port.
 *
 * @param pinfo
 *   Per-port state.
 * @param type
 *   ARP type flag (SEND_ARP_REQUEST or SEND_GRATUITOUS_ARP).
 */
void pktgen_send_arp_requests(port_info_t *pinfo, uint32_t type);

/** Start packet transmission on a port. */
void pktgen_start_transmitting(port_info_t *pinfo);

/** Stop packet transmission on a port. */
void pktgen_stop_transmitting(port_info_t *pinfo);

/** Return non-zero if a port is currently transmitting. */
int pktgen_port_transmitting(int port);

/** Switch the active display page (e.g. "main", "range", "seq"). */
void pktgen_set_page(char *str);

/** Enable or disable the terminal screen. */
void pktgen_screen(int state);

/** Force an immediate display update regardless of the timer tick. */
void pktgen_force_update(void);

void pktgen_update_display(void);

/** Clear and redraw the display screen. */
void pktgen_clear_display(void);

/** Start or stop the latency sampler on a port based on @p state. */
void pktgen_start_stop_latency_sampler(port_info_t *pinfo, uint32_t state);

/** Start the latency sampler on a port. */
void pktgen_start_latency_sampler(port_info_t *pinfo);

/** Stop the latency sampler on a port. */
void pktgen_stop_latency_sampler(port_info_t *pinfo);

/**
 * Save the current pktgen configuration to a file.
 *
 * @param path
 *   Destination file path.
 * @return
 *   0 on success, negative on error.
 */
int pktgen_save(char *path);

/** Clear the terminal screen. */
void pktgen_cls(void);

/** Send an IPv4 ping (ICMP Echo Request) on a port. */
void pktgen_ping4(port_info_t *pinfo);

#ifdef INCLUDE_PING6
/** Send an IPv6 ping (ICMPv6 Echo Request) on a port. */
void pktgen_ping6(port_info_t *pinfo);
#endif

/** Clear accumulated statistics for a port. */
void pktgen_clear_stats(port_info_t *pinfo);

/** Reset a port's configuration to factory defaults. */
void pktgen_reset(port_info_t *pinfo);

/** Stop and restart the Ethernet device for a port. */
void pktgen_port_restart(port_info_t *pinfo);

/** Enable or disable learning the source MAC address from received ARP replies. */
void pktgen_mac_from_arp(int state);

/** Send a burst of priming packets to pre-fill downstream buffers. */
void pktgen_prime_ports(port_info_t *pinfo);

/** Terminate pktgen cleanly. */
void pktgen_quit(void);

/** Set the number of ports displayed per screen page. */
void pktgen_set_page_size(uint32_t page_size);

/** Set the currently displayed port number. */
void pktgen_set_port_number(uint16_t port_number);

/** Set the prime-burst packet count for a port. */
void pktgen_set_port_prime(port_info_t *pinfo, uint32_t cnt);

/** Reset per-port settings for a port to their defaults. */
void pktgen_port_defaults(uint16_t pid);

/** Reset all sequence-packet slots for a port to their defaults. */
void pktgen_seq_defaults(uint16_t pid);

struct pg_ipaddr;

/* Single */

/**
 * Set the source or destination IP address for single-packet mode.
 *
 * @param pinfo   Per-port state.
 * @param type    's' for source, 'd' for destination.
 * @param ip      Parsed IP address.
 * @param ip_ver  4 for IPv4, 6 for IPv6.
 */
void single_set_ipaddr(port_info_t *pinfo, char type, struct pg_ipaddr *ip, int ip_ver);

/** Set the transport protocol ("tcp", "udp", or "icmp"). */
void single_set_proto(port_info_t *pinfo, char *type);

/** Set the TCP initial sequence number. */
void single_set_tcp_seq(port_info_t *pinfo, uint32_t seq);

/** Set the TCP initial acknowledgement number. */
void single_set_tcp_ack(port_info_t *pinfo, uint32_t ack);

/** Set TCP flags from a string (e.g. "syn", "ack", "fin"). */
void single_set_tcp_flags(port_info_t *pinfo, const char *flags);

/** Set the VLAN ID for single-packet mode. */
void single_set_vlan_id(port_info_t *pinfo, uint16_t vlanid);

/** Set the CoS / 802.1p priority value. */
void single_set_cos(port_info_t *pinfo, uint8_t cos);

/** Set the ToS (DSCP) byte value. */
void single_set_tos(port_info_t *pinfo, uint8_t tos);

/**
 * Set a MAC address for single-packet mode.
 *
 * @param which  "src" or "dst".
 */
void single_set_mac(port_info_t *pinfo, const char *which, struct rte_ether_addr *mac);

/** Set the destination MAC address. */
void single_set_dst_mac(port_info_t *pinfo, struct rte_ether_addr *mac);

/** Set the source MAC address. */
void single_set_src_mac(port_info_t *pinfo, struct rte_ether_addr *mac);

/** Set the packet type ("ipv4", "ipv6", "vlan", etc.). */
void single_set_pkt_type(port_info_t *pinfo, const char *type);

/** Set the TX packet count (0 means send forever). */
void single_set_tx_count(port_info_t *pinfo, uint32_t cnt);

/** Set the TX burst size. */
void single_set_tx_burst(port_info_t *pinfo, uint32_t burst);

/** Set the RX burst size. */
void single_set_rx_burst(port_info_t *pinfo, uint32_t burst);

/** Set the packet size in bytes (excluding FCS). */
void single_set_pkt_size(port_info_t *pinfo, uint16_t size);

/** Set the TX rate as a percentage string (e.g. "100" or "50.5"). */
void single_set_tx_rate(port_info_t *pinfo, const char *rate);

/** Set the jitter threshold in microseconds. */
void single_set_jitter(port_info_t *pinfo, uint64_t threshold);

/** Set the IP TTL / hop-limit value. */
void single_set_ttl_value(port_info_t *pinfo, uint8_t ttl);

/**
 * Set the source or destination L4 port number.
 *
 * @param type       's' for source, 'd' for destination.
 * @param portValue  Port number to set.
 */
void single_set_port_value(port_info_t *pinfo, char type, uint32_t portValue);

/** Set the outer and inner VLAN IDs for Q-in-Q mode. */
void single_set_qinqids(port_info_t *pinfo, uint16_t outerid, uint16_t innerid);

/** Set VxLAN tunnel flags, group ID, and VNI. */
void single_set_vxlan(port_info_t *pinfo, uint16_t flags, uint16_t group_id, uint32_t vxlan_id);

/**
 * Configure the latency sampler for a port.
 *
 * @param type           Sampler type string ("simple" or "poisson").
 * @param num_samples    Number of samples to collect.
 * @param sampling_rate  Sampling rate in packets per second.
 * @param outfile        Path to write the sample data, or empty for none.
 */
void single_set_latsampler_params(port_info_t *pinfo, char *type, uint32_t num_samples,
                                  uint32_t sampling_rate, char outfile[]);

/* Debug */

/** Dump port debug information to the log. */
void debug_dump(port_info_t *pinfo, char *str);

/** Enable or disable port LED blinking. */
void debug_blink(port_info_t *pinfo, uint32_t state);

/** Override the TX inter-burst cycle count for debugging. */
void debug_set_tx_cycles(port_info_t *pinfo, uint32_t cycles);

/** Override the RX poll cycle count for debugging. */
void debug_set_rx_cycles(port_info_t *pinfo, uint32_t cycles);

/** Dump the full L2P lcore-to-port matrix to the log. */
void debug_matrix_dump(void);

/** Dump mempool statistics for a named pool on a port. */
void debug_mempool_dump(port_info_t *pinfo, char *name);

/** Enable raw packet dump for the next @p cnt received packets. */
void debug_set_port_dump(port_info_t *pinfo, uint32_t cnt);

/** Print TX rate debug counters for a port. */
void debug_tx_rate(port_info_t *pinfo);

/** Enable or disable PCAP file replay mode on a port. */
void pktgen_pcap_handler(port_info_t *pinfo, uint32_t state);

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
/** Display the bonding driver mode for a port. */
void show_bonding_mode(port_info_t *pinfo);
#endif

/* Enable or toggle types */

/** Enable or disable the RX TAP interface. */
void enable_rx_tap(port_info_t *pinfo, uint32_t state);

/** Enable or disable the TX TAP interface. */
void enable_tx_tap(port_info_t *pinfo, uint32_t state);

/** Enable or disable VLAN tagging. */
void enable_vlan(port_info_t *pinfo, uint32_t state);

/** Enable or disable VxLAN encapsulation. */
void enable_vxlan(port_info_t *pinfo, uint32_t state);

/** Enable or disable Q-in-Q double VLAN tagging. */
void enable_qinq(port_info_t *pinfo, uint32_t state);

/** Enable or disable MPLS label insertion. */
void enable_mpls(port_info_t *pinfo, uint32_t state);

/** Enable or disable GRE IPv4 encapsulation. */
void enable_gre(port_info_t *pinfo, uint32_t state);

/** Enable or disable GRE Ethernet frame encapsulation. */
void enable_gre_eth(port_info_t *pinfo, uint32_t state);

/** Enable or disable ICMP Echo reply processing. */
void enable_icmp_echo(port_info_t *pinfo, uint32_t state);

/** Enable or disable random source IP address generation. */
void enable_rnd_s_ip(port_info_t *pinfo, uint32_t state);

/** Enable or disable random source port generation. */
void enable_rnd_s_pt(port_info_t *pinfo, uint32_t state);

/** Enable or disable random bitfield packet mode. */
void enable_random(port_info_t *pinfo, uint32_t state);

/** Enable or disable latency measurement packet injection. */
void enable_latency(port_info_t *pinfo, uint32_t state);

/** Enable or disable global MAC-from-ARP learning. */
void enable_mac_from_arp(uint32_t state);

/** Enable or disable clock_gettime() as the time source (vs rdtsc). */
void enable_clock_gettime(uint32_t state);

/** Enable or disable input packet processing (ARP/ICMP handling). */
void enable_process(port_info_t *pinfo, int state);

/** Enable or disable packet capture to memory. */
void enable_capture(port_info_t *pinfo, uint32_t state);

/** Enable or disable range-mode packet generation. */
void enable_range(port_info_t *pinfo, uint32_t state);

/** Enable or disable PCAP file replay mode. */
void enable_pcap(port_info_t *pinfo, uint32_t state);

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
/** Enable or disable the bonding driver TX-zero-packets workaround. */
void enable_bonding(port_info_t *pinfo, uint32_t state);
#endif

/* PCAP */

/** Set a BPF filter string for PCAP packet capture. */
void pcap_filter(port_info_t *pinfo, char *str);

/* Range commands */

/**
 * Set start, minimum, or maximum destination MAC in range mode.
 *
 * @param what  "start", "min", "max", or "inc".
 */
void range_set_dest_mac(port_info_t *pinfo, const char *what, struct rte_ether_addr *mac);

/** Set start, minimum, maximum, or increment for the source MAC in range mode. */
void range_set_src_mac(port_info_t *pinfo, const char *what, struct rte_ether_addr *mac);

/** Set start, minimum, maximum, or increment for the source IP in range mode. */
void range_set_src_ip(port_info_t *pinfo, char *what, struct pg_ipaddr *ip);

/** Set start, minimum, maximum, or increment for the destination IP in range mode. */
void range_set_dst_ip(port_info_t *pinfo, char *what, struct pg_ipaddr *ip);

/** Set start, minimum, maximum, or increment for the source L4 port in range mode. */
void range_set_src_port(port_info_t *pinfo, char *what, uint16_t port);

/** Set start, minimum, maximum, or increment for the destination L4 port in range mode. */
void range_set_dst_port(port_info_t *pinfo, char *what, uint16_t port);

/** Set the transport protocol for range mode ("tcp" or "udp"). */
void range_set_proto(port_info_t *pinfo, const char *type);

/** Set the packet type for range mode. */
void range_set_pkt_type(port_info_t *pinfo, const char *type);

/** Set TCP flags for range mode from a string. */
void range_set_tcp_flags(port_info_t *pinfo, const char *flags);

/** Set start, minimum, maximum, or increment for the TCP sequence number in range mode. */
void range_set_tcp_seq(port_info_t *pinfo, char *what, uint32_t seq);

/** Set start, minimum, maximum, or increment for the TCP acknowledgement number in range mode. */
void range_set_tcp_ack(port_info_t *pinfo, char *what, uint32_t ack);

/** Set start, minimum, maximum, or increment for the packet size in range mode. */
void range_set_pkt_size(port_info_t *pinfo, char *what, uint16_t size);

/** Set start, minimum, maximum, or increment for the GTP-U TEID in range mode. */
void range_set_gtpu_teid(port_info_t *pinfo, char *what, uint32_t teid);

/** Set start, minimum, maximum, or increment for the VLAN ID in range mode. */
void range_set_vlan_id(port_info_t *pinfo, char *what, uint16_t id);

/** Set start, minimum, maximum, or increment for the ToS byte in range mode. */
void range_set_tos_id(port_info_t *pinfo, char *what, uint8_t id);

/** Set start, minimum, maximum, or increment for the CoS value in range mode. */
void range_set_cos_id(port_info_t *pinfo, char *what, uint8_t id);

/** Set the MPLS label entry for range mode. */
void range_set_mpls_entry(port_info_t *pinfo, uint32_t mpls_entry);

/** Set the Q-in-Q outer and inner VLAN IDs for range mode. */
void range_set_qinqids(port_info_t *pinfo, uint16_t outerid, uint16_t innerid);

/** Set the GRE key for range mode. */
void range_set_gre_key(port_info_t *pinfo, uint32_t gre_key);

/** Set start, minimum, maximum, or increment for the TTL in range mode. */
void range_set_ttl(port_info_t *pinfo, char *what, uint8_t ttl);

/** Set start, minimum, maximum, or increment for the IPv6 hop limit in range mode. */
void range_set_hop_limits(port_info_t *pinfo, char *what, uint8_t hop_limits);

/** Set start, minimum, maximum, or increment for the IPv6 traffic class in range mode. */
void range_set_traffic_class(port_info_t *pinfo, char *what, uint8_t traffic_class);

/* Sequence */

/** Set the number of active sequence packets for a port. */
void pktgen_set_port_seqCnt(port_info_t *pinfo, uint32_t cnt);

/**
 * Set all fields for one sequence-packet slot.
 *
 * @param seqnum     Slot index (0 .. NUM_SEQ_PKTS-1).
 * @param daddr      Destination MAC address.
 * @param saddr      Source MAC address.
 * @param ip_daddr   Destination IP address.
 * @param ip_saddr   Source IP address.
 * @param sport      Source L4 port.
 * @param dport      Destination L4 port.
 * @param ip         'v' for IPv4, '6' for IPv6.
 * @param proto      't' for TCP, 'u' for UDP, 'i' for ICMP.
 * @param vlanid     VLAN ID.
 * @param pktsize    Packet size in bytes.
 * @param gtpu_teid  GTP-U TEID (0 if unused).
 */
void pktgen_set_seq(port_info_t *pinfo, uint32_t seqnum, struct rte_ether_addr *daddr,
                    struct rte_ether_addr *saddr, struct pg_ipaddr *ip_daddr,
                    struct pg_ipaddr *ip_saddr, uint32_t sport, uint32_t dport, char ip, char proto,
                    uint16_t vlanid, uint32_t pktsize, uint32_t gtpu_teid);

/** Set CoS and ToS values for a sequence-packet slot. */
void pktgen_set_cos_tos_seq(port_info_t *pinfo, uint32_t seqnum, uint32_t cos, uint32_t tos);

/** Set VxLAN parameters for a sequence-packet slot. */
void pktgen_set_vxlan_seq(port_info_t *pinfo, uint32_t seqnum, uint32_t flag, uint32_t gid,
                          uint32_t vid);

/** Set TCP flags for a sequence-packet slot from a string. */
void seq_set_tcp_flags(port_info_t *pinfo, uint32_t seqnum, const char *flags);

/* Pattern */

/** Set the payload fill pattern type ("zero", "abc", "user", or "none"). */
void pattern_set_type(port_info_t *pinfo, char *str);

/** Set the user-defined payload fill pattern string. */
void pattern_set_user_pattern(port_info_t *pinfo, char *str);

/**
 * Parse a TCP flags string into a bitmask.
 *
 * @return
 *   Bitmask of TCP flag bits.
 */
uint16_t tcp_flags_from_str(const char *str);

/**
 * Format a TCP flag bitmask as a human-readable string.
 *
 * @param flags  Bitmask of TCP flag bits.
 * @param buf    Output buffer.
 * @param len    Size of @p buf.
 * @return
 *   0 on success, negative if the buffer is too small.
 */
int tcp_str_from_flags(uint16_t flags, char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CMDS_H_ */
