/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_PKTMBUF_H_
#define _RTE_LUA_PKTMBUF_H_

/**
 * @file
 *
 * Lua bindings for DPDK pktmbuf mempools.
 *
 * Exposes pktmbuf mempool operations to Lua scripts under the "pktmbuf"
 * library name, allowing scripts to allocate and manage packet buffers.
 */

#include <rte_log.h>

#define lua_c
#include <lua.h>
#include <lauxlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_PKTMBUF_LIBNAME "pktmbuf"

typedef struct rte_mempool pktmbuf_t;

/**
 * Open the pktmbuf Lua library and register its functions.
 *
 * Called automatically by the Lua runtime when the library is required.
 *
 * @param L
 *   Lua state to register the library into.
 * @return
 *   Number of values pushed onto the Lua stack (1 — the library table).
 */
int luaopen_pktmbuf(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_PKTMBUF_H_ */
