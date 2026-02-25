/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2026> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_DAPI_H_
#define _RTE_LUA_DAPI_H_

/**
 * @file
 *
 * Lua bindings for the DAPI (Data-plane API) library.
 *
 * Exposes DAPI functionality to Lua scripts under the "dapi" library name.
 * Only available when RTE_LIBRTE_DAPI is enabled at build time.
 */

#include <rte_log.h>

#define lua_c
#include <lua.h>
#include <lauxlib.h>

#ifdef RTE_LIBRTE_DAPI
#include <dapi.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dapi dapi_t;

#define LUA_DAPI_LIBNAME "dapi"

/**
 * Open the DAPI Lua library and register its functions.
 *
 * Called automatically by the Lua runtime when the library is required.
 *
 * @param L
 *   Lua state to register the library into.
 * @return
 *   Number of values pushed onto the Lua stack (1 — the library table).
 */
int luaopen_dapi(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_DAPI_H_ */
