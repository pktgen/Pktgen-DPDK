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
/* Created 2011 by Keith Wiles @ intel.com */

#define lpktgenlib_c
#define LUA_LIB

#include "lpktgenlib.h"

#include <stdint.h>
#include <netinet/in.h>

#include <cmdline_parse.h>
#include <cmdline_parse_etheraddr.h>
#include <cmdline_parse_ipaddr.h>
#include <cmdline_parse_portlist.h>
#include <lua-socket.h>
#include <lualib.h>

#include "pktgen-cmds.h"
#include "commands.h"

extern const char * help_info[];

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif


/* Defined in libwr_lua/lua_shell.c */
extern int execute_lua_string(lua_State * L, char * str);
extern int dolibrary(lua_State *L, const char *name);

/**************************************************************************//**
*
* setf_integer - Helper routine to set Lua variables.
*
* DESCRIPTION
* Helper routine to a set Lua variables.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
setf_integer(lua_State * L, const char * name, lua_Integer value) {
	lua_pushinteger(L, value);
	lua_setfield(L, -2, name);
}

/**************************************************************************//**
*
* setf_string - Helper routine to set Lua variables.
*
* DESCRIPTION
* Helper routine to a set Lua variables.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
setf_string(lua_State * L, const char * name, char * value) {
	lua_pushstring(L, value);
	lua_setfield(L, -2, name);
}

#if 0
/**************************************************************************//**
*
* setf_stringLen - Helper routine to set Lua variables.
*
* DESCRIPTION
* Helper routine to a set Lua variables.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
setf_stringLen(lua_State * L, const char * name, char * value, int len) {
	lua_pushlstring(L, value, len);
	lua_setfield(L, -2, name);
}

/**************************************************************************//**
*
* setf_udata - Helper routine to set Lua variables.
*
* DESCRIPTION
* Helper routine to a set Lua variables.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
setf_udata(lua_State * L, const char * name, void * value) {
	lua_pushlightuserdata(L, value);
	lua_setfield(L, -2, name);
}
#endif

static __inline__ void
getf_etheraddr(lua_State * L, const char * field, cmdline_etheraddr_t * value) {
	lua_getfield(L, 3, field);
	cmdline_parse_etheraddr(NULL, luaL_checkstring(L, -1), value, sizeof(cmdline_etheraddr_t));
	lua_pop(L, 1);
}

static __inline__ void
getf_ipaddr(lua_State * L, const char * field, void * value, uint32_t flags) {
	cmdline_parse_token_ipaddr_t tk;

	lua_getfield(L, 3, field);
	tk.ipaddr_data.flags = flags;
	cmdline_parse_ipaddr((cmdline_parse_token_hdr_t *)&tk,
	    luaL_checkstring(L, -1), value, sizeof(cmdline_ipaddr_t));
	lua_pop(L, 1);
}

static __inline__ uint32_t
getf_integer(lua_State * L, const char * field) {
	uint32_t	value;

	lua_getfield(L, 3, field);
	value 	= luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	return value;
}

static __inline__ char *
getf_string(lua_State * L, const char * field) {
	char	  * value;

	lua_getfield(L, 3, field);
	value 	= (char *)luaL_checkstring(L, -1);
	lua_pop(L, 1);

	return value;
}

static inline void parse_portlist(const char * buf, void * pl) {

#if (RTE_VERSION >= RTE_VERSION_NUM(2,0,0,0))
	cmdline_parse_portlist(NULL, buf, pl, sizeof(cmdline_portlist_t));
#else
	cmdline_parse_portlist(NULL, buf, pl, PORTLIST_TOKEN_SIZE);
#endif
}

/**************************************************************************//**
*
* pktgen_set - Set a number of Pktgen values.
*
* DESCRIPTION
* Set a number of Pktgen values for a given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_set (lua_State *L) {
	uint32_t	value;
	cmdline_portlist_t	portlist;
	char * what;
		
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "set, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	what = (char *)luaL_checkstring(L, 2);
	value = luaL_checkinteger(L, 3);

	foreach_port( portlist.map, _do(
		if (!strcasecmp(what, "count"))
			pktgen_set_tx_count(info, value);
		else if (!strcasecmp(what, "size"))
			pktgen_set_pkt_size(info, value);
		else if (!strcasecmp(what, "rate"))
			pktgen_set_tx_rate(info, value);
		else if (!strcasecmp(what, "burst"))
			pktgen_set_tx_burst(info, value);
		else if (!strcasecmp(what, "cycles"))
			pktgen_set_tx_cycles(info, value);
		else if (!strcasecmp(what, "sport"))
			pktgen_set_port_value(info, what[0], value);
		else if (!strcasecmp(what, "dport"))
			pktgen_set_port_value(info, what[0], value);
		else if (!strcasecmp(what, "seqCnt"))
			pktgen_set_port_seqCnt(info, value);
		else if (!strcasecmp(what, "prime"))
			pktgen_set_port_prime(info, value);
		else if (!strcasecmp(what, "dump"))
			pktgen_set_port_dump(info, value);
		else
			return luaL_error(L, "set does not support %s", what);
	) );

	pktgen_update_display();
	return 0;
}

 /**************************************************************************//**
 *
 * set_seq - Set the sequence data for a given port.
 *
 * DESCRIPTION
 * Set the sequence data for a given port and sequence number.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

 static int set_seq(lua_State * L, uint32_t seqnum)
 {
	cmdline_portlist_t	portlist;
	uint32_t	pktsize, sport, dport;
	uint16_t	vlanid;
	cmdline_etheraddr_t daddr;
	cmdline_etheraddr_t saddr;
	cmdline_ipaddr_t ip_daddr;
	cmdline_ipaddr_t ip_saddr;
	cmdline_parse_token_ipaddr_t tkd, tks;
	char * proto, * ip;

	parse_portlist(luaL_checkstring(L, 2), &portlist);
 	cmdline_parse_etheraddr(NULL, luaL_checkstring(L, 3), &daddr, sizeof(daddr));
 	cmdline_parse_etheraddr(NULL, luaL_checkstring(L, 4), &saddr, sizeof(saddr));
 	tkd.ipaddr_data.flags = CMDLINE_IPADDR_V4;
 	cmdline_parse_ipaddr((cmdline_parse_token_hdr_t *)&tkd,
 	    luaL_checkstring(L, 5), &ip_daddr, sizeof(cmdline_ipaddr_t));
 	tks.ipaddr_data.flags = CMDLINE_IPADDR_NETWORK | CMDLINE_IPADDR_V4;
 	cmdline_parse_ipaddr((cmdline_parse_token_hdr_t *)&tks,
 	    luaL_checkstring(L, 6), &ip_saddr, sizeof(cmdline_ipaddr_t));
 	sport 	= luaL_checkinteger(L, 7);
 	dport 	= luaL_checkinteger(L, 8);
 	proto	= (char *)luaL_checkstring(L, 9);
 	ip		= (char *)luaL_checkstring(L, 10);
 	vlanid	= luaL_checkinteger(L, 11);
 	pktsize = luaL_checkinteger(L, 12);

 	if ( (proto[0] == 'i') && (ip[3] == '6') ) {
 		lua_putstring(L, "Must use IPv4 with ICMP type packets\n");
 		return -1;
 	}

 	foreach_port(portlist.map,
 		pktgen_set_seq(info, seqnum, &daddr, &saddr, &ip_daddr, &ip_saddr,
 				sport, dport, ip[3], proto[0], vlanid, pktsize) );

 	pktgen_update_display();

 	return 0;
 }

 /**************************************************************************//**
 *
 * pktgen_seq - Set the sequence data for a given port.
 *
 * DESCRIPTION
 * Set the sequence data for a given port and sequence number.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int pktgen_seq (lua_State *L) {
	uint32_t	seqnum;
		
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "seq, wrong number of arguments");
	case 12:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if ( seqnum >= NUM_SEQ_PKTS )
		return -1;

	return set_seq(L, seqnum);
}

/**************************************************************************//**
*
* set_seqTable - Set the sequence data for a given port.
*
* DESCRIPTION
* Set the sequence data for a given port and sequence number.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int set_seqTable(lua_State * L, uint32_t seqnum)
{
	cmdline_portlist_t	portlist;
	uint32_t	pktSize, sport, dport;
	uint16_t	vlanid;
	cmdline_etheraddr_t daddr;
	cmdline_etheraddr_t saddr;
	cmdline_ipaddr_t ip_daddr;
	cmdline_ipaddr_t ip_saddr;
	char * ipProto, * ethType;

	parse_portlist(luaL_checkstring(L, 2), &portlist);

	getf_etheraddr(L, "eth_dst_addr", &daddr);
	getf_etheraddr(L, "eth_src_addr", &saddr);
	getf_ipaddr(L, "ip_dst_addr", &ip_daddr, CMDLINE_IPADDR_V4);
	getf_ipaddr(L, "ip_src_addr", &ip_saddr, CMDLINE_IPADDR_NETWORK | CMDLINE_IPADDR_V4);
	sport		= getf_integer(L, "sport");
	dport		= getf_integer(L, "dport");
	ipProto		= getf_string(L, "ipProto");
	ethType		= getf_string(L, "ethType");
	vlanid		= getf_integer(L, "vlanid");
	pktSize		= getf_integer(L, "pktSize");

	if ( (ipProto[0] == 'i') && (ethType[3] == '6') ) {
		lua_putstring(L, "Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	foreach_port(portlist.map,
		pktgen_set_seq(info, seqnum, &daddr, &saddr, &ip_daddr, &ip_saddr,
				sport, dport, ethType[3], ipProto[0], vlanid, pktSize) );

	pktgen_update_display();

	return 0;
}

/**************************************************************************//**
*
* pktgen_seqTable - Set the sequence data for a given port.
*
* DESCRIPTION
* Set the sequence data for a given port and sequence number.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_seqTable (lua_State *L) {
	uint32_t	seqnum;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "seqTable, wrong number of arguments");
	case 3:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if ( seqnum >= NUM_SEQ_PKTS )
		return -1;

	return set_seqTable(L, seqnum);
}

/**************************************************************************//**
*
* pktgen_ports_per_page - Set the number of ports per page.
*
* DESCRIPTION
* Set the number of ports per page.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_ports_per_page(lua_State *L) {
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "ports_per_page, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_set_page_size(luaL_checkinteger(L,1));
	return 0;
}

/**************************************************************************//**
*
* pktgen_icmp - Enable or Disable ICMP echo processing.
*
* DESCRIPTION
* Enable or disable ICMP echo process for a given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_icmp (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "icmp, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	foreach_port( portlist.map,
			pktgen_set_icmp_echo(info, parseState((char *)luaL_checkstring(L, 2))) );
	return 0;
}

/**************************************************************************//**
*
* pktgen_sendARP - Send ARP type packets from a given port list.
*
* DESCRIPTION
* Send APR request and gratuitous packets for a given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_sendARP (lua_State *L) {
	cmdline_portlist_t	portlist;
	char * what;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "sendARP, wrong number of arguments");
	case 2:
		break;
	}
	what = (char *)luaL_checkstring(L, 2);
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	foreach_port(portlist.map,
			pktgen_send_arp_requests(info, (what[0] == 'g') ? GRATUITOUS_ARP : 0) );
	return 0;
}

/**************************************************************************//**
*
* pktgen_set_mac - Set the MAC address for a set of ports.
*
* DESCRIPTION
* Set the MAC address for a set of ports.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_set_mac (lua_State *L) {
	cmdline_portlist_t	portlist;
	cmdline_etheraddr_t mac;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "set_mac, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	cmdline_parse_etheraddr(NULL, luaL_checkstring(L, 2), &mac, sizeof(mac));

	foreach_port(portlist.map,
		pktgen_set_dst_mac(info, &mac) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_macFromArp - Enable or Disable getting MAC address from ARP packets.
*
* DESCRIPTION
* Enable or disable getting MAC address from an ARP request.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_macFromArp (lua_State *L) {
	char * state;
	uint32_t	onOff;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "mac_from_arp, wrong number of arguments");
	case 1:
		break;
	}
	state = (char *)luaL_checkstring(L, 1);
	
	onOff = parseState(state);

	pktgen_mac_from_arp(onOff);

	return 0;
}

/**************************************************************************//**
*
* pktgen_prototype - Set the packet protocol type.
*
* DESCRIPTION
* Set the packet protocol type.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_prototype (lua_State *L) {
	char * type;
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "set_proto, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	type = (char *)luaL_checkstring(L, 2);

	foreach_port(portlist.map,
		pktgen_set_proto(info, type[0]) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_set_ip_addr - Set the ip address value for src and dst.
*
* DESCRIPTION
* Set the IP address for src and dst.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_set_ip_addr (lua_State *L) {
	cmdline_portlist_t	portlist;
	cmdline_ipaddr_t ipaddr;
	cmdline_parse_token_ipaddr_t tk;
	char	  * type;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "set_ipaddr, wrong number of arguments");
	case 3:
		break;
	}
	type = (char *)luaL_checkstring(L, 2);
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	tk.ipaddr_data.flags = CMDLINE_IPADDR_V4;
	if ( type[0] == 's' )
		tk.ipaddr_data.flags |= CMDLINE_IPADDR_NETWORK;
	cmdline_parse_ipaddr((cmdline_parse_token_hdr_t *)&tk, luaL_checkstring(L, 3), &ipaddr, sizeof(cmdline_ipaddr_t));

	foreach_port( portlist.map,
		pktgen_set_ipaddr(info, type[0], &ipaddr) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_set_type - Set the type of packet IPv4/v6
*
* DESCRIPTION
* Set the port packet types to IPv4 or v6.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_set_type (lua_State *L) {
	char * type;
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "set_type, wrong number of arguments");
	case 2:
		break;
	}
	type = (char *)luaL_checkstring(L, 2);
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_pkt_type(info, type) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_send_ping4 - Send ping packets for IPv4
*
* DESCRIPTION
* Send a ping packet for IPv4.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_send_ping4 (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "ping4, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_ping4(info) );

	return 0;
}

#ifdef INCLUDE_PING6
/**************************************************************************//**
*
* pktgen_send_ping6 - Send IPv6 ICMP echo requests.
*
* DESCRIPTION
* Send IPv6 ICMP echo requests.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_send_ping6 (lua_State *L) {
	cmdline_portlist_t	portlist;


	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "ping6, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_ping6(info) );

	return 0;
}
#endif

/**************************************************************************//**
*
* pktgen_pcap - Enable or disable PCAP support sending.
*
* DESCRIPTION
* Enable or disable PCAP sending.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_pcap (lua_State *L) {
	cmdline_portlist_t	portlist;
	char * what;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "pcap, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	what = (char *)luaL_checkstring(L, 2);

	foreach_port( portlist.map,
		pktgen_pcap_enable_disable(info, what) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_start - Start ports sending packets.
*
* DESCRIPTION
* Start ports sending packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_start (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "start, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_start_transmitting(info) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_stop - Stop ports from sending packets
*
* DESCRIPTION
* Stop port from sending packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_stop (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "stop, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_stop_transmitting(info) );
	return 0;
}

/**************************************************************************//**
*
* pktgen_scrn - Enable or Disable the screen updates.
*
* DESCRIPTION
* Enable or disable screen updates.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_scrn (lua_State *L) {
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "screen, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_screen((char *)luaL_checkstring(L, 1));
	return 0;
}

/**************************************************************************//**
*
* pktgen_prime - Send a set of packet to prime the forwarding tables.
*
* DESCRIPTION
* Send a small set of packet to prime the forwarding table on a port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_prime (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "prime, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_prime_ports(info) );
	return 0;
}

static __inline__ void __delay(int32_t t) {
	int32_t		n;

	while( t > 0 ) {
		rte_timer_manage();
		n = (t > 10)? 10 : t;
		rte_delay_ms(n);
		t -= n;
	}
}

/**************************************************************************//**
*
* pktgen_delay - Delay for a given number of milliseconds.
*
* DESCRIPTION
* Delay a script for a given number of milliseconds.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_delay (lua_State *L) {
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "delay, wrong number of arguments");
	case 1:
		break;
	}

	__delay(luaL_checkinteger(L, 1));

	return 0;
}

/**************************************************************************//**
*
* pktgen_pause - Delay for a given number of milliseconds and display a message
*
* DESCRIPTION
* Delay a script for a given number of milliseconds and display a message
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_pause (lua_State *L) {
	char * str;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "pause, wrong number of arguments");
	case 2:
		break;
	}
	str = (char *)luaL_checkstring(L, 1);
	if ( strlen(str) > 0 )
		lua_putstring(L, str);

	__delay(luaL_checkinteger(L, 2));
	return 0;
}

/**************************************************************************//**
*
* pktgen_continue - Display a message and wait for a single keyboard input.
*
* DESCRIPTION
* Display a message and wait for a keyboard input.
*
* RETURNS: the single keyboard character typed as a string.
*
* SEE ALSO:
*/

static int pktgen_continue (lua_State *L) {
	char	buf[4], * str;
	int		n;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "continue, wrong number of arguments");
	case 1:
		break;
	}
	str = (char *)luaL_checkstring(L, 1);

	if ( strlen(str) > 0 )
		lua_putstring(L, str);

	buf[0] = '\0';
	n = fread(buf, 1, 1, (FILE *)_get_stdin(L));
	if ( n > 0 )
		buf[n] = '\0';

	lua_pushstring(L, buf);
	return 1;
}

/**************************************************************************//**
*
* pktgen_input - Display a message and wait for keyboard input.
*
* DESCRIPTION
* Display a message and wait for a keyboard input.
*
* RETURNS: keyboard string typed at display
*
* SEE ALSO:
*/

static int pktgen_input (lua_State *L) {
	char	buf[256], c, * str;
	uint32_t	n, idx;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "input, wrong number of arguments");
	case 1:
		break;
	}
	str = (char *)luaL_checkstring(L, 1);

	if ( strlen(str) > 0 )
		lua_putstring(L, str);

	idx = 0;
	buf[idx] = '\0';
	while ( idx < (sizeof(buf) - 2) ) {
		n = fread(&c, 1, 1, (FILE *)_get_stdin(L));
		if ( (n <= 0) || (c == '\r') || (c == '\n') )
			break;
		buf[idx++] = c;
	}
	buf[idx] = '\0';

	lua_pushstring(L, buf);
	return 1;
}

/**************************************************************************//**
*
* pktgen_sleep - Sleep for a given number of seconds.
*
* DESCRIPTION
* Delay a script for a given number of seconds.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_sleep (lua_State *L) {
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "sleep, wrong number of arguments");
	case 1:
		break;
	}
	rte_delay_ms(luaL_checkinteger(L, 1) * 1000);
	return 0;
}

/**************************************************************************//**
*
* pktgen_load - Load and execute a script.
*
* DESCRIPTION
* Load and execute a script
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_load (lua_State *L) {
	char * path;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "load, wrong number of arguments");
	case 1:
		break;
	}
	path = (char *)luaL_checkstring(L, 1);
	
	if ( pktgen_load_cmds(path) )
		return luaL_error(L, "load command failed for %s\n", path);
	return 0;
}

/**************************************************************************//**
*
* pktgen_config_save - Save to a configuration file.
*
* DESCRIPTION
* Save configuration to a file.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_config_save (lua_State *L) {
	char * path;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "save, wrong number of arguments");
	case 1:
		break;
	}
	path = (char *)luaL_checkstring(L, 1);

	if ( pktgen_save(path) )
		return luaL_error(L, "save command failed for %s\n", path);
	return 0;
}

/**************************************************************************//**
*
* pktgen_clear - Clear all port statistics
*
* DESCRIPTION
* Clear all port statistics to zero for a given port
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_clear (lua_State *L) {

	cmdline_portlist_t	portlist;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "clear, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_clear_stats(info) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_clear_all - Clear all port statistics
*
* DESCRIPTION
* Clear all port statistics to zero for a given port
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_clear_all (__attribute__ ((unused)) lua_State *L) {

	forall_ports( pktgen_clear_stats(info) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_cls_screen - Clear and redraw the screen
*
* DESCRIPTION
* Clear and redraw the screen
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_cls_screen (__attribute__ ((unused)) lua_State *L) {

	pktgen_cls();

	return 0;
}

/**************************************************************************//**
*
* pktgen_update - Update the screen information
*
* DESCRIPTION
* Update the screen information
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_update_screen (__attribute__ ((unused)) lua_State *L) {

	pktgen_update();

	return 0;
}

/**************************************************************************//**
*
* pktgen_reset - Reset pktgen to all default values.
*
* DESCRIPTION
* Reset pktgen to all default values.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_reset_config (lua_State *L) {
	cmdline_portlist_t	portlist;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "reset, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_reset(info) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_dst_mac - Set a destination MAC address
*
* DESCRIPTION
* Set a destination MAC address.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_dst_mac (lua_State *L) {
	cmdline_portlist_t	portlist;
	cmdline_etheraddr_t mac;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "dst_mac, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	cmdline_parse_etheraddr(NULL, luaL_checkstring(L, 2), &mac, sizeof(mac));

	foreach_port( portlist.map,
		pktgen_set_dest_mac(info, "start", &mac) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_src_mac - Set the source MAC address in the range data.
*
* DESCRIPTION
* Set the source MAC address for a given set of ports in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_src_mac (lua_State *L) {
	cmdline_portlist_t	portlist;
	cmdline_etheraddr_t mac;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "src_mac, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	cmdline_parse_etheraddr(NULL, luaL_checkstring(L, 2), &mac, sizeof(mac));

	foreach_port( portlist.map,
		pktgen_set_src_mac(info, "start", &mac) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_dst_ip - Set the IP address in the range data.
*
* DESCRIPTION
* Set the IP address in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_dst_ip (lua_State *L) {
	cmdline_portlist_t	portlist;
	cmdline_ipaddr_t ipaddr;
	cmdline_parse_token_ipaddr_t tk;
	char	  * type;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "dst_ip, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	tk.ipaddr_data.flags = CMDLINE_IPADDR_V4;
	cmdline_parse_ipaddr((cmdline_parse_token_hdr_t *)&tk, luaL_checkstring(L, 3), &ipaddr, sizeof(cmdline_ipaddr_t));

	type = (char *)luaL_checkstring(L, 2);
	foreach_port( portlist.map,
		pktgen_set_dst_ip(info, type, &ipaddr) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_src_ip - Set the source IP address in the range data.
*
* DESCRIPTION
* Set the source IP address in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_src_ip (lua_State *L) {
	cmdline_portlist_t	portlist;
	cmdline_ipaddr_t ipaddr;
	cmdline_parse_token_ipaddr_t tk;
	char	  * type;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "src_ip, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	tk.ipaddr_data.flags = CMDLINE_IPADDR_V4;
	cmdline_parse_ipaddr((cmdline_parse_token_hdr_t *)&tk, luaL_checkstring(L, 3), &ipaddr, sizeof(ipaddr));

	type = (char *)luaL_checkstring(L, 2);
	foreach_port( portlist.map,
		pktgen_set_src_ip(info, type, &ipaddr) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_dst_port - Set the port type in the range data.
*
* DESCRIPTION
* Set the port type in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_dst_port (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "dst_port, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_dst_port(info, (char *)luaL_checkstring(L, 2), luaL_checkinteger(L, 3)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_src_port - Set the source port value in the range data.
*
* DESCRIPTION
* Set the source port value in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_src_port (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "src_port, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_src_port(info, (char *)luaL_checkstring(L, 2), luaL_checkinteger(L, 3)));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_vlan_id - Set the VLAN id in the range data.
*
* DESCRIPTION
* Set the VLAN id in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_vlan_id (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t vlan_id;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "vlan_id, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	vlan_id = luaL_checkinteger(L, 3);
	if ( (vlan_id < MIN_VLAN_ID) || (vlan_id > MAX_VLAN_ID) )
		vlan_id = 1;

	foreach_port( portlist.map,
		pktgen_set_vlan_id(info, (char *)luaL_checkstring(L, 2), vlan_id) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_vlanid - Set the VLAN id for a single port
*
* DESCRIPTION
* Set the VLAN id for a single port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_vlanid (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t vlanid;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "vlanid, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	vlanid = luaL_checkinteger(L, 2);
	if ( (vlanid < MIN_VLAN_ID) || (vlanid > MAX_VLAN_ID) )
		vlanid = 1;

	foreach_port( portlist.map,
		pktgen_set_vlanid(info, vlanid) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_vlan - Enable or Disable vlan header
*
* DESCRIPTION
* Enable or disable insertion of VLAN header.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_vlan (lua_State *L) {
	cmdline_portlist_t	portlist;
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_vlan(info, parseState(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_mpls_entry - Set the MPLS entry in the range data.
*
* DESCRIPTION
* Set the VLAN id in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_mpls_entry (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t mpls_entry;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "mpls_entry, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	mpls_entry = strtoul(luaL_checkstring(L, 2), NULL, 16);

	foreach_port( portlist.map,
		pktgen_set_mpls_entry(info, mpls_entry) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_mpls - Enable or Disable MPLS header
*
* DESCRIPTION
* Enable or disable insertion of MPLS header.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_mpls (lua_State *L) {
	cmdline_portlist_t	portlist;
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "mpls, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_mpls(info, parseState(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_qinqids - Set the Q-in-Q ID's in the range data.
*
* DESCRIPTION
* Set the Q-in-Q ID's in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_qinqids (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t qinq_id1, qinq_id2;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "qinqids, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	qinq_id1 = luaL_checkinteger(L, 2);
	if ( (qinq_id1 < MIN_VLAN_ID) || (qinq_id1 > MAX_VLAN_ID) )
		qinq_id1 = 1;

	qinq_id2 = luaL_checkinteger(L, 3);
	if ( (qinq_id2 < MIN_VLAN_ID) || (qinq_id2 > MAX_VLAN_ID) )
		qinq_id2 = 1;

	foreach_port( portlist.map,
		pktgen_set_qinqids(info, qinq_id1, qinq_id2) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_qinq - Enable or Disable Q-in-Q header
*
* DESCRIPTION
* Enable or disable insertion of Q-in-Q header.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_qinq (lua_State *L) {
	cmdline_portlist_t	portlist;
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "qinq, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_qinq(info, parseState(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_gre_key - Set the GRE key in the range data.
*
* DESCRIPTION
* Set the GRE key in the range data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_gre_key (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t gre_key;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "gre_key, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	gre_key = luaL_checkinteger(L, 2);

	foreach_port( portlist.map,
		pktgen_set_gre_key(info, gre_key) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_gre - Enable or Disable GRE with IPv4 payload
*
* DESCRIPTION
* Enable or disable GRE with IPv4 payload.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_gre (lua_State *L) {
	cmdline_portlist_t	portlist;
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "gre, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_gre(info, parseState(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_gre_eth - Enable or Disable GRE with Ethernet payload
*
* DESCRIPTION
* Enable or disable GRE with Ethernet payload
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_gre_eth (lua_State *L) {
	cmdline_portlist_t	portlist;
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "gre_eth, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_set_gre_eth(info, parseState(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_pkt_size - Set the port range size.
*
* DESCRIPTION
* Set the port range size.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_pkt_size (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	size;
	char	  * type;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "pkt_size, wrong number of arguments");
	case 3:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	type = (char *)luaL_checkstring(L, 2);
	size = luaL_checkinteger(L, 3);

	foreach_port( portlist.map,
		pktgen_set_range_pkt_size(info, type, size) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_range - Enable or disable the range data sending.
*
* DESCRIPTION
* Enable or disable the range data sending.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_range (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "range, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_range_enable_disable(info, (char *)luaL_checkstring(L,2)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_page - Set the page type to be displayed.
*
* DESCRIPTION
* Set the page type to be displayed.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_page (lua_State *L) {
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "page, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_set_page((char *)luaL_checkstring(L, 1));
	return 0;
}

/**************************************************************************//**
*
* pktgen_port - Set the port type number
*
* DESCRIPTION
* Set the port type IPv4 or IPv6
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_port (lua_State *L) {
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "port, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_set_port_number(luaL_checkinteger(L, 1));
	return 0;
}

/**************************************************************************//**
*
* pktgen_process - Enable or Disable input packet processing.
*
* DESCRIPTION
* Enable or disable input packet processing.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_process (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_process_enable_disable(info, (char *)luaL_checkstring(L, 2)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_garp - Enable or Disable GARP packet processing.
*
* DESCRIPTION
* Enable or disable GARP packet processing.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_garp (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
		pktgen_garp_enable_disable(info, (char *)luaL_checkstring(L, 2)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
*
* pktgen_blink - Enable or disable port Led blinking.
*
* DESCRIPTION
* Enable or disable port led blinking.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_blink (lua_State *L) {
	cmdline_portlist_t	portlist;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "blink, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port( portlist.map,
			pktgen_blink_enable_disable(info, (char *)luaL_checkstring(L, 2)) );

	if ( pktgen.blinklist )
		pktgen.flags |= BLINK_PORTS_FLAG;
	else
		pktgen.flags &= ~BLINK_PORTS_FLAG;
	pktgen_update_display();

	return 0;
}

/**************************************************************************//**
*
* isSending - Get the current state of the transmitter on a port.
*
* DESCRIPTION
* Get the current state of the transmitter on a port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void isSending(lua_State * L, port_info_t * info)
{
	lua_pushinteger(L, info->pid);		// Push the table index
	lua_pushinteger(L, pktgen_port_transmitting(info->pid));

	// Now set the table as an array with pid as the index.
	lua_rawset(L, -3);
}

/**************************************************************************//**
*
* pktgen_isSending - Get the current state of the transmitter on a port.
*
* DESCRIPTION
* Get the current state of the transmitter on a port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_isSending (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	n;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "isSending, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port( portlist.map,
			_do( isSending(L, info); n++ ) );

	setf_integer(L, "n", n);

	return 1;
}

/**************************************************************************//**
*
* link_state - Get the current link state of a port.
*
* DESCRIPTION
* Get the current link state of a port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void link_state(lua_State * L, port_info_t * info)
{
	char buff[32];
	
	lua_pushinteger(L, info->pid);		// Push the table index
	lua_pushstring(L, pktgen_link_state(info->pid, buff, sizeof(buff)));

	// Now set the table as an array with pid as the index.
	lua_rawset(L, -3);
}

/**************************************************************************//**
*
* pktgen_linkState - Get the current link state of a port.
*
* DESCRIPTION
* Get the current link state of a port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_linkState (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	n;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "linkState, wrong number of arguments");
	case 1:
		break;
	}
	portlist.map = 0;
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port( portlist.map,
			_do( link_state(L, info); n++ ) );

	setf_integer(L, "n", n);

	return 1;
}

/**************************************************************************//**
*
* port_sizes - return port size stats on a port
*
* DESCRIPTION
* Return the stats on packet sizes for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void port_sizes(lua_State * L, port_info_t * info)
{
	port_sizes_t	sizes;

	pktgen_port_sizes(info->pid, &sizes);

	lua_pushinteger(L, info->pid);		// Push the table index
	lua_newtable(L);						// Create the structure table for a packet

	setf_integer(L, "runt", sizes.runt);
	setf_integer(L, "_64", sizes._64);
	setf_integer(L, "_65_127", sizes._65_127);
	setf_integer(L, "_128_255", sizes._128_255);
	setf_integer(L, "_256_511", sizes._256_511);
	setf_integer(L, "_512_1023", sizes._512_1023);
	setf_integer(L, "_1024_1518", sizes._1024_1518);
	setf_integer(L, "jumbo", sizes.jumbo);
	setf_integer(L, "broadcast", sizes.broadcast);
	setf_integer(L, "multicast", sizes.multicast);

	// Now set the table as an array with pid as the index.
	lua_rawset(L, -3);
}

/**************************************************************************//**
*
* pktgen_portSizes - return port size stats on a port
*
* DESCRIPTION
* Return the stats on packet sizes for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_portSizes (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	n;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "portSizes, wrong number of arguments");
	case 1:
		break;
	}
	portlist.map = 0;
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	
	lua_newtable(L);

	n = 0;
	foreach_port( portlist.map,
			_do( port_sizes(L, info); n++ ) );

	setf_integer(L, "n", n);

	return 1;
}

/**************************************************************************//**
*
* pkt_stats - Return the other packet stats for a given port.
*
* DESCRIPTION
* Return the packet stats for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void pkt_stats(lua_State * L, port_info_t * info)
{
	pkt_stats_t	stats;

	pktgen_pkt_stats(info->pid, &stats);

	lua_pushinteger(L, info->pid);		// Push the table index
	lua_newtable(L);						// Create the structure table for a packet

	setf_integer(L, "arp_pkts", stats.arp_pkts);
	setf_integer(L, "echo_pkts", stats.echo_pkts);
	setf_integer(L, "ip_pkts", stats.ip_pkts);
	setf_integer(L, "ipv6_pkts", stats.ipv6_pkts);
	setf_integer(L, "vlan_pkts", stats.vlan_pkts);
	setf_integer(L, "dropped_pkts", stats.dropped_pkts);
	setf_integer(L, "unknown_pkts", stats.unknown_pkts);
	setf_integer(L, "tx_failed", stats.tx_failed);

	// Now set the table as an array with pid as the index.
	lua_rawset(L, -3);
}

/**************************************************************************//**
*
* pktgen_pktStats - Return the other packet stats for a given port.
*
* DESCRIPTION
* Return the packet stats for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_pktStats (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	n;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "pktStats, wrong number of arguments");
	case 1:
		break;
	}
	portlist.map = 0;
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	
	lua_newtable(L);

	n = 0;
	foreach_port( portlist.map,
			_do( pkt_stats(L, info); n++ ) );

	setf_integer(L, "n", n);

	return 1;
}

/**************************************************************************//**
*
* port_stats - Return the other port stats for a given ports.
*
* DESCRIPTION
* Return the other port stats for a given ports.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void port_stats(lua_State * L, port_info_t * info, char * type )
{
	eth_stats_t	stats;

	pktgen_port_stats(info->pid, type, &stats);

	lua_pushinteger(L, info->pid);		// Push the table index
	lua_newtable(L);						// Create the structure table for a packet

	setf_integer(L, "ipackets", stats.ipackets);
	setf_integer(L, "opackets", stats.opackets);
	setf_integer(L, "ibytes", stats.ibytes);
	setf_integer(L, "obytes", stats.obytes);
	setf_integer(L, "ierrors", stats.ierrors);
	setf_integer(L, "oerrors", stats.oerrors);
	setf_integer(L, "rx_nombuf", stats.rx_nombuf);

	if ( strcmp(type, "rate") == 0 ) {
		setf_integer(L, "pkts_rx", stats.ipackets);
		setf_integer(L, "pkts_tx", stats.opackets);
		setf_integer(L, "mbits_rx", iBitsTotal(stats)/Million);
		setf_integer(L, "mbits_tx", oBitsTotal(stats)/Million);
	}

	// Now set the table as an array with pid as the index.
	lua_rawset(L, -3);
}

/**************************************************************************//**
*
* pktgen_portStats - Return the other port stats for a given ports.
*
* DESCRIPTION
* Return the other port stats for a given ports.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_portStats (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	n;
	char * type;
	
	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "portStats, wrong number of arguments");
	case 2:
		break;
	}

	parse_portlist(luaL_checkstring(L, 1), &portlist);
	type = (char *)luaL_checkstring(L, 2);

	lua_newtable(L);

	n = 0;
	foreach_port( portlist.map,
			_do( port_stats(L, info, type); n++ ) );

	setf_integer(L, "n", n);

	return 1;
}

/**************************************************************************//**
*
* pktgen_help - Display the current help information.
*
* DESCRIPTION
* Display the current help information.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_help (lua_State *L) {
	int		i;
	
	lua_concat(L, 0);
	for(i=1; help_info[i] != NULL; i++ ) {
		if ( strcmp(help_info[i], "<<PageBreak>>") != 0 ) {
			lua_pushstring(L, help_info[i]);
			lua_concat(L, 2);
		}
	}

	return 1;
}

/**************************************************************************//**
*
* pktgen_compile - Compile a packet data structure into a real packet.
*
* DESCRIPTION
* Compile a packet data structure into real packet data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_compile (lua_State *L) {
	uint32_t	seqnum;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "compile, wrong number of arguments");
	case 3:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if ( seqnum >= NUM_SEQ_PKTS )
		return -1;

	return set_seqTable(L, seqnum);
}

/**************************************************************************//**
*
* decompile_pkt - Convert a packet to a data structure.
*
* DESCRIPTION
* Convert a packet to a data structure.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void decompile_pkt(lua_State * L, port_info_t * info, uint32_t seqnum) {
	char	buff[32];
	pkt_seq_t * p;

	p = &info->seq_pkt[seqnum];

	lua_pushinteger(L, info->pid);		// Push the table index
	lua_newtable(L);						// Create the structure table for a packet

	// Add each member to the packet table indexed with port id.
	setf_string(L, "eth_dst_addr", inet_mtoa(buff, sizeof(buff), &p->eth_dst_addr));
	setf_string(L, "eth_src_addr", inet_mtoa(buff, sizeof(buff), &p->eth_src_addr));
	setf_string(L, "ip_dst_addr", inet_ntop4(buff, sizeof(buff), htonl(p->ip_dst_addr), 0xFFFFFFFF));
	setf_string(L, "ip_src_addr", inet_ntop4(buff, sizeof(buff), htonl(p->ip_dst_addr), p->ip_mask));
	setf_integer(L, "dport", p->dport);
	setf_integer(L, "sport", p->sport);
	setf_integer(L, "vlanid", p->vlanid);
	setf_string(L, "ethType", (char *)(
			(p->ethType == ETHER_TYPE_IPv4)? "ipv4" :
			(p->ethType == ETHER_TYPE_IPv6)? "ipv6" :
			(p->ethType == ETHER_TYPE_VLAN)? "vlan" : "unknown"));
	setf_string(L, "ipProto", (char *)(
			(p->ipProto == PG_IPPROTO_TCP)? "tcp" :
			(p->ipProto == PG_IPPROTO_ICMP)? "icmp" : "udp"));

	setf_integer(L, "pktSize", p->pktSize+FCS_SIZE);
	setf_integer(L, "tlen", p->tlen);

	// Now set the table as an array with pid as the index.
	lua_rawset(L, -3);
}

/**************************************************************************//**
*
* pktgen_decompile - Convert a packet to a data structure.
*
* DESCRIPTION
* Convert a packet to a data structure.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_decompile (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	seqnum, n;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "decompile, wrong number of arguments");
	case 2:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if ( seqnum >= NUM_SEQ_PKTS )
		return 0;
	parse_portlist(luaL_checkstring(L, 2), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port( portlist.map,
			_do ( decompile_pkt(L, info, seqnum); n++ ) );

	setf_integer(L, "n", n);

	return 1;
}
/**************************************************************************//**
*
* pktgen_sendPkt - send the compiled packet to a given port.
*
* DESCRIPTION
* Send the compiled packet to the given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_sendPkt (lua_State *L) {
	cmdline_portlist_t	portlist;
	uint32_t	seqnum;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "sendPkt, wrong number of arguments");
	case 2:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);

	seqnum = luaL_checkinteger(L, 2);
	if ( (seqnum >= NUM_EXTRA_TX_PKTS) || (portlist.map == 0) )
		return 0;

	foreach_port( portlist.map,
		pktgen_send_pkt(info, seqnum) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_portCount - Return number of ports used
*
* DESCRIPTION
* Return the number of ports used
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_portCount (lua_State *L) {

	lua_pushinteger(L, pktgen.port_cnt);

	return 1;
}

/**************************************************************************//**
*
* pktgen_totalPorts - Return the total number of ports
*
* DESCRIPTION
* Return the total number of ports seen by DPDK
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_totalPorts(lua_State *L) {

	lua_pushinteger(L, pktgen.nb_ports);

	return 1;
}

/**************************************************************************//**
*
* pktgen_recvPkt - Receive a packet from a port.
*
* DESCRIPTION
* Receive a packer from a port to be decompiled later.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_recvPkt (lua_State *L) {
	cmdline_portlist_t	portlist;

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "recvPkt, wrong number of arguments");
	case 1:
		break;
	}
	parse_portlist(luaL_checkstring(L, 1), &portlist);
	if ( portlist.map == 0 )
		return 0;

	foreach_port( portlist.map,
		pktgen_recv_pkt(info) );

	return 0;
}

/**************************************************************************//**
*
* pktgen_run - Run a Lua or command script on the local disk or in a string.
*
* DESCRIPTION
* Run a script file on the local disk, can be Lua or command lines.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_run (lua_State *L) {

	switch( lua_gettop(L) ) {
	default: return luaL_error(L, "run( ['cmd'|'lua'], <string_or_path>), arguments wrong.");
	case 3:
		break;
	}

	// A cmd executes a file on the disk and can be a lua Script of command line file.
	if ( strcasecmp("cmd", luaL_checkstring(L, 1)) == 0 )
		pktgen_load_cmds((char *)luaL_checkstring(L, 2));
	else if ( strcasecmp("lua", luaL_checkstring(L, 1)) == 0 )		// Only a Lua script in memory.
		execute_lua_string(L, (char *)luaL_checkstring(L,2));
	else
		return luaL_error(L, "run( ['cmd'|'lua'], <string>), arguments wrong.");

	return 0;
}

static const char * lua_help_info[] = {
	"Pktgen Lua functions and values using pktgen.XXX to access\n",
	"set            - Set a number of options\n",
	"set_mac        - Set the MAC address for a port\n",
	"set_ipaddr     - Set the src and dst IP addresses\n",
	"mac_from_arp   - Configure MAC from ARP packet\n",
	"set_proto      - Set the prototype value\n",
	"set_type       - Set the type value\n",
	"\n",
	"seq            - Set the sequence data for a port\n",
	"seqTable       - Set the sequence data for a port using tables\n",
	"ports_per_page - Set the number of ports per page\n",
	"icmp_echo      - Enable/disable ICMP echo support\n",
	"send_arp       - Send a ARP request or GRATUITOUS_ARP\n",
	"pcap           - Load a PCAP file\n",
	"ping4          - Send a Ping IPv4 packet (ICMP echo)\n",
#ifdef INCLUDE_PING6
	"ping6          - Send a Ping IPv6 packet (ICMP echo)\n",
#endif
	"start          - Start a set of ports sending packets\n",
	"stop           - Stop a set of ports sending packets\n",
	"screen         - Turn off and on the screen updates\n",
	"prime          - Send a small number of packets to prime the routes\n",
	"delay          - Delay a given number of milliseconds\n",
	"pause          - Delay for a given number of milliseconds and display message\n",
	"sleep          - Delay a given number of seconds\n",
	"load           - load and run a command file or Lua file.\n",
	"save           - Save the configuration of Pktgen to a file.\n",
	"clear          - Clear stats for the given ports\n",
	"clr            - Clear all stats on all ports\n",
	"cls            - Redraw the screen\n",
	"\n",
	"update         - Update the screen information\n",
	"reset          - Reset the configuration to all ports\n",
	"vlan           - Enable or disable VLAN header\n",
	"mpls           - Enable or disable MPLS header\n",
    "qinq           - Enable or disable Q-in-Q header\n",
	"gre            - Enable or disable GRE with IPv4 payload\n",
	"gre_eth        - Enable or disable GRE with Ethernet payload\n",
	"\n",
	"Range commands\n",
	"dst_mac        - Set the destination MAC address for a port\n",
	"src_mac        - Set the src MAC address for a port\n",
	"src_ip         - Set the source IP address and netmask value\n",
	"dst_ip         - Set the destination IP address\n",
	"src_port       - Set the IP source port number\n",
	"dst_port       - Set the IP destination port number\n",
	"vlan_id        - Set the vlan id value\n",
	"mpls_entry     - Set the MPLS entry\n",
    "qinqids        - Set the Q-in-Q ID's\n",
    "gre_key        - Set the GRE key\n",
	"pkt_size       - the packet size for a range port\n",
	"range          - Enable or disable sending range data on a port.\n",
	"\n",
	"page           - Select a page to display, seq, range, pcap and a number from 0-N\n",
	"port           - select a different port number used for sequence and range pages.\n",
	"process        - Enable or disable input packet processing on a port\n",
	"garp           - Enable or disable GARP packet processing on a port\n",
	"blink          - Blink an led on a port\n",
	"help           - Return the help text\n",
	"Lua.help       - Lua command help text\n",
	"\n",
	"isSending      - Test to see if a port is sending packets\n",
	"linkState      - Return the current link state of a port\n",
	"\n",
	"portSizes      - Return the stats on the size of packet for a port.\n",
	"pktStats       - return the current packet stats on a port\n",
	"portStats      - return the current port stats\n",
	"portCount      - Number of port being used\n",
	"totalPorts     - Total number of ports seen by DPDK\n",
	"\n",
	"compile        - Convert a structure into a frame to be sent\n",
	"decompile      - decompile a frame into Ethernet, IP, TCP, UDP or other protocols\n",
	"sendPkt        - Not working.\n",
	"recvPkt        - Not working.\n",
	"\n",
	"run            - Load a Lua string or command file and execute it.\n",
	"continue       - Display a message and wait for keyboard key and return\n",
	"input          - Wait for a keyboard input line and return line.\n",
	"\n",
	"Pktgen.info.XXXX values below\n",
	"\n",
	"Lua_Version    - Lua version string\n",
	"Lua_Release    - Lua release string\n",
	"Lua_Copyright  - Lua Copyright string\n",
	"Lua_Authors    - Lua Authors string\n",
	"\n",
  	"Pktgen_Version - Pktgen version string\n",
  	"Pktgen_Copyright - Pktgen copyright string\n",
  	"Pktgen_Authors - Pktgen Authors string\n",
  	"DPDK_Version   - DPDK version string\n",
  	"DPDK_Copyright - DPDK copyright string",
	"\n",
  	"startSeqIdx    - Start of Sequence index value\n",
  	"singlePktIdx   - Single packet index value\n",
  	"rangePktIdx    - Start of the Range packet index\n",
  	"pingPktIdx     - Ping packet index value\n",
  	"startExtraTxIdx- Start of Extra TX index value",
	"\n",
  	"numSeqPkts     - Max number of sequence packets\n",
  	"numExtraTxPkts - Number of Extra TX packet buffers\n",
  	"numTotalPkts   - Number of total packet buffers\n",
	"\n",
  	"minPktSize     - Min packet size plus FCS\n",
  	"maxPktSize     - Max packet size plus FCS\n",
  	"minVlanID      - Min VLAN ID value\n",
  	"maxVlanID      - Max VLAN ID value\n",
  	"vlanTagSize    - VLAN Tag size\n",
  	"mbufCacheSize  - mbuf cache size value]n",
	"\n",
  	"defaultPktBurst- Default burst packet count\n",
  	"defaultBuffSize- Default buffer size value\n",
  	"maxMbufsPerPort- Max mbufs per port value\n",
  	"maxPrimeCount  - Max prime count\n",
  	"portCount      - Number of ports used\n",
  	"totalPorts     - Total ports found\n",

	NULL
};

/**************************************************************************//**
*
* pktgen_lua_help - Display the current Lua help information.
*
* DESCRIPTION
* Display the current Lua help information.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static int pktgen_lua_help (lua_State *L) {
	int		i;

	lua_concat(L, 0);
	for(i=1; lua_help_info[i] != NULL; i++ ) {
		lua_pushstring(L, lua_help_info[i]);
		lua_concat(L, 2);
	}

	return 1;
}

static const luaL_Reg pktgenlib[] = {
  {"set",			pktgen_set},			// Set a number of options

  {"start",			pktgen_start},			// Start a set of ports sending packets
  {"stop",			pktgen_stop},			// Stop a set of ports sending packets

  // Set the single packet value on main screen
  {"set_mac",   	pktgen_set_mac},		// Set the MAC address for a port
  {"set_ipaddr",	pktgen_set_ip_addr},	// Set the src and dst IP addresses
  {"mac_from_arp",  pktgen_macFromArp},		// Configure MAC from ARP packet
  {"set_proto", 	pktgen_prototype},		// Set the prototype value
  {"set_type",  	pktgen_set_type},		// Set the type value

  {"ping4",     	pktgen_send_ping4},		// Send a Ping IPv4 packet (ICMP echo)
#ifdef INCLUDE_PING6
  {"ping6",     	pktgen_send_ping6},		// Send a Ping IPv6 packet (ICMP echo)
#endif

  {"pcap",			pktgen_pcap},			// Load a PCAP file
  {"icmp_echo", 	pktgen_icmp},			// Enable/disable ICMP echo support
  {"send_arp",  	pktgen_sendARP},		// Send a ARP request or GRATUITOUS_ARP

  // Setup sequence packets
  {"seq",     		pktgen_seq},			// Set the sequence data for a port
  {"seqTable", 		pktgen_seqTable},		// Set the sequence data for a port using tables

  {"screen",		pktgen_scrn},			// Turn off and on the screen updates
  {"prime",			pktgen_prime},			// Send a small number of packets to prime the routes
  {"delay",			pktgen_delay},			// Delay a given number of milliseconds
  {"pause",			pktgen_pause},			// Delay for a given number of milliseconds and display message
  {"sleep",			pktgen_sleep},			// Delay a given number of seconds
  {"load",			pktgen_load},			// load and run a command file or Lua file.
  {"save",			pktgen_config_save},	// Save the configuration of Pktgen to a file.
  {"clear",			pktgen_clear},			// Clear stats for the given ports
  {"clr",			pktgen_clear_all},		// Clear all stats on all ports
  {"cls",			pktgen_cls_screen},		// Redraw the screen
  {"update",		pktgen_update_screen},	// Update the screen information
  {"reset",			pktgen_reset_config},	// Reset the configuration to all ports
  {"portCount",		pktgen_portCount},		// Used port count value
  {"totalPorts",    pktgen_totalPorts},     // Total ports seen by DPDK

  {"vlan",			pktgen_vlan},			// Enable or disable VLAN header
  {"vlanid",		pktgen_vlanid},			// Set the vlan ID for a given portlist

  {"mpls",			pktgen_mpls},			// Enable or disable MPLS header
  {"qinq",			pktgen_qinq},			// Enable or disable Q-in-Q header
  {"gre",           pktgen_gre},			// Enable or disable GRE with IPv4 payload
  {"gre_eth",		pktgen_gre_eth},		// Enable or disable GRE with Ethernet payload

  // Range commands
  {"dst_mac",		pktgen_dst_mac},		// Set the destination MAC address for a port
  {"src_mac",		pktgen_src_mac},		// Set the src MAC address for a port
  {"src_ip",		pktgen_src_ip},			// Set the source IP address and netmask value
  {"dst_ip",		pktgen_dst_ip},			// Set the destination IP address
  {"src_port",		pktgen_src_port},		// Set the IP source port number
  {"dst_port",		pktgen_dst_port},		// Set the IP destination port number
  {"vlan_id",		pktgen_vlan_id},		// Set the vlan id value
  {"mpls_entry",	pktgen_mpls_entry},		// Set the MPLS entry value
  {"qinqids",		pktgen_qinqids},		// Set the Q-in-Q ID values
  {"gre_key",		pktgen_gre_key},		// Set the GRE key
  {"pkt_size",		pktgen_pkt_size},		// the packet size for a range port
  {"range",			pktgen_range},			// Enable or disable sending range data on a port.

  {"ports_per_page",pktgen_ports_per_page},	// Set the number of ports per page
  {"page",			pktgen_page},			// Select a page to display, seq, range, pcap and a number from 0-N
  {"port",			pktgen_port},			// select a different port number used for sequence and range pages.
  {"process",		pktgen_process},		// Enable or disable input packet processing on a port
  {"garp",			pktgen_garp},			// Enable or disable GARP packet processing on a port
  {"blink",			pktgen_blink},			// Blink an led on a port
  {"help",			pktgen_help},			// Return the help text
  {"Lua.help",		pktgen_lua_help},		// Lua command help text
  
  {"isSending",		pktgen_isSending},		// Test to see if a port is sending packets
  {"linkState",		pktgen_linkState},		// Return the current link state of a port
  
  {"portSizes",		pktgen_portSizes},		// Return the stats on the size of packet for a port.
  {"pktStats",		pktgen_pktStats},		// return the current packet stats on a port
  {"portStats", 	pktgen_portStats},		// return the current port stats

  {"compile",		pktgen_compile},		// Convert a structure into a frame to be sent
  {"decompile",		pktgen_decompile},		// decompile a frame into Ethernet, IP, TCP, UDP or other protocols
  {"sendPkt",		pktgen_sendPkt},		// Not working.
  {"recvPkt",		pktgen_recvPkt},		// Not working.

  {"run",			pktgen_run},			// Load a Lua string or command file and execute it.
  {"continue",		pktgen_continue},		// Display a message and wait for keyboard key and return
  {"input",			pktgen_input},			// Wait for a keyboard input line and return line.

  {NULL, NULL}
};

/* }====================================================== */


/**************************************************************************//**
*
* luaopen_pktgen - Initialize the Lua support for pktgen.
*
* DESCRIPTION
* Initialize the Lua library for Pktgen.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

LUALIB_API int luaopen_pktgen (lua_State *L) {

	luaL_newlib(L, pktgenlib);

	lua_pushstring(L, "info");			// Push the table index name
	lua_newtable(L);					// Create the structure table for information

	setf_string(L, "Lua_Version", (char *)LUA_VERSION);
	setf_string(L, "Lua_Release", (char *)LUA_RELEASE);
	setf_string(L, "Lua_Copyright", (char *)LUA_COPYRIGHT);
	setf_string(L, "Lua_Authors", (char *)LUA_AUTHORS);

  	setf_string(L, "Pktgen_Version", (char *)PKTGEN_VERSION);
  	setf_string(L, "Pktgen_Copyright", (char *)wr_copyright_msg());
  	setf_string(L, "Pktgen_Authors", (char *)"Keith Wiles @ Wind River Systems");
  	setf_string(L, "DPDK_Version",
  			(char *)("DPDK-"RTE_STR(RTE_VER_MAJOR)"."RTE_STR(RTE_VER_MINOR)"."RTE_STR(RTE_VER_PATCH_LEVEL)));
  	setf_string(L, "DPDK_Copyright", (char *)wr_powered_by());

  	setf_integer(L, "startSeqIdx", FIRST_SEQ_PKT);
  	setf_integer(L, "singlePktIdx", SINGLE_PKT);
  	setf_integer(L, "rangePktIdx", RANGE_PKT);
  	setf_integer(L, "pingPktIdx", PING_PKT);
  	setf_integer(L, "startExtraTxIdx", EXTRA_TX_PKT);

  	setf_integer(L, "numSeqPkts", NUM_SEQ_PKTS);
  	setf_integer(L, "numExtraTxPkts", NUM_EXTRA_TX_PKTS);
  	setf_integer(L, "numTotalPkts", NUM_TOTAL_PKTS);

  	setf_integer(L, "minPktSize", MIN_PKT_SIZE + FCS_SIZE);
  	setf_integer(L, "maxPktSize", MAX_PKT_SIZE + FCS_SIZE);
  	setf_integer(L, "minVlanID", MIN_VLAN_ID);
  	setf_integer(L, "maxVlanID", MAX_VLAN_ID);
  	setf_integer(L, "vlanTagSize", VLAN_TAG_SIZE);
  	setf_integer(L, "mbufCacheSize", MBUF_CACHE_SIZE);

  	setf_integer(L, "defaultPktBurst", DEFAULT_PKT_BURST);
  	setf_integer(L, "defaultBuffSize", DEFAULT_BUFF_SIZE);
  	setf_integer(L, "maxMbufsPerPort", MAX_MBUFS_PER_PORT);
  	setf_integer(L, "maxPrimeCount", MAX_PRIME_COUNT);

	// Now set the table for the info values.
	lua_rawset(L, -3);

	return 1;
}

/**************************************************************************//**
*
* _lua_openlib - Open the Pktgen Lua library.
*
* DESCRIPTION
* Open and initialize the Pktgen Lua Library.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
_lua_openlib(lua_State * L) {

	lua_gc(L, LUA_GCSTOP, 0);	/* stop collector during initialization */

	luaL_openlibs(L); /* open libraries */

	luaL_requiref(L, LUA_PKTGENLIBNAME, luaopen_pktgen, 1);
	lua_pop(L, 1);

	lua_gc(L, LUA_GCRESTART, 0);

    assert( dolibrary(L, PKTGEN_SHORTCUTS) == LUA_OK );
}
