/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
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
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CMDS_H_
#define _PKTGEN_CMDS_H_

#include <inttypes.h>

#include "pktgen.h"

/* onOff values */
enum { DISABLE_STATE = 0, ENABLE_STATE = 1 };

static __inline__ uint32_t
parseState(const char * state) {
	return ( !strcasecmp(state, "on") || !strcasecmp(state, "enable") || !strcasecmp(state, "start") ) ?
			ENABLE_STATE : DISABLE_STATE;
}


extern int pktgen_port_transmitting(int port);
extern char * pktgen_link_state(int port, char * buff, int len);
extern char * pktgen_transmit_count(int port, char * buff, int len);
extern char * pktgen_transmit_rate(int port, char * buff, int len);
extern int pktgen_port_sizes(int port, port_sizes_t * psizes);
extern char * pktgen_transmit_count_rate(int port, char * buff, int len);
extern int pktgen_pkt_stats(int port, pkt_stats_t * pstats);
extern int pktgen_port_stats(int port, const char * name, eth_stats_t * pstats);
extern char * pktgen_flags_string( port_info_t * info );
extern void pktgen_matrix_dump(void);
extern void pktgen_redisplay( int cls_flag );
extern void pktgen_update_display(void);
extern void pktgen_update(void);
extern void pktgen_set_page_size(uint32_t page_size);
extern void pktgen_screen(const char * onOff);
extern void pktgen_set_port_number(uint32_t port_number);
extern void pktgen_set_icmp_echo(port_info_t * info, uint32_t onOff);
extern void pktgen_config_mac_from_arp(uint32_t onOff);
extern void pktgen_mempool_dump(port_info_t * info, char * name);
extern void pktgen_start_transmitting(port_info_t * info);
extern void pktgen_stop_transmitting(port_info_t * info);
extern void pktgen_prime_ports(port_info_t * info);
extern void pktgen_set_proto(port_info_t * info, char type);
extern void pktgen_set_rx_tap(port_info_t * info, uint32_t onOff);
extern void pktgen_set_tx_tap(port_info_t * info, uint32_t onOff);
extern int pktgen_save(char * path);

extern void pktgen_pcap_enable_disable(port_info_t * info, char * str);
extern void pktgen_blink_enable_disable(port_info_t * info, char * str);
extern void pktgen_process_enable_disable(port_info_t * info, char * str);
extern void pktgen_pcap_filter(port_info_t * info, char * str);
extern void pktgen_set_pkt_type(port_info_t * info, const char * type);
extern void pktgen_clear_stats(port_info_t * info);
extern void pktgen_cls(void);
extern void pktgen_port_defaults(uint32_t pid, uint8_t seq);
extern void pktgen_ping4(port_info_t * info);
#ifdef INCLUDE_PING6
extern void pktgen_ping6(port_info_t * info);
#endif
extern void pktgen_reset(port_info_t * info);
extern void pktgen_set_tx_count(port_info_t * info, uint32_t cnt);
extern void pktgen_set_port_seqCnt(port_info_t * info, uint32_t cnt);
extern void pktgen_set_port_prime(port_info_t * info, uint32_t cnt);
extern void pktgen_set_port_dump(port_info_t * info, uint32_t cnt);
extern void pktgen_set_tx_burst(port_info_t * info, uint32_t burst);
extern void pktgen_set_tx_cycles(port_info_t * info, uint32_t cycles);
extern void pktgen_set_rx_cycles(port_info_t * info, uint32_t cycles);
extern void pktgen_set_pkt_size(port_info_t * info, uint32_t size);
extern void pktgen_set_port_value(port_info_t * info, char type, uint32_t portValue);
extern void pktgen_set_tx_rate(port_info_t * info, uint32_t rate);
extern void pktgen_set_ipaddr(port_info_t * info, char type, cmdline_ipaddr_t * ip);
extern void pktgen_set_dst_mac(port_info_t * info, cmdline_etheraddr_t * mac);
extern void pktgen_range_enable_disable(port_info_t * info, char * str);
extern void pktgen_set_dest_mac(port_info_t * info, const char * what, cmdline_etheraddr_t * mac);
extern void pktgen_set_src_mac(port_info_t * info, const char * what, cmdline_etheraddr_t * mac);
extern void pktgen_set_src_ip(port_info_t * info, char * what, cmdline_ipaddr_t * ip);
extern void pktgen_set_dst_ip(port_info_t * info, char * what, cmdline_ipaddr_t * ip);
extern void pktgen_set_src_port(port_info_t * info, char * what, uint16_t port);
extern void pktgen_set_dst_port(port_info_t * info, char * what, uint16_t port);
extern void pktgen_send_arp_requests(port_info_t * info, uint32_t type);
extern void pktgen_set_page( char * str );
extern void pktgen_set_seq(port_info_t * info, uint32_t seqnum,
		cmdline_etheraddr_t *daddr, cmdline_etheraddr_t * saddr,
		cmdline_ipaddr_t * ip_daddr, cmdline_ipaddr_t * ip_saddr,
		uint32_t sport, uint32_t dport, char ip, char proto, uint16_t vlanid, uint32_t pktsize);
extern void pktgen_set_range_pkt_size(port_info_t * info, char * what, uint16_t size);
extern void pktgen_send_pkt(port_info_t * info, uint32_t seqnum);
extern void pktgen_recv_pkt(port_info_t * info);
extern void pktgen_dump_enable_disable(port_info_t *info, char * str);

extern void pktgen_compile_pkt(port_info_t * info, uint32_t seqnum,
		cmdline_etheraddr_t *daddr, cmdline_etheraddr_t * saddr,
		cmdline_ipaddr_t * ip_daddr, cmdline_ipaddr_t * ip_saddr,
		uint32_t sport, uint32_t dport, char type, char proto,
		uint16_t vlanid, uint32_t pktsize);

extern void pktgen_quit(void);

extern void pktgen_set_vlan(port_info_t * info, uint32_t onOff);
extern void pktgen_set_vlan_id(port_info_t * info, char * what, uint16_t id);
extern void pktgen_set_vlanid(port_info_t * info, uint16_t vlanid);

extern void pktgen_set_mpls(port_info_t * info, uint32_t onOff);
extern void pktgen_set_mpls_entry(port_info_t * info, uint32_t mpls_entry);

extern void pktgen_set_qinq(port_info_t * info, uint32_t onOff);
extern void pktgen_set_qinqids(port_info_t * info, uint16_t outerid, uint16_t innerid);

extern void pktgen_set_gre(port_info_t * info, uint32_t onOff);
extern void pktgen_set_gre_eth(port_info_t * info, uint32_t onOff);
extern void pktgen_set_gre_key(port_info_t * info, uint32_t gre_key);

extern void pktgen_garp_enable_disable(port_info_t * info, char * str);

extern void pktgen_mac_from_arp(uint32_t onOff);

extern void pktgen_set_random(port_info_t * info, uint32_t onOff);

#endif /* _PKTGEN_CMDS_H_ */
