/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_DPDK_H_
#define _RTE_LUA_DPDK_H_

/**
 * @file
 *
 * DPDK Lua helper types, macros, and inline field accessor functions.
 *
 * Defines common structures (pkt_data), iteration macros (foreach_port),
 * argument validation helpers (validate_arg_count), and a set of
 * setf_*/getf_* inline functions for pushing and pulling values between
 * C and Lua table fields.
 */

#include <stdint.h>
#include <netinet/in.h>

#include <rte_log.h>

#define lua_c
#include <lua.h>
#include <lauxlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LUA_DPDK_LIBNAME "dpdk"

/** Emit a log message at @p level using the Lua log type. */
#define LUA_LOG(level, fmt, args...) \
    rte_log(RTE_LOG_##level, lua_logtype, "%s(): " fmt "\n", __func__, ##args)

/** Execute @p _exp inside a do/while(0) block (suppresses compiler warnings). */
#define _do(_exp) \
    do {          \
        _exp;     \
    } while ((0))

/**
 * Iterate over all ports set in @p _portlist and execute @p _action for each.
 *
 * @param _portlist   64-bit portlist bitmap.
 * @param _action     Statement or block executed with @c pid set to each active port id.
 */
#define foreach_port(_portlist, _action)                  \
    do {                                                  \
        uint64_t *_pl = (uint64_t *)&_portlist;           \
        uint16_t pid, idx, bit;                           \
                                                          \
        RTE_ETH_FOREACH_DEV(pid)                          \
        {                                                 \
            idx = (pid / (sizeof(uint64_t) * 8));         \
            bit = (pid - (idx * (sizeof(uint64_t) * 8))); \
            if ((_pl[idx] & (1LL << bit)) == 0)           \
                continue;                                 \
            _action;                                      \
        }                                                 \
    } while ((0))

/**
 * Validate that exactly @p _n arguments are present on the Lua stack.
 *
 * Returns a Lua error to the caller if the count does not match.
 *
 * @param _l   Lua state.
 * @param _n   Expected argument count.
 */
#define validate_arg_count(_l, _n)                                                     \
    do {                                                                               \
        switch (lua_gettop(_l)) {                                                      \
        default:                                                                       \
            return luaL_error(_l, "%s, Invalid arg count should be %d", __func__, _n); \
        case _n:                                                                       \
            break;                                                                     \
        }                                                                              \
    } while ((0))

    struct pkt_data {
        /* Packet type and information */
        struct rte_ether_addr eth_dst_addr; /**< Destination Ethernet address */
        struct rte_ether_addr eth_src_addr; /**< Source Ethernet address */

        uint32_t ip_src_addr; /**< Source IPv4 address also used for IPv6 */
        uint32_t ip_dst_addr; /**< Destination IPv4 address */
        uint32_t ip_mask;     /**< IPv4 Netmask value */

        uint16_t sport;          /**< Source port value */
        uint16_t dport;          /**< Destination port value */
        uint16_t ethType;        /**< IPv4 or IPv6 */
        uint16_t ipProto;        /**< TCP or UDP or ICMP */
        uint16_t vlanid;         /**< VLAN ID value if used */
        uint16_t ether_hdr_size; /**< Size of Ethernet header in packet for VLAN ID */

        uint16_t pktSize; /**< Size of packet in bytes not counting FCS */
        uint16_t pad0;
    };

    typedef struct rte_mempool pktmbuf_t;
    typedef struct rte_mempool mempool_t;
    typedef struct vec vec_t;

    /**
     * Push an integer value and set it as field @p name in the table at the top of the stack.
     *
     * @param L      Lua state.
     * @param name   Field name in the table.
     * @param value  Integer value to set.
     */
    static __inline__ void setf_integer(lua_State * L, const char *name, lua_Integer value)
    {
        lua_pushinteger(L, value);
        lua_setfield(L, -2, name);
    }

    /**
     * Push a C closure (with no upvalues) and set it as field @p name in the table at stack top.
     *
     * @param L     Lua state.
     * @param name  Field name in the table.
     * @param fn    C function to push as a closure.
     */
    static __inline__ void setf_function(lua_State * L, const char *name, lua_CFunction fn)
    {
        lua_pushcclosure(L, fn, 0);
        lua_setfield(L, -2, name);
    }

    /**
     * Push a NUL-terminated string and set it as field @p name in the table at stack top.
     *
     * @param L      Lua state.
     * @param name   Field name in the table.
     * @param value  String value to push.
     */
    static __inline__ void setf_string(lua_State * L, const char *name, const char *value)
    {
        lua_pushstring(L, value);
        lua_setfield(L, -2, name);
    }

    /**
     * Push a length-delimited string and set it as field @p name in the table at stack top.
     *
     * @param L      Lua state.
     * @param name   Field name in the table.
     * @param value  String data to push (need not be NUL-terminated).
     * @param len    Number of bytes in @p value.
     */
    static __inline__ void setf_stringLen(lua_State * L, const char *name, char *value, int len)
    {
        lua_pushlstring(L, value, len);
        lua_setfield(L, -2, name);
    }

    /**
     * Push a light userdata pointer and set it as field @p name in the table at stack top.
     *
     * @param L      Lua state.
     * @param name   Field name in the table.
     * @param value  Pointer to push as light userdata.
     */
    static __inline__ void setf_udata(lua_State * L, const char *name, void *value)
    {
        lua_pushlightuserdata(L, value);
        lua_setfield(L, -2, name);
    }

    /**
     * Read an integer field from the Lua table at stack index 3.
     *
     * @param L      Lua state (field is read from the table at index 3).
     * @param field  Name of the integer field to retrieve.
     * @return       Field value as uint32_t, or 0 if the field is absent or not an integer.
     */
    static __inline__ uint32_t getf_integer(lua_State * L, const char *field)
    {
        uint32_t value = 0;

        lua_getfield(L, 3, field);
        if (lua_isinteger(L, -1))
            value = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        return value;
    }

    /**
     * Read a string field from the Lua table at stack index 3.
     *
     * @param L      Lua state (field is read from the table at index 3).
     * @param field  Name of the string field to retrieve.
     * @return       Pointer to the Lua-managed string, or NULL if absent or not a string.
     */
    static __inline__ const char *getf_string(lua_State * L, const char *field)
    {
        const char *value = NULL;

        lua_getfield(L, 3, field);
        if (lua_isstring(L, -1))
            value = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        return value;
    }

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_DPDK_H_ */
