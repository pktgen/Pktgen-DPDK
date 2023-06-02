/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2023> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#define lua_dpdk_c
#define LUA_LIB
#define lua_c

#include <rte_common.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_cycles.h>
#include <vec.h>
#include <rte_timer.h>
#include <rte_version.h>

#include <pg_delay.h>
#include <pg_strings.h>
#include <portlist.h>

#include "lua_config.h"
#include "lua_stdio.h"
#include "lua_dpdk.h"
#include "lua_pktmbuf.h"
#ifdef RTE_LIBRTE_DAPI
#include <lua_dapi.h>
#else
int luaopen_dapi(lua_State *L);
#endif
#include "lua_vec.h"
#include "lua_utils.h"

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

#ifndef __rte_weak
#define __rte_weak __attribute__((__weak__))
#endif

static int lua_logtype;

static __inline__ void
__delay(int32_t t)
{
    int32_t n;

    while (t > 0) {
        n = (t > 10) ? 10 : t;
        rte_delay_us_sleep(n * 1000);
        t -= n;
    }
}

static int
dpdk_delay(lua_State *L)
{
    validate_arg_count(L, 1);

    __delay(luaL_checkinteger(L, 1));

    return 0;
}

static int
dpdk_pause(lua_State *L)
{
    char *str;
    int v;

    validate_arg_count(L, 2);

    str = (char *)luaL_checkstring(L, 1);
    if (strlen(str) > 0)
        lua_putstring(str);

    v = luaL_checkinteger(L, 2);
    __delay(v);

    return 0;
}

static int
dpdk_continue(lua_State *L)
{
    char buf[4], *str;
    int n;

    validate_arg_count(L, 1);

    str = (char *)luaL_checkstring(L, 1);

    if (strlen(str) > 0)
        lua_putstring(str);

    buf[0] = '\0';
    n      = fread(buf, 1, 1, (FILE *)lua_get_stdin(lua_find_luaData(L)));
    if (n > 0)
        buf[n] = '\0';

    lua_pushstring(L, buf);
    return 1;
}

static int
dpdk_input(lua_State *L)
{
    char buf[256], c, *str;
    uint32_t n, idx;

    validate_arg_count(L, 1);

    str = (char *)luaL_checkstring(L, 1);

    if (strlen(str) > 0)
        lua_putstring(str);

    idx      = 0;
    buf[idx] = '\0';
    while (idx < (sizeof(buf) - 2)) {
        n = fread(&c, 1, 1, (FILE *)lua_get_stdin(lua_find_luaData(L)));
        if ((n <= 0) || (c == '\r') || (c == '\n'))
            break;
        buf[idx++] = c;
    }
    buf[idx] = '\0';

    lua_pushstring(L, buf);
    return 1;
}

static int
dpdk_sleep(lua_State *L)
{
    validate_arg_count(L, 1);

    rte_delay_us_sleep((luaL_checkinteger(L, 1) * 1000) * 1000);
    return 0;
}

static void
link_state(lua_State *L, uint16_t pid)
{
    struct rte_eth_link link;
    char buff[32];

    lua_pushinteger(L, pid); /* Push the table index */
    rte_eth_link_get_nowait(pid, &link);

    if (link.link_status)
        snprintf(buff, sizeof(buff), "<UP-%u-%s>", (uint32_t)link.link_speed,
                 (link.link_duplex == RTE_ETH_LINK_FULL_DUPLEX) ? ("FD") : ("HD"));
    else
        snprintf(buff, sizeof(buff), "<--Down-->");
    lua_pushstring(L, buff);

    /* Now set the table as an array with pid as the index. */
    lua_rawset(L, -3);
}

static int
dpdk_linkState(lua_State *L)
{
    portlist_t portlist;
    uint32_t n;

    validate_arg_count(L, 1);

    portlist = 0;
    portlist_parse(luaL_checkstring(L, 1), RTE_MAX_ETHPORTS, &portlist);

    lua_newtable(L);

    n = 0;
    foreach_port(portlist, _do(link_state(L, pid); n++));

    setf_integer(L, "n", n);

    return 1;
}

static void
port_stats(lua_State *L, uint16_t pid)
{
    struct rte_eth_stats stats;

    rte_eth_stats_get(pid, &stats);

    lua_pushinteger(L, pid); /* Push the table index */
    lua_newtable(L);         /* Create the structure table for a packet */

    setf_integer(L, "ipackets", stats.ipackets);
    setf_integer(L, "opackets", stats.opackets);
    setf_integer(L, "ibytes", stats.ibytes);
    setf_integer(L, "obytes", stats.obytes);
    setf_integer(L, "ierrors", stats.ierrors);
    setf_integer(L, "oerrors", stats.oerrors);
    setf_integer(L, "rx_nombuf", stats.rx_nombuf);

    /* Now set the table as an array with pid as the index. */
    lua_rawset(L, -3);
}

static int
dpdk_portStats(lua_State *L)
{
    portlist_t portlist;
    uint32_t n;

    validate_arg_count(L, 1);

    portlist_parse(luaL_checkstring(L, 1), RTE_MAX_ETHPORTS, &portlist);

    lua_newtable(L);

    n = 0;
    foreach_port(portlist, _do(port_stats(L, pid); n++));

    setf_integer(L, "n", n);

    return 1;
}

static int
dpdk_compile(lua_State *L)
{
    validate_arg_count(L, 3);

    return 1;
}

static void
decompile_pkt(lua_State *L, uint16_t pid)
{
    char buff[256];
    struct pkt_data *p = NULL;

    lua_pushinteger(L, pid); /* Push the table index */
    lua_newtable(L);         /* Create the structure table for a packet */

    /* Add each member to the packet table indexed with port id. */
    setf_string(L, "eth_dst_addr", inet_mtoa(buff, sizeof(buff), &p->eth_dst_addr));
    setf_string(L, "eth_src_addr", inet_mtoa(buff, sizeof(buff), &p->eth_src_addr));
    if (p->ethType == RTE_ETHER_TYPE_IPV4) {
        setf_string(L, "ip_dst_addr",
                    inet_ntop4(buff, sizeof(buff), htonl(p->ip_dst_addr), 0xFFFFFFFF));
        setf_string(L, "ip_src_addr",
                    inet_ntop4(buff, sizeof(buff), htonl(p->ip_src_addr), p->ip_mask));
    }
    setf_integer(L, "dport", p->dport);
    setf_integer(L, "sport", p->sport);
    setf_integer(L, "vlanid", p->vlanid);
    setf_string(L, "ethType",
                (char *)((p->ethType == RTE_ETHER_TYPE_IPV4)   ? "ipv4"
                         : (p->ethType == RTE_ETHER_TYPE_IPV6) ? "ipv6"
                         : (p->ethType == RTE_ETHER_TYPE_VLAN) ? "vlan"
                                                               : "unknown"));
    setf_string(L, "ipProto",
                (char *)((p->ipProto == IPPROTO_TCP)    ? "tcp"
                         : (p->ipProto == IPPROTO_ICMP) ? "icmp"
                                                        : "udp"));

    setf_integer(L, "pktSize", p->pktSize + RTE_ETHER_CRC_LEN);
    /* Now set the table as an array with pid as the index. */
    lua_rawset(L, -3);
}

static int
dpdk_decompile(lua_State *L)
{
    portlist_t portlist;
    uint32_t n;

    validate_arg_count(L, 2);

    portlist_parse(luaL_checkstring(L, 2), RTE_MAX_ETHPORTS, &portlist);

    lua_newtable(L);

    n = 0;
    foreach_port(portlist, _do(decompile_pkt(L, pid); n++));

    setf_integer(L, "n", n);

    return 1;
}

static int
dpdk_tx_burst(lua_State *L)
{
    uint16_t pid, qid;
    uint32_t nb_pkts;
    int rc;
    struct rte_mbuf **mbs;

    validate_arg_count(L, 4);

    pid     = luaL_checkinteger(L, 1);
    qid     = luaL_checkinteger(L, 1);
    mbs     = (struct rte_mbuf **)luaL_checkudata(L, 3, "mbufs");
    nb_pkts = luaL_checkinteger(L, 4);

    rc = rte_eth_tx_burst(pid, qid, mbs, nb_pkts);

    lua_pushinteger(L, rc);

    return 1;
}

static int
dpdk_portCount(lua_State *L)
{
    validate_arg_count(L, 0);

    lua_pushinteger(L, rte_eth_dev_count_avail());

    return 1;
}

static int
dpdk_totalPorts(lua_State *L)
{
    validate_arg_count(L, 0);

    lua_pushinteger(L, rte_eth_dev_count_total());

    return 1;
}

static int
dpdk_rx_burst(lua_State *L)
{
    uint16_t pid, qid;
    struct rte_mbuf **mbs;
    int rc, nb_pkts;

    validate_arg_count(L, 4);

    pid     = luaL_checkinteger(L, 1);
    qid     = luaL_checkinteger(L, 1);
    mbs     = (struct rte_mbuf **)luaL_checkudata(L, 3, "mbufs");
    nb_pkts = luaL_checkinteger(L, 4);

    rc = rte_eth_tx_burst(pid, qid, mbs, nb_pkts);

    lua_pushinteger(L, rc);

    return 1;
}

static int
dpdk_version(lua_State *L)
{
    validate_arg_count(L, 0);

    lua_pushstring(L, rte_version());

    return 1;
}

static const luaL_Reg dpdklib[] = {
    {"delay", dpdk_delay}, /* Delay a given number of milliseconds */
    {"pause", dpdk_pause}, /* Delay for a given number of milliseconds and display message */
    {"continue", dpdk_continue},
    {"sleep", dpdk_sleep}, /* Delay a given number of seconds */

    {"portCount", dpdk_portCount},   /* Used port count value */
    {"totalPorts", dpdk_totalPorts}, /* Total ports seen by DPDK */
    {"port_stats", dpdk_portStats},
    {"input", dpdk_input},

    {"linkState", dpdk_linkState}, /* Return the current link state of a port */

    {"compile", dpdk_compile}, /* Convert a structure into a frame to be sent */
    {"decompile",
     dpdk_decompile}, /* decompile a frame into Ethernet, IP, TCP, UDP or other protocols */
    {"tx_burst", dpdk_tx_burst},
    {"rx_bust", dpdk_rx_burst},
    {"version", dpdk_version},

    {NULL, NULL}};

static const char *rte_copyright = "Copyright(c) <2010-2023>, Intel Corporation";

static int
luaopen_dpdk(lua_State *L)
{
    luaL_newlib(L, dpdklib);

    lua_pushstring(L, "dinfo"); /* Push the table index name */
    lua_newtable(L);            /* Create the structure table for information */

    setf_string(L, "Lua_Version", (char *)LUA_VERSION);
    setf_string(L, "Lua_Release", (char *)LUA_RELEASE);
    setf_string(L, "Lua_Copyright", (char *)LUA_COPYRIGHT);
    setf_string(L, "Lua_Authors", (char *)LUA_AUTHORS);

    setf_string(L, "DAPI_Authors", (char *)"Keith Wiles @ Intel Corp");
    setf_string(L, "DPDK_Version", (char *)rte_version());
    setf_string(L, "DPDK_Copyright", (char *)rte_copyright);

    /* Now set the table for the info values. */
    lua_rawset(L, -3);

    return 1;
}

__rte_weak int
luaopen_dapi(lua_State *L __rte_unused)
{
    return 0;
}

static void
dpdk_lua_openlib(lua_State *L)
{
    lua_gc(L, LUA_GCSTOP, 0);

    luaL_requiref(L, LUA_DPDK_LIBNAME, luaopen_dpdk, 1);
    lua_pop(L, 1);

    if (luaopen_pktmbuf(L))
        lua_pop(L, 1);

    if (luaopen_vec(L))
        lua_pop(L, 1);

    if (luaopen_dapi(L))
        lua_pop(L, 1);

    lua_gc(L, LUA_GCRESTART, 0);
}

RTE_INIT(dpdk_lua_init)
{
    lua_logtype = rte_log_register("librte.lua");
    if (lua_logtype >= 0)
        rte_log_set_level(lua_logtype, RTE_LOG_INFO);

    lua_newlib_add(dpdk_lua_openlib, 10);
}
