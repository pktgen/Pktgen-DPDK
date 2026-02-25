/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2011 by Keith Wiles @ intel.com */

#ifndef LPKTGENLIB_H_
#define LPKTGENLIB_H_

/**
 * @file
 *
 * Lua bindings for the Pktgen application API.
 *
 * Registers the "pktgen" Lua library so that Lua scripts can control
 * port configuration, traffic generation, and statistics collection.
 * Declarations are conditionally compiled under LUA_ENABLED.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_ENABLED
#define lua_c
#include <lua.h>
#include <lauxlib.h>

#define LUA_PKTGENLIBNAME "pktgen" /**< Lua library name for the pktgen bindings */
#define PKTGEN_SHORTCUTS  "Pktgen" /**< Lua shortcut table name */

/**
 * Open the Pktgen Lua library and register all pktgen API functions.
 *
 * Called automatically by the Lua runtime when the library is required.
 *
 * @param L
 *   Lua state to register the library into.
 * @return
 *   Number of values pushed onto the Lua stack (1 — the library table).
 */
LUALIB_API int luaopen_pktgen(lua_State *L);

/**
 * Register all Pktgen Lua libraries (including shortcut aliases) into @p L.
 *
 * @param L
 *   Lua state to register the libraries into.
 */
void pktgen_lua_openlib(lua_State *L);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LPKTGENLIB_H_ */
