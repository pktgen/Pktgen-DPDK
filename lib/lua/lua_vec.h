/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_VEC_H_
#define _RTE_LUA_VEC_H_

/**
 * @file
 *
 * Lua bindings for the vec (pointer vector) library.
 *
 * Exposes vec container operations to Lua scripts under the "vec"
 * library name.
 */

#include <stdint.h>
#include <netinet/in.h>

#include <rte_log.h>

#define lua_c
#include <lua.h>
#include <lauxlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_VEC_LIBNAME "vec"

/**
 * Open the vec Lua library and register its functions.
 *
 * Called automatically by the Lua runtime when the library is required.
 *
 * @param L
 *   Lua state to register the library into.
 * @return
 *   Number of values pushed onto the Lua stack (1 — the library table).
 */
int luaopen_vec(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_VEC_H_ */
