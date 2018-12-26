/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_DAPI_H_
#define _RTE_LUA_DAPI_H_

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

#define LUA_DAPI_LIBNAME        "dapi"

int luaopen_dapi(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_DAPI_H_ */
