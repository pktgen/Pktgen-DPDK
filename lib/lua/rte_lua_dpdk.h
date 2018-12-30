/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_DPDK_H_
#define _RTE_LUA_DPDK_H_

#include <stdint.h>
#include <netinet/in.h>

#include <rte_log.h>

#define lua_c
#include <lua.h>
#include <lauxlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_DPDK_LIBNAME        "dpdk"

#define LUA_LOG(level, fmt, args...) \
	rte_log(RTE_LOG_ ## level, lua_logtype, "%s(): " fmt "\n", \
		__func__, ## args)

#define _do(_exp)       do { _exp; } while ((0))

#define foreach_port(_portlist, _action)                                \
        do {                                                            \
                uint64_t *_pl = (uint64_t *)&_portlist;                 \
                uint16_t pid, idx, bit;                                 \
                                                                        \
                RTE_ETH_FOREACH_DEV(pid) {                              \
                        idx = (pid / (sizeof(uint64_t) * 8));           \
                        bit = (pid - (idx * (sizeof(uint64_t) * 8)));   \
                        if ( (_pl[idx] & (1LL << bit)) == 0)            \
                                continue;                               \
                        _action;                                        \
                }                                                       \
        } while ((0))

#define validate_arg_count(_l, _n) do {                                 \
                switch (lua_gettop(_l)) {                               \
                default: return luaL_error(_l,                          \
                        "%s, Invalid arg count should be %d",           \
                        __func__, _n);                                  \
                case _n:                                                \
                        break;                                          \
                }                                                       \
        } while((0))

struct pkt_data {
        /* Packet type and information */
        struct ether_addr eth_dst_addr; /**< Destination Ethernet address */
        struct ether_addr eth_src_addr; /**< Source Ethernet address */

        uint32_t ip_src_addr;      /**< Source IPv4 address also used for IPv6 */
        uint32_t ip_dst_addr;      /**< Destination IPv4 address */
        uint32_t ip_mask;          /**< IPv4 Netmask value */

        uint16_t sport;         /**< Source port value */
        uint16_t dport;         /**< Destination port value */
        uint16_t ethType;       /**< IPv4 or IPv6 */
        uint16_t ipProto;       /**< TCP or UDP or ICMP */
        uint16_t vlanid;        /**< VLAN ID value if used */
        uint16_t ether_hdr_size;/**< Size of Ethernet header in packet for VLAN ID */

        uint16_t pktSize;       /**< Size of packet in bytes not counting FCS */
        uint16_t pad0;
};

typedef struct rte_mempool pktmbuf_t;
typedef struct rte_mempool mempool_t;
typedef struct rte_vec vec_t;

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

#if 0
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
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_DPDK_H_ */
