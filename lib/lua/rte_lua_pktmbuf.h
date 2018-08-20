/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */
/* Created by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_PKTMBUF_H_
#define _RTE_LUA_PKTMBUF_H_

#include <rte_log.h>

#define lua_c
#include <lua.h>
#include <lauxlib.h>

#include <rte_lua.h>
#include <rte_lua_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_PKTMBUF_LIBNAME     "pktmbuf"

typedef struct rte_mempool pktmbuf_t;

int luaopen_pktmbuf(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_PKTMBUF_H_ */
