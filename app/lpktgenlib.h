/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2011 by Keith Wiles @ intel.com */

#ifndef LPKTGENLIB_H_
#define LPKTGENLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_ENABLED
#define lua_c
#include <lua.h>
#include <lauxlib.h>

#define LUA_PKTGENLIBNAME "pktgen"
#define PKTGEN_SHORTCUTS  "Pktgen"

LUALIB_API int luaopen_pktgen(lua_State *L);
void pktgen_lua_openlib(lua_State *L);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LPKTGENLIB_H_ */
