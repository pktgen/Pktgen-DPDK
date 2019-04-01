 /*-
 * Copyright (c) <2011-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2011 by Keith Wiles @ intel.com */

#define lpktgenlib_c
#define LUA_LIB
#define lua_c

#include <pg_ether.h>
#include <pg_inet.h>
#include "lpktgenlib.h"

#include <stdint.h>
#include <netinet/in.h>

#include <rte_lua.h>
#include <rte_lua_stdio.h>
#include <rte_lua_utils.h>

#include "pktgen-cmds.h"
#include <cli.h>
#include <luaconf.h>
#include <lualib.h>

#include <rte_net.h>
#include <rte_lua.h>
#include <rte_lua_stdio.h>
#include <rte_strings.h>

#if RTE_VERSION >= RTE_VERSION_NUM(17, 11, 0, 0)
#include <rte_bus_pci.h>
#endif

#include <cli_help.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

extern pktgen_t pktgen;

void pktgen_quit(void);

static int
pktgen_exit(lua_State *L __rte_unused)
{
	pktgen_quit();
	return 0;
}

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
setf_integer(lua_State *L, const char *name, lua_Integer value)
{
	lua_pushinteger(L, value);
	lua_setfield(L, -2, name);
}

#if 0 /* not used */
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
setf_function(lua_State *L, const char *name, lua_CFunction fn)
{
	lua_pushcclosure(L, fn, 0);
	lua_setfield(L, -2, name);
}
#endif

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
setf_string(lua_State *L, const char *name, const char *value)
{
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
setf_stringLen(lua_State *L, const char *name, char *value, int len)
{
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
setf_udata(lua_State *L, const char *name, void *value)
{
	lua_pushlightuserdata(L, value);
	lua_setfield(L, -2, name);
}

#endif

static __inline__ void
getf_etheraddr(lua_State *L, const char *field, struct ether_addr *value)
{
	lua_getfield(L, 3, field);
	if (lua_isstring(L, -1) )
		rte_ether_aton(luaL_checkstring(L, -1), value);
	lua_pop(L, 1);
}

static __inline__ void
getf_ipaddr(lua_State *L, const char *field, void *value, uint32_t flags)
{
	lua_getfield(L, 3, field);
	if (lua_isstring(L, -1) ) {
		rte_atoip((char *)(uintptr_t)luaL_checkstring(L, -1), flags, value,
				     sizeof(struct pg_ipaddr));
	}
	lua_pop(L, 1);
}

static __inline__ uint32_t
getf_integer(lua_State *L, const char *field)
{
	uint32_t value = 0;

	lua_getfield(L, 3, field);
	if (lua_isinteger(L, -1))
		value   = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	return value;
}

static __inline__ char *
getf_string(lua_State *L, const char *field)
{
	char      *value = NULL;

	lua_getfield(L, 3, field);
	if (lua_isstring(L, -1) )
		value   = (char *)luaL_checkstring(L, -1);
	lua_pop(L, 1);

	return value;
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

static int
pktgen_set(lua_State *L) {
	uint32_t value;
	portlist_t portlist;
	char *what;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "set, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	what = (char *)luaL_checkstring(L, 2);
	value = luaL_checknumber(L, 3);

	foreach_port(portlist, _do(
		if (!strcasecmp(what, "count"))
			single_set_tx_count(info, value);
		else if (!strcasecmp(what, "size"))
			single_set_pkt_size(info, value);
		else if (!strcasecmp(what, "rate"))
			single_set_tx_rate(info, luaL_checkstring(L, 3));
		else if (!strcasecmp(what, "burst"))
			single_set_tx_burst(info, value);
		else if (!strcasecmp(what, "cycles"))
			debug_set_tx_cycles(info, value);
		else if (!strcasecmp(what, "sport"))
			single_set_port_value(info, what[0], value);
		else if (!strcasecmp(what, "dport"))
			single_set_port_value(info, what[0], value);
		else if (!strcasecmp(what, "seq_cnt"))
			pktgen_set_port_seqCnt(info, value);
		else if (!strcasecmp(what, "seqCnt"))
			pktgen_set_port_seqCnt(info, value);
		else if (!strcasecmp(what, "prime"))
			pktgen_set_port_prime(info, value);
		else if (!strcasecmp(what, "dump"))
			debug_set_port_dump(info, value);
		else
			return luaL_error(L,
					"set does not support %s",
					what);
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

static int
set_seq(lua_State *L, uint32_t seqnum)
{
	portlist_t portlist;
	uint32_t pktsize, sport, dport, gtpu_teid;
	uint16_t vlanid;
	uint8_t cos, tos;
	struct ether_addr daddr;
	struct ether_addr saddr;
	struct pg_ipaddr ip_daddr;
	struct pg_ipaddr ip_saddr;
	char *proto, *ip;

	rte_parse_portlist(luaL_checkstring(L, 2), &portlist);
	rte_ether_aton(luaL_checkstring(L, 3), &daddr);
	rte_ether_aton(luaL_checkstring(L, 4), &saddr);

	sport   = luaL_checkinteger(L, 7);
	dport   = luaL_checkinteger(L, 8);

	/* Determine if we are IPv4 or IPv6 packets */
	ip      = (char *)luaL_checkstring(L, 9);
	if (ip[3] == '6') {
		rte_atoip(luaL_checkstring(L, 5), PG_IPADDR_V6,
				  &ip_daddr, sizeof(struct pg_ipaddr));
		rte_atoip(luaL_checkstring(L, 6),
				  PG_IPADDR_NETWORK | PG_IPADDR_V6,
				  &ip_saddr, sizeof(struct pg_ipaddr));
	} else {
		rte_atoip(luaL_checkstring(L, 5), PG_IPADDR_V4,
				  &ip_daddr, sizeof(struct pg_ipaddr));
		rte_atoip(luaL_checkstring(L, 6),
				  PG_IPADDR_NETWORK | PG_IPADDR_V4,
				  &ip_saddr, sizeof(struct pg_ipaddr));
	}
	proto   = (char *)luaL_checkstring(L, 10);
	vlanid  = luaL_checkinteger(L, 11);
	pktsize = luaL_checkinteger(L, 12);
	if (lua_gettop(L) == 13)
		gtpu_teid = luaL_checkinteger(L, 13);
	else
		gtpu_teid = 0;

	cos = 0;
	tos = 0;
	if (lua_gettop(L) > 13) {
		cos  = luaL_checkinteger(L, 14);
		tos  = luaL_checkinteger(L, 15);
	}

	if ( (proto[0] == 'i') && (ip[3] == '6') ) {
		lua_putstring("Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	foreach_port(portlist,
	             pktgen_set_seq(info, seqnum, &daddr, &saddr, &ip_daddr,
	                            &ip_saddr,
	                            sport, dport, ip[3], proto[0], vlanid,
	                            pktsize, gtpu_teid);
			pktgen_set_cos_tos_seq(info, seqnum, cos, tos));

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

static int
pktgen_seq(lua_State *L) {
	uint32_t seqnum;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "seq, wrong number of arguments");
	case 12:
	case 13:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if (seqnum >= NUM_SEQ_PKTS)
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

static int
set_seqTable(lua_State *L, uint32_t seqnum)
{
	portlist_t portlist;
	uint32_t pktSize, sport, dport, gtpu_teid;
	uint16_t vlanid;
	uint8_t cos, tos;
	struct ether_addr daddr;
	struct ether_addr saddr;
	struct pg_ipaddr ip_daddr;
	struct pg_ipaddr ip_saddr;
	char *ipProto, *ethType;

	rte_parse_portlist(luaL_checkstring(L, 2), &portlist);

	getf_etheraddr(L, "eth_dst_addr", &daddr);
	getf_etheraddr(L, "eth_src_addr", &saddr);
	getf_ipaddr(L, "ip_dst_addr", &ip_daddr, PG_IPADDR_V4);
	getf_ipaddr(L, "ip_src_addr", &ip_saddr,
	            PG_IPADDR_NETWORK | PG_IPADDR_V4);

	sport       = getf_integer(L, "sport");
	dport       = getf_integer(L, "dport");
	ipProto     = getf_string(L, "ipProto");
	ethType     = getf_string(L, "ethType");
	vlanid      = getf_integer(L, "vlanid");
	pktSize     = getf_integer(L, "pktSize");
	cos         = getf_integer(L, "cos");
	tos         = getf_integer(L, "tos");

	gtpu_teid = getf_integer(L, "gtpu_teid");

	if ( (ipProto[0] == 'i') && (ethType[3] == '6') ) {
		lua_putstring("Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	foreach_port(portlist,
	             pktgen_set_seq(info, seqnum, &daddr, &saddr, &ip_daddr,
	                            &ip_saddr,
	                            sport, dport, ethType[3], ipProto[0],
	                            vlanid, pktSize, gtpu_teid);
			pktgen_set_cos_tos_seq(info, seqnum, cos, tos) );

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

static int
pktgen_seqTable(lua_State *L) {
	uint32_t seqnum;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "seqTable, wrong number of arguments");
	case 3:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if (seqnum >= NUM_SEQ_PKTS)
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

static int
pktgen_ports_per_page(lua_State *L) {
	switch (lua_gettop(L) ) {
	default: return luaL_error(L,
				   "ports_per_page, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_set_page_size(luaL_checkinteger(L, 1));
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

static int
pktgen_icmp(lua_State *L) {
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "icmp, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	foreach_port(portlist,
		     enable_icmp_echo(info,
					  estate((char *)luaL_checkstring(L,
									      2))) );
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

static int
pktgen_sendARP(lua_State *L) {
	portlist_t portlist;
	char *what;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "sendARP, wrong number of arguments");
	case 2:
		break;
	}
	what = (char *)luaL_checkstring(L, 2);
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	foreach_port(portlist,
		     pktgen_send_arp_requests(info,
					      (what[0] == 'g') ? GRATUITOUS_ARP : 0) );
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

static int
pktgen_set_mac(lua_State *L)
{
	portlist_t portlist;
	struct ether_addr mac;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "set_mac, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	rte_ether_aton(luaL_checkstring(L, 2), &mac);

	foreach_port(portlist,
	             single_set_dst_mac(info, &mac) );

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

static int
pktgen_macFromArp(lua_State *L)
{
	char *state;
	uint32_t onOff;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L,
				   "mac_from_arp, wrong number of arguments");
	case 1:
		break;
	}
	state = (char *)luaL_checkstring(L, 1);

	onOff = estate(state);

	enable_mac_from_arp(onOff);

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

static int
pktgen_prototype(lua_State *L)
{
	char *type;
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "set_proto, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	type = (char *)luaL_checkstring(L, 2);

	foreach_port(portlist,
		     single_set_proto(info, type) );

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

static int
pktgen_set_ip_addr(lua_State *L) {
	portlist_t portlist;
	struct pg_ipaddr ipaddr;
	int flags;
	char      *type;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "set_ipaddr, wrong number of arguments");
	case 3:
		break;
	}
	type = (char *)luaL_checkstring(L, 2);
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	flags = PG_IPADDR_V4;
	if (type[0] == 's')
		flags |= PG_IPADDR_NETWORK;
	rte_atoip(luaL_checkstring(L, 3), flags,
			  &ipaddr, sizeof(struct pg_ipaddr));

	foreach_port(portlist,
	             single_set_ipaddr(info, type[0], &ipaddr) );

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

static int
pktgen_set_type(lua_State *L)
{
	char *type;
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "set_type, wrong number of arguments");
	case 2:
		break;
	}
	type = (char *)luaL_checkstring(L, 2);
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     single_set_pkt_type(info, type) );

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

static int
pktgen_send_ping4(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "ping4, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
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

static int
pktgen_send_ping6(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "ping6, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
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

static int
pktgen_pcap(lua_State *L)
{
	portlist_t portlist;
	char *what;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "pcap, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	what = (char *)luaL_checkstring(L, 2);

	foreach_port(portlist,
		     enable_pcap(info, estate(what)) );

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

static int
pktgen_start(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "start, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
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

static int
pktgen_stop(lua_State *L) {
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "stop, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
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

static int
pktgen_scrn(lua_State *L)
{
	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "screen, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_screen(estate((const char *)luaL_checkstring(L, 1)));
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

static int
pktgen_prime(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "prime, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	             pktgen_prime_ports(info) );
	return 0;
}

static __inline__ void
__delay(int32_t t)
{
	int32_t n;

	while (t > 0) {
		cli_use_timers();
		n = (t > 10) ? 10 : t;
		rte_delay_us_sleep(n * 1000);
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

static int
pktgen_delay(lua_State *L)
{
	switch (lua_gettop(L) ) {
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

static int
pktgen_pause(lua_State *L)
{
	char *str;
	int v;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "pause, wrong number of arguments");
	case 2:
		break;
	}
	str = (char *)luaL_checkstring(L, 1);
	if (strlen(str) > 0)
		lua_putstring(str);

	v = luaL_checkinteger(L, 2);
	__delay(v);

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

static int
pktgen_continue(lua_State *L)
{
	char buf[4], *str;
	int n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "continue, wrong number of arguments");
	case 1:
		break;
	}
	str = (char *)luaL_checkstring(L, 1);

	if (strlen(str) > 0)
		lua_putstring(str);

	buf[0] = '\0';
	n = fread(buf, 1, 1, (FILE *)lua_get_stdin(pktgen.ld));
	if (n > 0)
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

static int
pktgen_input(lua_State *L)
{
	char buf[256], c, *str;
	uint32_t n, idx;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "input, wrong number of arguments");
	case 1:
		break;
	}
	str = (char *)luaL_checkstring(L, 1);

	if (strlen(str) > 0)
		lua_putstring(str);

	idx = 0;
	buf[idx] = '\0';
	while (idx < (sizeof(buf) - 2) ) {
		n = fread(&c, 1, 1, (FILE *)lua_get_stdin(pktgen.ld));
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

static int
pktgen_sleep(lua_State *L)
{
	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "sleep, wrong number of arguments");
	case 1:
		break;
	}
	__delay(luaL_checkinteger(L, 1) * 1000);
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

static int
pktgen_load(lua_State *L)
{
	char *path;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "load, wrong number of arguments");
	case 1:
		break;
	}
	path = (char *)luaL_checkstring(L, 1);

	if (cli_execute_cmdfile(path) )
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

static int
pktgen_config_save(lua_State *L)
{
	char *path;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "save, wrong number of arguments");
	case 1:
		break;
	}
	path = (char *)luaL_checkstring(L, 1);

	if (pktgen_save(path) )
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

static int
pktgen_clear(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "clear, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
         pktgen_clear_stats(info) );
	pktgen_update_display();

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

static int
pktgen_clear_all(lua_State *L __rte_unused)
{
	forall_ports(pktgen_clear_stats(info) );
	pktgen_update_display();

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

static int
pktgen_cls_screen(lua_State *L __rte_unused)
{
	pktgen_clear_display();

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

static int
pktgen_update_screen(lua_State *L __rte_unused)
{
	pktgen_update_display();

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

static int
pktgen_reset_config(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "reset, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	             pktgen_reset(info) );

	return 0;
}

/**************************************************************************//**
 *
 * pktgen_restart - Reset ports
 *
 * DESCRIPTION
 * Reset ports
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_restart(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "reset, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	             pktgen_port_restart(info) );

	return 0;
}

/**************************************************************************//**
 *
 * range_dst_mac - Set a destination MAC address
 *
 * DESCRIPTION
 * Set a destination MAC address.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_dst_mac(lua_State *L)
{
	portlist_t portlist;
	struct ether_addr mac;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "dst_mac, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	rte_ether_aton(luaL_checkstring(L, 3), &mac);

	foreach_port(portlist,
	             range_set_dest_mac(info, luaL_checkstring(L, 2), &mac) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_src_mac - Set the source MAC address in the range data.
 *
 * DESCRIPTION
 * Set the source MAC address for a given set of ports in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_src_mac(lua_State *L)
{
	portlist_t portlist;
	struct ether_addr mac;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "src_mac, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	rte_ether_aton(luaL_checkstring(L, 3), &mac);

	foreach_port(portlist,
	             range_set_src_mac(info, luaL_checkstring(L, 2), &mac) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_dst_ip - Set the IP address in the range data.
 *
 * DESCRIPTION
 * Set the IP address in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_dst_ip(lua_State *L)
{
	portlist_t portlist;
	struct pg_ipaddr ipaddr;
	char      *type;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "dst_ip, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	rte_atoip(luaL_checkstring(L, 3), PG_IPADDR_V4,
			  &ipaddr, sizeof(struct pg_ipaddr));

	type = (char *)luaL_checkstring(L, 2);
	foreach_port(portlist,
	             range_set_dst_ip(info, type, &ipaddr) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_src_ip - Set the source IP address in the range data.
 *
 * DESCRIPTION
 * Set the source IP address in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_src_ip(lua_State *L)
{
	portlist_t portlist;
	struct pg_ipaddr ipaddr;
	char      *type;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "src_ip, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	rte_atoip(luaL_checkstring(L, 3), PG_IPADDR_V4,
			  &ipaddr, sizeof(ipaddr));

	type = (char *)luaL_checkstring(L, 2);
	foreach_port(portlist,
	             range_set_src_ip(info, type, &ipaddr) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_dst_port - Set the port type in the range data.
 *
 * DESCRIPTION
 * Set the port type in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_dst_port(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "dst_port, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     range_set_dst_port(info, (char *)luaL_checkstring(L, 2),
					 luaL_checkinteger(L, 3)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_src_port - Set the source port value in the range data.
 *
 * DESCRIPTION
 * Set the source port value in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_ip_proto(lua_State *L)
{
	portlist_t portlist;
	const char *ip;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "ip_proto, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	ip = luaL_checkstring(L, 2);
	foreach_port(portlist,
		     range_set_proto(info, ip));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_src_port - Set the source port value in the range data.
 *
 * DESCRIPTION
 * Set the source port value in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_src_port(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "src_port, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     range_set_src_port(info, (char *)luaL_checkstring(L, 2),
					 luaL_checkinteger(L, 3)));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_gtpu_teid - Set the GTPU-TEID value in the range data.
 *
 * DESCRIPTION
 * Set the source port value in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_gtpu_teid(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "GTP-U TEID, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     range_set_gtpu_teid(info, (char *)luaL_checkstring(L, 2),
					  luaL_checkinteger(L, 3)));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_vlan_id - Set the VLAN id in the range data.
 *
 * DESCRIPTION
 * Set the VLAN id in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_vlan_id(lua_State *L)
{
	portlist_t portlist;
	uint32_t vlan_id;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "vlan_id, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	vlan_id = luaL_checkinteger(L, 3);

	foreach_port(portlist,
		     range_set_vlan_id(info, (char *)luaL_checkstring(L, 2),
					vlan_id) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_cos - Set the CoS in the range data.
 *
 * DESCRIPTION
 * Set the CoS in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_cos(lua_State *L)
{
	portlist_t portlist;
	uint32_t cos;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "cos, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	cos = luaL_checkinteger(L, 3);

	foreach_port(portlist,
		     range_set_cos_id(info, (char *)luaL_checkstring(L, 2),
					cos) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_tos - Set the ToS in the range data.
 *
 * DESCRIPTION
 * Set the ToS in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_tos(lua_State *L)
{
	portlist_t portlist;
	uint32_t tos;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "tos, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	tos = luaL_checkinteger(L, 3);

	foreach_port(portlist,
		     range_set_tos_id(info, (char *)luaL_checkstring(L, 2),
					tos) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * single_vlan_id - Set the VLAN id for a single port
 *
 * DESCRIPTION
 * Set the VLAN id for a single port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
single_vlan_id(lua_State *L)
{
	portlist_t portlist;
	uint32_t vlanid;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "vlanid, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	vlanid = luaL_checkinteger(L, 2);
	if ( (vlanid < MIN_VLAN_ID) || (vlanid > MAX_VLAN_ID) )
		vlanid = 1;

	foreach_port(portlist,
		     single_set_vlan_id(info, vlanid) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * single_cos - Set the 802.1p prio for a single port
 *
 * DESCRIPTION
 * Set the 802.1p cos for a single port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
single_cos(lua_State *L) {
	portlist_t portlist;
	uint8_t cos;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "cos, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	cos = luaL_checkinteger(L, 3);
	if (cos > MAX_COS)
		cos = 0;

	foreach_port(portlist,
	             single_set_cos(info, cos) );

	pktgen_update_display();
	return 0;
}


/**************************************************************************//**
 *
 * single_tos - Set the TOS for a single port
 *
 * DESCRIPTION
 * Set the TOS for a single port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
single_tos(lua_State *L) {
	portlist_t portlist;
	uint8_t tos;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "tos, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	tos = luaL_checkinteger(L, 2);

	foreach_port(portlist,
	             single_set_tos(info, tos) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * single_vxlan_id - Set the VxLAN for a single port
 *
 * DESCRIPTION
 * Set the VxLAN for a single port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
single_vxlan_id(lua_State *L) {
	portlist_t portlist;
	uint8_t flags, group_id;
	uint32_t vxlan_id;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "tos, wrong number of arguments");
	case 4:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	flags = luaL_checkinteger(L, 2);
	group_id = luaL_checkinteger(L, 3);
	vxlan_id = luaL_checkinteger(L, 4);

	foreach_port(portlist,
	             single_set_vxlan(info, flags, group_id, vxlan_id) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * single_vlan - Enable or Disable vlan header
 *
 * DESCRIPTION
 * Enable or disable insertion of VLAN header.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
single_vlan(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_vlan(info, estate(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * single_vxlan - Enable or Disable vxlan header
 *
 * DESCRIPTION
 * Enable or disable insertion of VxLAN header.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
single_vxlan(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_vxlan(info, estate(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_mpls_entry - Set the MPLS entry in the range data.
 *
 * DESCRIPTION
 * Set the VLAN id in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_mpls_entry(lua_State *L)
{
	portlist_t portlist;
	uint32_t mpls_entry;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "mpls_entry, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	mpls_entry = strtoul(luaL_checkstring(L, 2), NULL, 16);

	foreach_port(portlist,
		     range_set_mpls_entry(info, mpls_entry) );

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

static int
pktgen_mpls(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "mpls, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_mpls(info,
				     estate(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_qinqids - Set the Q-in-Q ID's in the range data.
 *
 * DESCRIPTION
 * Set the Q-in-Q ID's in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_qinqids(lua_State *L)
{
	portlist_t portlist;
	uint32_t qinq_id1, qinq_id2;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "qinqids, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	qinq_id1 = luaL_checkinteger(L, 2);
	if ( (qinq_id1 < MIN_VLAN_ID) || (qinq_id1 > MAX_VLAN_ID) )
		qinq_id1 = 1;

	qinq_id2 = luaL_checkinteger(L, 3);
	if ( (qinq_id2 < MIN_VLAN_ID) || (qinq_id2 > MAX_VLAN_ID) )
		qinq_id2 = 1;

	foreach_port(portlist,
		     single_set_qinqids(info, qinq_id1, qinq_id2) );

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

static int
pktgen_qinq(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "qinq, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_qinq(info,
				     estate(luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_gre_key - Set the GRE key in the range data.
 *
 * DESCRIPTION
 * Set the GRE key in the range data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_gre_key(lua_State *L)
{
	portlist_t portlist;
	uint32_t gre_key;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "gre_key, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	gre_key = luaL_checkinteger(L, 2);

	foreach_port(portlist,
		     range_set_gre_key(info, gre_key) );

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

static int
pktgen_gre(lua_State *L) {
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "gre, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_gre(info, estate(luaL_checkstring(L, 2))) );

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

static int
pktgen_gre_eth(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "gre_eth, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_gre_eth(info, estate(luaL_checkstring(L,
									  2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range_pkt_size - Set the port range size.
 *
 * DESCRIPTION
 * Set the port range size.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_pkt_size(lua_State *L)
{
	portlist_t portlist;
	uint32_t size;
	char      *type;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "pkt_size, wrong number of arguments");
	case 3:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	type = (char *)luaL_checkstring(L, 2);
	size = luaL_checkinteger(L, 3);

	foreach_port(portlist,
		     range_set_pkt_size(info, type, size) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * range - Enable or disable the range data sending.
 *
 * DESCRIPTION
 * Enable or disable the range data sending.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "range, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     enable_range(info, estate((const char *)luaL_checkstring(L, 2))));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_latency - Enable or disable the latency testing.
 *
 * DESCRIPTION
 * Enable or disable the latency testing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_latency(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "latency, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     enable_latency(info, estate((const char *)luaL_checkstring(L, 2))));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_jitter - Set Jitter threshold
 *
 * DESCRIPTION
 * Set Jitter threshold in micro-seconds
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_jitter(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "jitter, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     single_set_jitter(info, luaL_checkinteger(L, 2)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_pattern - Set the pattern type.
 *
 * DESCRIPTION
 * Set the pattern type.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_pattern(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "pattern, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     pattern_set_type(info,
					     (char *)luaL_checkstring(L, 2)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_pattern - Set the pattern type.
 *
 * DESCRIPTION
 * Set the pattern type.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_user_pattern(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L,
				   "user.pattern, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     pattern_set_user_pattern(info, (char *)luaL_checkstring(L, 2)) );

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

static int
pktgen_page(lua_State *L)
{
	switch (lua_gettop(L) ) {
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

static int
pktgen_port(lua_State *L)
{
	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "port, wrong number of arguments");
	case 1:
		break;
	}
	pktgen_set_port_number((uint16_t)luaL_checkinteger(L, 1));
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

static int
pktgen_process(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     enable_process(info, estate((const char *)luaL_checkstring(L, 2))));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_capture - Enable or Disable capture packet processing.
 *
 * DESCRIPTION
 * Enable or disable capture packet processing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_capture(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "capture, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     enable_capture(info, estate((const char *)luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_bonding - Enable or Disable bonding to send zero packets
 *
 * DESCRIPTION
 * Enable or disable bonding packet processing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_bonding(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "bonding, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     enable_bonding(info, estate((const char *)luaL_checkstring(L, 2))) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_rxtap - Enable or Disable rxtap packet processing.
 *
 * DESCRIPTION
 * Enable or disable rxtap packet processing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_rxtap(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "rxtap, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_rx_tap(info, estate((char *)luaL_checkstring(L, 2))));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * pktgen_txtap - Enable or Disable txtap packet processing.
 *
 * DESCRIPTION
 * Enable or disable txtap packet processing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_txtap(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "txtap, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
		     enable_tx_tap(info, estate((char *)luaL_checkstring(L, 2))));

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

static int
pktgen_garp(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "process, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     enable_garp(info, estate((const char *)luaL_checkstring(L, 2))));

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

static int
pktgen_blink(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "blink, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	foreach_port(portlist,
	     debug_blink(info, estate((const char *)luaL_checkstring(L, 2))));

	if (pktgen.blinklist)
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

static void
isSending(lua_State *L, port_info_t *info)
{
	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_pushstring(L, pktgen_port_transmitting(info->pid) ? "y" : "n");

	/* Now set the table as an array with pid as the index. */
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

static int
pktgen_isSending(lua_State *L)
{
	portlist_t portlist;
	uint32_t n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "isSending, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
		     _do(isSending(L, info); n++) );

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

static void
link_state(lua_State *L, port_info_t *info)
{
	char buff[32];

	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_pushstring(L, pktgen_link_state(info->pid, buff, sizeof(buff)));

	/* Now set the table as an array with pid as the index. */
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

static int
pktgen_linkState(lua_State *L)
{
	portlist_t portlist;
	uint32_t n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "linkState, wrong number of arguments");
	case 1:
		break;
	}
	portlist = 0;
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
	             _do(link_state(L, info); n++) );

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

static void
port_sizes(lua_State *L, port_info_t *info)
{
	port_sizes_t sizes;

	pktgen_port_sizes(info->pid, &sizes);

	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_newtable(L);		/* Create the structure table for a packet */

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

	/* Now set the table as an array with pid as the index. */
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

static int
pktgen_portSizes(lua_State *L)
{
	portlist_t portlist;
	uint32_t n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "portSizes, wrong number of arguments");
	case 1:
		break;
	}
	portlist = 0;
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
	             _do(port_sizes(L, info); n++) );

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

static void
pkt_stats(lua_State *L, port_info_t *info)
{
	struct ether_addr ethaddr;
	char mac_buf[32];
	pkt_stats_t stats;
	uint64_t avg_lat, ticks, jitter;
	uint32_t flags = rte_atomic32_read(&info->port_flags);

	pktgen_pkt_stats(info->pid, &stats);

	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_newtable(L);		/* Create the structure table for a packet */

	setf_integer(L, "arp_pkts", stats.arp_pkts);
	setf_integer(L, "echo_pkts", stats.echo_pkts);
	setf_integer(L, "ip_pkts", stats.ip_pkts);
	setf_integer(L, "ipv6_pkts", stats.ipv6_pkts);
	setf_integer(L, "vlan_pkts", stats.vlan_pkts);
	setf_integer(L, "dropped_pkts", stats.dropped_pkts);
	setf_integer(L, "unknown_pkts", stats.unknown_pkts);
	setf_integer(L, "tx_failed", stats.tx_failed);

	rte_eth_macaddr_get(info->pid, &ethaddr);

	ether_format_addr(mac_buf, sizeof(mac_buf), &ethaddr);
	setf_string(L, "mac_addr", mac_buf);

	avg_lat = 0;
	jitter = 0;
	if (flags & SEND_LATENCY_PKTS) {
		ticks = rte_get_timer_hz() / 1000000;
		if (ticks == 0)
			printf("Ticks = %lu\n", ticks);
		else if (info->latency_nb_pkts > 0) {
			avg_lat = (info->avg_latency / info->latency_nb_pkts) / ticks;
			if (avg_lat > info->max_latency)
				info->max_latency = avg_lat;
			if (info->min_latency == 0)
				info->min_latency = avg_lat;
			else if (avg_lat < info->min_latency)
				info->min_latency = avg_lat;
			jitter = (info->jitter_count * 100) / info->latency_nb_pkts;
			info->latency_nb_pkts = 0;
			info->avg_latency     = 0;
			info->jitter_count    = 0;
		} else {
			printf("Latency pkt count = %d\n", info->latency_nb_pkts);
		}

		setf_integer(L, "avg_latency", avg_lat);
		setf_integer(L, "max_latency", info->max_latency);
		setf_integer(L, "min_latency", info->min_latency);
		setf_integer(L, "jitter_count", jitter);
	}

	/* Now set the table as an array with pid as the index. */
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

static int
pktgen_pktStats(lua_State *L)
{
	portlist_t portlist;
	uint32_t n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "pktStats, wrong number of arguments");
	case 1:
		break;
	}
	portlist = 0;
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
	             _do(pkt_stats(L, info); n++) );

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

static void
port_stats(lua_State *L, port_info_t *info, char *type)
{
	eth_stats_t stats;

	pktgen_port_stats(info->pid, type, &stats);

	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_newtable(L);		/* Create the structure table for a packet */

	setf_integer(L, "ipackets", stats.ipackets);
	setf_integer(L, "opackets", stats.opackets);
	setf_integer(L, "ibytes", stats.ibytes);
	setf_integer(L, "obytes", stats.obytes);
	setf_integer(L, "ierrors", stats.ierrors);
	setf_integer(L, "oerrors", stats.oerrors);
	setf_integer(L, "rx_nombuf", stats.rx_nombuf);
	setf_integer(L, "imissed", stats.imissed);

	if (strcmp(type, "rate") == 0) {
		setf_integer(L, "pkts_rx", stats.ipackets);
		setf_integer(L, "pkts_tx", stats.opackets);
		setf_integer(L, "mbits_rx", iBitsTotal(stats) / Million);
		setf_integer(L, "mbits_tx", oBitsTotal(stats) / Million);
	}

	/* Now set the table as an array with pid as the index. */
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

static int
pktgen_portStats(lua_State *L)
{
	portlist_t portlist;
	uint32_t n;
	char *type;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "portStats, wrong number of arguments");
	case 2:
		break;
	}

	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	type = (char *)luaL_checkstring(L, 2);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
	             _do(port_stats(L, info, type); n++) );

	setf_integer(L, "n", n);

	return 1;
}

/**************************************************************************//**
 *
 * port_info - Return the other port stats for a given ports.
 *
 * DESCRIPTION
 * Return the other port stats for a given ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static void
port_info(lua_State *L, port_info_t *info)
{
	struct rte_eth_dev_info dev = { 0 };
	eth_stats_t stats;
	pkt_seq_t *pkt;
	char buff[32];

	pkt = &info->seq_pkt[SINGLE_PKT];

	pktgen_port_stats(info->pid, "port", &stats);

	/*------------------------------------*/
	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_newtable(L);		/* Create the structure table for a packet */

	/*------------------------------------*/
	lua_pushstring(L, "stats");
	lua_newtable(L);
	setf_integer(L, "ipackets", stats.ipackets);
	setf_integer(L, "opackets", stats.opackets);
	setf_integer(L, "ibytes", stats.ibytes);
	setf_integer(L, "obytes", stats.obytes);
	setf_integer(L, "ierrors", stats.ierrors);
	setf_integer(L, "oerrors", stats.oerrors);
	setf_integer(L, "rx_nombuf", stats.rx_nombuf);
	setf_integer(L, "imissed", stats.imissed);
	lua_rawset(L, -3);

	/*------------------------------------*/
	lua_pushstring(L, "totals");
	lua_newtable(L);
	setf_integer(L, "pkts_rx", stats.ipackets);
	setf_integer(L, "pkts_tx", stats.opackets);
	setf_integer(L, "mbits_rx", iBitsTotal(stats) / Million);
	setf_integer(L, "mbits_tx", oBitsTotal(stats) / Million);
	lua_rawset(L, -3);

	/*------------------------------------*/
	lua_pushstring(L, "size_cnts");
	lua_newtable(L);
	setf_integer(L, "broadcast", info->sizes.broadcast);
	setf_integer(L, "multicast", info->sizes.multicast);
	setf_integer(L, "_64", info->sizes._64);
	setf_integer(L, "_65_127", info->sizes._65_127);
	setf_integer(L, "_128_255", info->sizes._128_255);
	setf_integer(L, "_256_511", info->sizes._256_511);
	setf_integer(L, "_512_1023", info->sizes._512_1023);
	setf_integer(L, "_1024_1518", info->sizes._1024_1518);
	setf_integer(L, "jumbo", info->sizes.jumbo);
	setf_integer(L, "runts", info->sizes.runt);

	setf_integer(L, "arp_pkts", info->stats.arp_pkts);
	setf_integer(L, "echo_pkts", info->stats.echo_pkts);

	setf_integer(L, "ierrors", info->prev_stats.ierrors);
	setf_integer(L, "oerrors", info->prev_stats.oerrors);

	setf_integer(L, "tx_failed", info->stats.tx_failed);
	setf_integer(L, "imissed", info->stats.imissed);

#if RTE_VERSION < RTE_VERSION_NUM(2, 2, 0, 0)
	setf_integer(L, "ibadcrc", info->stats.ibadcrc);
	setf_integer(L, "ibadlen", info->stats.ibadlen);
#endif
#if RTE_VERSION < RTE_VERSION_NUM(16, 4, 0, 0)
	setf_integer(L, "ibadcrc", info->stats.imcasts);
#endif
	setf_integer(L, "ibadcrc", info->stats.rx_nombuf);
	lua_rawset(L, -3);

	/*------------------------------------*/
	lua_pushstring(L, "total_stats");
	lua_newtable(L);
	setf_integer(L, "max_ipackets", pktgen.max_total_ipackets);
	setf_integer(L, "max_opackets", pktgen.max_total_opackets);

	setf_integer(L, "cumm_rate_ipackets", pktgen.cumm_rate_totals.ipackets);
	setf_integer(L, "cumm_rate_opackets", pktgen.cumm_rate_totals.opackets);

	setf_integer(L, "cumm_rate_itotals", iBitsTotal(pktgen.cumm_rate_totals));
	setf_integer(L, "cumm_rate_ototals", oBitsTotal(pktgen.cumm_rate_totals));
	lua_rawset(L, -3);

	/*------------------------------------*/
	lua_pushstring(L, "info");
	lua_newtable(L);
	setf_string(L, "pattern_type",
			    (info->fill_pattern_type == ABC_FILL_PATTERN) ? "abcd..." :
		        (info->fill_pattern_type == NO_FILL_PATTERN) ? "None" :
		        (info->fill_pattern_type == ZERO_FILL_PATTERN) ? "Zero" :
		        info->user_pattern);

	if (rte_atomic64_read(&info->transmit_count) == 0)
		setf_string(L, "tx_count", "Forever");
	else
		setf_integer(L, "tx_count", rte_atomic64_read(&info->transmit_count));
	setf_integer(L, "tx_rate", info->tx_rate);

	setf_integer(L, "pkt_size", pkt->pktSize + ETHER_CRC_LEN);
	setf_integer(L, "tx_burst", info->tx_burst);

	setf_string(L, "eth_type",
		(pkt->ethType == ETHER_TYPE_IPv4) ? "IPv4" :
		(pkt->ethType == ETHER_TYPE_IPv6) ? "IPv6" :
		(pkt->ethType == ETHER_TYPE_ARP) ? "ARP" : "Other");
	setf_string(L, "proto_type",
		(pkt->ipProto == PG_IPPROTO_TCP) ? "TCP" :
		(pkt->ipProto == PG_IPPROTO_ICMP) ? "ICMP" :
		(rte_atomic32_read(&info->port_flags) & SEND_VXLAN_PACKETS) ? "VXLAN" : "UDP");
	setf_integer(L, "vlanid", pkt->vlanid);

	/*------------------------------------*/
	lua_pushstring(L, "l2_l3_info");
	lua_newtable(L);
	setf_integer(L, "src_port", pkt->sport);
	setf_integer(L, "dst_port", pkt->dport);

	inet_ntop4(buff, sizeof(buff),
				htonl(pkt->ip_dst_addr.addr.ipv4.s_addr), 0xFFFFFFFF);
	setf_string(L, "dst_ip", buff);

	inet_ntop4(buff, sizeof(buff),
				htonl(pkt->ip_src_addr.addr.ipv4.s_addr), pkt->ip_mask);
	setf_string(L, "src_ip", buff);

	inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr);
	setf_string(L, "dst_mac", buff);
	inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr);
	setf_string(L, "src_mac", buff);
	lua_rawset(L, -3);

	rte_eth_dev_info_get(info->pid, &dev);
#if RTE_VERSION < RTE_VERSION_NUM(18, 4, 0, 0)
	if (dev.pci_dev)
		snprintf(buff, sizeof(buff), "%04x:%04x/%02x:%02d.%d",
					dev.pci_dev->id.vendor_id,
					dev.pci_dev->id.device_id,
					dev.pci_dev->addr.bus,
					dev.pci_dev->addr.devid,
					dev.pci_dev->addr.function);
	else
#else
	struct rte_bus *bus;
	if (dev.device)
		bus = rte_bus_find_by_device(dev.device);
	else
		bus = NULL;
	if (bus && !strcmp(bus->name, "pci")) {
		struct rte_pci_device *pci_dev = RTE_DEV_TO_PCI(dev.device);
		snprintf(buff, sizeof(buff), "%04x:%04x/%02x:%02d.%d",
					pci_dev->id.vendor_id,
					pci_dev->id.device_id,
					pci_dev->addr.bus,
					pci_dev->addr.devid,
					pci_dev->addr.function);
	} else
#endif
		snprintf(buff, sizeof(buff), "%04x:%04x/%02x:%02d.%d",
					0, 0, 0, 0, 0);
	setf_string(L, "pci_vendor", buff);

	/*------------------------------------*/
	lua_pushstring(L, "tx_debug");
	lua_newtable(L);
	setf_integer(L, "tx_pps", info->tx_pps);
	setf_integer(L, "tx_cycles", info->tx_cycles);
	lua_rawset(L, -3);

	/*------------------------------------*/
	lua_pushstring(L, "802.1p");
	lua_newtable(L);
	setf_integer(L, "cos", pkt->cos);
	setf_integer(L, "dscp", pkt->tos >> 2);
	setf_integer(L, "ipp", pkt->tos >> 5);
	lua_rawset(L, -3);

	/*------------------------------------*/
	lua_pushstring(L, "VxLAN");
	lua_newtable(L);
	setf_integer(L, "vni_flags", pkt->vni_flags);
	setf_integer(L, "group_id", pkt->group_id);
	setf_integer(L, "vlan_id", pkt->vlanid);

	pktgen_link_state(info->pid, buff, sizeof(buff));
	setf_string(L, "link_state", buff);
	lua_rawset(L, -3);
	lua_rawset(L, -3);

	/* Now set the table as an array with pid as the index. */
	lua_rawset(L, -3);
}

/**************************************************************************//**
 *
 * pktgen_portInfo - Return the other port Info for a given ports.
 *
 * DESCRIPTION
 * Return the other port Info for a given ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_portInfo(lua_State *L)
{
	portlist_t portlist;
	uint32_t n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "portInfo, wrong number of arguments");
	case 1:
		break;
	}

	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
	             _do(port_info(L, info); n++) );

	setf_integer(L, "n", n);

	return 1;
}

static void
_pktgen_push_line(void *arg, const char **h)
{
	lua_State *L = arg;
	int j;

	for(j = 0; h[j] != NULL; j++) {
		if (strcmp(h[j], CLI_HELP_PAUSE)) {
			lua_pushstring(L, h[j]);
			lua_concat(L, 2);
		}
	}
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

static int
pktgen_help(lua_State *L)
{
	lua_concat(L, 0);

	cli_help_foreach(_pktgen_push_line, L);

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

static int
pktgen_compile(lua_State *L)
{
	uint32_t seqnum;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "compile, wrong number of arguments");
	case 3:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if (seqnum >= NUM_SEQ_PKTS)
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

static void
decompile_pkt(lua_State *L, port_info_t *info, uint32_t seqnum)
{
	char buff[128];
	pkt_seq_t *p;

	p = &info->seq_pkt[seqnum];

	lua_pushinteger(L, info->pid);	/* Push the table index */
	lua_newtable(L);		/* Create the structure table for a packet */

	/* Add each member to the packet table indexed with port id. */
	setf_string(L, "eth_dst_addr",
		    inet_mtoa(buff, sizeof(buff), &p->eth_dst_addr));
	setf_string(L, "eth_src_addr",
		    inet_mtoa(buff, sizeof(buff), &p->eth_src_addr));
	if (p->ethType == ETHER_TYPE_IPv4) {
		setf_string(L, "ip_dst_addr",
			    inet_ntop4(buff, sizeof(buff),
				       htonl(p->ip_dst_addr.addr.ipv4.s_addr),
				       0xFFFFFFFF));
		setf_string(L, "ip_src_addr",
			    inet_ntop4(buff, sizeof(buff),
				       htonl(p->ip_dst_addr.addr.ipv4.s_addr),
				       p->ip_mask));
	} else {
		setf_string(L, "ip_dst_addr",
			    inet_ntop6(buff, sizeof(buff),
				       p->ip_dst_addr.addr.ipv6.s6_addr));
		setf_string(L, "ip_src_addr",
			    inet_ntop6(buff, sizeof(buff),
				       p->ip_dst_addr.addr.ipv6.s6_addr));
	}
	setf_integer(L, "dport", p->dport);
	setf_integer(L, "sport", p->sport);
	setf_integer(L, "vlanid", p->vlanid);
	setf_integer(L, "cos", p->cos);
	setf_integer(L, "tos", p->tos);
	setf_string(L,
		    "ethType",
		    (char *)(
			    (p->ethType == ETHER_TYPE_IPv4) ? "ipv4" :
			    (p->ethType == ETHER_TYPE_IPv6) ? "ipv6" :
			    (p->ethType ==
			     ETHER_TYPE_VLAN) ? "vlan" : "unknown"));
	setf_string(L, "ipProto", (char *)(
			    (p->ipProto == PG_IPPROTO_TCP) ? "tcp" :
			    (p->ipProto == PG_IPPROTO_ICMP) ? "icmp" : "udp"));

	setf_integer(L, "pktSize", p->pktSize + ETHER_CRC_LEN);
	setf_integer(L, "gtpu_teid", p->gtpu_teid);

	/* Now set the table as an array with pid as the index. */
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

static int
pktgen_decompile(lua_State *L)
{
	portlist_t portlist;
	uint32_t seqnum, n;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "decompile, wrong number of arguments");
	case 2:
		break;
	}
	seqnum = luaL_checkinteger(L, 1);
	if (seqnum >= NUM_SEQ_PKTS)
		return 0;
	rte_parse_portlist(luaL_checkstring(L, 2), &portlist);

	lua_newtable(L);

	n = 0;
	foreach_port(portlist,
	             _do(decompile_pkt(L, info, seqnum); n++) );

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

static int
pktgen_sendPkt(lua_State *L)
{
	portlist_t portlist;
	uint32_t seqnum;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "sendPkt, wrong number of arguments");
	case 2:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);

	seqnum = luaL_checkinteger(L, 2);
	if ( (seqnum >= NUM_EXTRA_TX_PKTS) || (portlist == 0) )
		return 0;

	foreach_port(portlist,
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

static int
pktgen_portCount(lua_State *L)
{
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

static int
pktgen_totalPorts(lua_State *L)
{
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

static int
pktgen_recvPkt(lua_State *L)
{
	portlist_t portlist;

	switch (lua_gettop(L) ) {
	default: return luaL_error(L, "recvPkt, wrong number of arguments");
	case 1:
		break;
	}
	rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
	if (portlist == 0)
		return 0;

	foreach_port(portlist,
	             pktgen_recv_pkt(info) );

	return 0;
}

/**************************************************************************//**
 *
 * pktgen_rnd - Setup random bit patterns
 *
 * DESCRIPTION
 * Setup the random bit pattern support.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_rnd(lua_State *L)
{
        portlist_t portlist;
        char mask[33] = { 0 };
        const char * msk;
        int i, mask_idx = 0;
        char curr_bit;

        switch (lua_gettop(L) ) {
        default: return luaL_error(L, "rnd, wrong number of arguments");
        case 4:
                break;
        }
        rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
        if (portlist == 0)
                return 0;

	msk = luaL_checkstring(L, 4);
	if (strcmp(msk, "off"))
		/* Filter invalid characters from provided mask. This way the user can
		 * more easily enter long bitmasks, using for example '_' as a separator
		 * every 8 bits. */
		for (i = 0; (mask_idx < 32) && ((curr_bit = msk[i]) != '\0'); i++)
			if ((curr_bit == '0') || (curr_bit == '1') ||
			    (curr_bit == '.') || (curr_bit == 'X') || (curr_bit == 'x'))
				mask[mask_idx++] = curr_bit;

	foreach_port(portlist,
		     enable_random(info, pktgen_set_random_bitfield(info->rnd_bitfields,
									luaL_checkinteger(L, 2),
									luaL_checkinteger(L, 3), mask) ?
										ENABLE_STATE : DISABLE_STATE));

	return 0;
}

static void
add_rnd_pattern(lua_State *L, port_info_t *info)
{
	uint32_t i, curr_bit, idx;
	char mask[36];	/* 4*8 bits, 3 delimiter spaces, \0 */
	bf_spec_t *curr_spec;
	rnd_bits_t *rnd_bits = info->rnd_bitfields;

	lua_pushinteger(L, info->pid);	/* Push the port number as the table index */
	lua_newtable(L);		/* Create the structure table for a packet */

	for (idx = 0; idx < MAX_RND_BITFIELDS; idx++) {
		curr_spec = &rnd_bits->specs[idx];

		memset(mask, 0, sizeof(mask));
		memset(mask, ' ', sizeof(mask) - 1);
		/* Compose human readable bitmask representation */
		for (i = 0; i < MAX_BITFIELD_SIZE; ++i) {
			curr_bit = (uint32_t)1 << (MAX_BITFIELD_SIZE - i - 1);

			/* + i >> 3 for space delimiter after every 8 bits.
			 * Need to check rndMask before andMask: for random bits, the
			 * andMask is also 0. */
			mask[i + (i >> 3)] =
				((ntohl(curr_spec->rndMask) & curr_bit) != 0) ? 'X' :
				((ntohl(curr_spec->andMask) & curr_bit) == 0) ? '0' :
				((ntohl(curr_spec->orMask)  & curr_bit) != 0) ? '1' : '.';
		}

		lua_pushinteger(L, idx);	/* Push the RND bit index */
		lua_newtable(L);		/* Create the structure table for a packet */
		setf_integer(L, "offset", curr_spec->offset);
		setf_string(L, "mask", mask);
		setf_string(L, "active", (rnd_bits->active_specs & (1 << idx)) ? "Yes" : "No");
		lua_rawset(L, -3);
	}

	lua_rawset(L, -3);
}

/**************************************************************************//**
 *
 * pktgen_rnd_list - Return the random bit patterns in a table
 *
 * DESCRIPTION
 * Return a table of the random bit patterns.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pktgen_rnd_list(lua_State *L) {
        portlist_t portlist;
        int n;

        switch (lua_gettop(L) ) {
        default: return luaL_error(L, "rnd_list, wrong number of arguments");
        case 1:
        case 0:
                break;
        }
        if (lua_gettop(L) == 1)
                rte_parse_portlist(luaL_checkstring(L, 1), &portlist);
        else
                portlist = -1;

	lua_newtable(L);

        n = 0;
        foreach_port(portlist,
                _do(add_rnd_pattern(L, info); n++));

	setf_integer(L, "n", n);

	return 1;
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

static int
pktgen_run(lua_State *L)
{
	switch (lua_gettop(L) ) {
	default: return luaL_error(
			       L,
			       "run( ['cmd'|'lua'], <string_or_path>), arguments wrong.");
	case 3:
		break;
	}

	/* A cmd executes a file on the disk and can be a lua Script of command line file. */
	if (strcasecmp("cmd", luaL_checkstring(L, 1)) == 0)
		cli_execute_cmdfile(luaL_checkstring(L, 2));
	else if (strcasecmp("lua", luaL_checkstring(L, 1)) == 0)/* Only a Lua script in memory. */
		lua_execute_string(pktgen.ld, (char *)luaL_checkstring(L, 2));
	else
		return luaL_error(L, "run( ['cmd'|'lua'], <string>), arguments wrong.");

	return 0;
}

static const char *lua_help_info[] = {
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
	"rnd            - Enable or disable random bit patterns for a given portlist\n",
	"rnd_list       - List of current random bit patterns\n",
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
	"rxtap          - Enable or disable RX Tap packet processing on a port\n",
	"txtap          - Enable or disable TX Tap packet processing on a port\n",
	"\n",
	"page           - Select a page to display, seq, range, pcap and a number from 0-N\n",
	"port           - select a different port number used for sequence and range pages.\n",
	"process        - Enable or disable input packet processing on a port\n",
	"capture        - Enable or disable capture packet processing on a port\n",
    "bonding        - Enable or disable bonding support for sending zero packets\n",
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
	"minCos       - Min 802.1p value\n",
	"maxCos       - Max 802.1p value\n",
	"minTOS       - Min TOS value\n",
	"maxTOS       - Max TOS value\n",
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

static int
pktgen_lua_help(lua_State *L)
{
	int i;

	lua_concat(L, 0);
	for (i = 1; lua_help_info[i] != NULL; i++) {
		lua_pushstring(L, lua_help_info[i]);
		lua_concat(L, 2);
	}

	return 1;
}

static const luaL_Reg pktgenlib_range[] = {
	/* Range commands */
	{"dst_mac",       range_dst_mac},	/* Set the destination MAC address for a port */
	{"src_mac",       range_src_mac},	/* Set the src MAC address for a port */
	{"src_ip",        range_src_ip},	/* Set the source IP address and netmask value */
	{"dst_ip",        range_dst_ip},	/* Set the destination IP address */
	{"ip_proto",      range_ip_proto},	/* Set the IP Protocol type */
	{"src_port",      range_src_port},	/* Set the IP source port number */
	{"dst_port",      range_dst_port},	/* Set the IP destination port number */
	{"vlan_id",       range_vlan_id},	/* Set the vlan id value */
	{"mpls_entry",    range_mpls_entry},	/* Set the MPLS entry value */
	{"qinqids",       range_qinqids},	/* Set the Q-in-Q ID values */
	{"gre_key",       range_gre_key},	/* Set the GRE key */
	{"pkt_size",      range_pkt_size},	/* the packet size for a range port */
	{NULL, NULL}
};

static const luaL_Reg pktgenlib[] = {
	{"quit",	  pktgen_exit},
	{"set",           pktgen_set},			/* Set a number of options */

	{"start",         pktgen_start},		/* Start a set of ports sending packets */
	{"stop",          pktgen_stop},			/* Stop a set of ports sending packets */

	/* Set the single packet value on main screen */
	{"set_mac",       pktgen_set_mac},		/* Set the MAC address for a port */
	{"set_ipaddr",    pktgen_set_ip_addr},	/* Set the src and dst IP addresses */
	{"mac_from_arp",  pktgen_macFromArp},	/* Configure MAC from ARP packet */
	{"set_proto",     pktgen_prototype},	/* Set the prototype value */
	{"set_type",      pktgen_set_type},		/* Set the type value */

	{"ping4",         pktgen_send_ping4},	/* Send a Ping IPv4 packet (ICMP echo) */
#ifdef INCLUDE_PING6
	{"ping6",         pktgen_send_ping6},	/* Send a Ping IPv6 packet (ICMP echo) */
#endif

	{"pcap",          pktgen_pcap},			/* Load a PCAP file */
	{"icmp_echo",     pktgen_icmp},			/* Enable/disable ICMP echo support */
	{"send_arp",      pktgen_sendARP},		/* Send a ARP request or GRATUITOUS_ARP */

	/* Setup sequence packets */
	{"seq",           pktgen_seq},			/* Set the sequence data for a port */
	{"seqTable",      pktgen_seqTable},		/* Set the sequence data for a port using tables */

	{"screen",        pktgen_scrn},			/* Turn off and on the screen updates */
	{"prime",         pktgen_prime},		/* Send a small number of packets to prime the routes */
	{"delay",         pktgen_delay},		/* Delay a given number of milliseconds */
	{"pause",         pktgen_pause},		/* Delay for a given number of milliseconds and display message */
	{"sleep",         pktgen_sleep},		/* Delay a given number of seconds */
	{"load",          pktgen_load},			/* load and run a command file or Lua file. */
	{"save",          pktgen_config_save},	/* Save the configuration of Pktgen to a file. */
	{"clear",         pktgen_clear},		/* Clear stats for the given ports */
	{"clr",           pktgen_clear_all},	/* Clear all stats on all ports */
	{"cls",           pktgen_cls_screen},	/* Redraw the screen */
	{"update",        pktgen_update_screen},/* Update the screen information */
	{"reset",         pktgen_reset_config},	/* Reset the configuration to all ports */
	{"port_restart",  pktgen_restart},		/* Reset ports */
	{"portCount",     pktgen_portCount},	/* Used port count value */
	{"totalPorts",    pktgen_totalPorts},	/* Total ports seen by DPDK */

	{"vlan",          single_vlan},			/* Enable or disable VLAN header */
	{"vlanid",        single_vlan_id},		/* Set the vlan ID for a given portlist */

	{"cos",           single_cos},			/* Set the prio for a given portlist */
	{"tos",           single_tos},			/* Set the tos for a given portlist */
	{"vxlan",         single_vxlan},		/* Enable or disable VxLAN */
	{"vxlan_id",      single_vxlan_id},		/* Set the VxLAN values */


	{"mpls",          pktgen_mpls},			/* Enable or disable MPLS header */
	{"qinq",          pktgen_qinq},			/* Enable or disable Q-in-Q header */
	{"gre",           pktgen_gre},			/* Enable or disable GRE with IPv4 payload */
	{"gre_eth",       pktgen_gre_eth},		/* Enable or disable GRE with Ethernet payload */

	/* Range commands */
	{"dst_mac",       range_dst_mac},		/* Set the destination MAC address for a port */
	{"src_mac",       range_src_mac},		/* Set the src MAC address for a port */
	{"src_ip",        range_src_ip},		/* Set the source IP address and netmask value */
	{"dst_ip",        range_dst_ip},		/* Set the destination IP address */
	{"ip_proto",      range_ip_proto},		/* Set the IP Protocol type */
	{"src_port",      range_src_port},		/* Set the IP source port number */
	{"dst_port",      range_dst_port},		/* Set the IP destination port number */
	{"vlan_id",       range_vlan_id},		/* Set the vlan id value */
	{"mpls_entry",    range_mpls_entry},	/* Set the MPLS entry value */
	{"qinqids",       range_qinqids},		/* Set the Q-in-Q ID values */
	{"gre_key",       range_gre_key},		/* Set the GRE key */
	{"pkt_size",      range_pkt_size},		/* the packet size for a range port */
	{"cos",           range_cos},			/* Set the COS value */
	{"tos",           range_tos},			/* Set the COS value */
	{"set_range",     range},				/* Enable or disable sending range data on a port. */

	{"ports_per_page", pktgen_ports_per_page},	/* Set the number of ports per page */
	{"page",          pktgen_page},			/* Select a page to display, seq, range, pcap and a number from 0-N */
	{"port",          pktgen_port},			/* select a different port number used for sequence and range pages. */
	{"process",       pktgen_process},		/* Enable or disable input packet processing on a port */
	{"capture",       pktgen_capture},		/* Enable or disable capture on a port */
	{"bonding",       pktgen_bonding},		/* Enable or disable bonding on a port */
	{"garp",          pktgen_garp},			/* Enable or disable GARP packet processing on a port */
	{"blink",         pktgen_blink},		/* Blink an led on a port */
	{"help",          pktgen_help},			/* Return the help text */
	{"Lua.help",      pktgen_lua_help},		/* Lua command help text */

	{"isSending",     pktgen_isSending},	/* Test to see if a port is sending packets */
	{"linkState",     pktgen_linkState},	/* Return the current link state of a port */

	{"portSizes",     pktgen_portSizes},	/* Return the stats on the size of packet for a port. */
	{"pktStats",      pktgen_pktStats},		/* return the current packet stats on a port */
	{"portStats",     pktgen_portStats},	/* return the current port stats */
	{"portInfo",      pktgen_portInfo},		/* return the current port stats */

	{"compile",       pktgen_compile},		/* Convert a structure into a frame to be sent */
	{"decompile",     pktgen_decompile},	/* decompile a frame into Ethernet, IP, TCP, UDP or other protocols */
	{"sendPkt",       pktgen_sendPkt},		/* Not working. */
	{"recvPkt",       pktgen_recvPkt},		/* Not working. */

	{"run",           pktgen_run},			/* Load a Lua string or command file and execute it. */
	{"continue",      pktgen_continue},		/* Display a message and wait for keyboard key and return */
	{"input",         pktgen_input},		/* Wait for a keyboard input line and return line. */

	{"pattern",       pktgen_pattern},		/* Set pattern type */
	{"userPattern",   pktgen_user_pattern},	/* Set the user pattern string */
	{"latency",       pktgen_latency},		/* Enable or disable latency testing */
	{"jitter",        pktgen_jitter},		/* Set the jitter threshold */
	{"gtpu_teid",     range_gtpu_teid},		/* set GTP-U TEID. */

	{"rnd",           pktgen_rnd},			/* Set up the rnd function on a portlist */
	{"rnd_list",      pktgen_rnd_list},		/* Return a table of rnd bit patterns per port */

	{"rxtap",         pktgen_rxtap},		/* enable or disable rxtap */
	{"txtap",         pktgen_txtap},		/* enable or disable rxtap */

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

LUALIB_API int
luaopen_pktgen(lua_State *L)
{
	luaL_newlib(L, pktgenlib);

	lua_pushstring(L, "info");	/* Push the table index name */
	lua_newtable(L);		/* Create the structure table for information */

	setf_string(L, "Lua_Version", (char *)LUA_VERSION);
	setf_string(L, "Lua_Release", (char *)LUA_RELEASE);
	setf_string(L, "Lua_Copyright", (char *)LUA_COPYRIGHT);
	setf_string(L, "Lua_Authors", (char *)LUA_AUTHORS);

	setf_string(L, "Pktgen_Version", (char *)PKTGEN_VERSION);
	setf_string(L, "Pktgen_Copyright", (char *)copyright_msg());
	setf_string(L, "Pktgen_Authors", (char *)"Keith Wiles @ Intel Corp");
	setf_string(L, "DPDK_Version", (char *)rte_version());
	setf_string(L, "DPDK_Copyright", (char *)powered_by());

	setf_integer(L, "startSeqIdx", FIRST_SEQ_PKT);
	setf_integer(L, "singlePktIdx", SINGLE_PKT);
	setf_integer(L, "rangePktIdx", RANGE_PKT);
	setf_integer(L, "pingPktIdx", PING_PKT);
	setf_integer(L, "startExtraTxIdx", EXTRA_TX_PKT);

	setf_integer(L, "numSeqPkts", NUM_SEQ_PKTS);
	setf_integer(L, "numExtraTxPkts", NUM_EXTRA_TX_PKTS);
	setf_integer(L, "numTotalPkts", NUM_TOTAL_PKTS);

	setf_integer(L, "minPktSize", MIN_PKT_SIZE + ETHER_CRC_LEN);
	setf_integer(L, "maxPktSize", MAX_PKT_SIZE + ETHER_CRC_LEN);
	setf_integer(L, "minVlanID", MIN_VLAN_ID);
	setf_integer(L, "maxVlanID", MAX_VLAN_ID);
	setf_integer(L, "vlanTagSize", VLAN_TAG_SIZE);
	setf_integer(L, "mbufCacheSize", MBUF_CACHE_SIZE);

	setf_integer(L, "minCos", MIN_COS);
	setf_integer(L, "maxCos", MAX_COS);
	setf_integer(L, "minTOS", MIN_TOS);
	setf_integer(L, "maxTOS", MAX_TOS);

	setf_integer(L, "defaultPktBurst", DEFAULT_PKT_BURST);
	setf_integer(L, "defaultBuffSize", DEFAULT_MBUF_SIZE);
	setf_integer(L, "maxMbufsPerPort", MAX_MBUFS_PER_PORT);
	setf_integer(L, "maxPrimeCount", MAX_PRIME_COUNT);

	/* Now set the table for the info values. */
	lua_rawset(L, -3);

	lua_pushstring(L, "range");	/* Push the table index name */
	lua_newtable(L);		/* Create the structure table for information */

	luaL_setfuncs(L, pktgenlib_range, 0);

	lua_rawset(L, -3);

	return 1;
}

/**************************************************************************//**
 *
 * pktgen_lua_openlib - Open the Pktgen Lua library.
 *
 * DESCRIPTION
 * Open and initialize the Pktgen Lua Library.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_lua_openlib(lua_State *L)
{
	lua_gc(L, LUA_GCSTOP, 0);	/* stop collector during initialization */

	luaL_requiref(L, LUA_PKTGENLIBNAME, luaopen_pktgen, 1);
	lua_pop(L, 1);

	lua_gc(L, LUA_GCRESTART, 0);
}
