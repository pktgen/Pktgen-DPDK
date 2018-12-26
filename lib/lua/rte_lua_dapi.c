/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#define rte_lua_dpdk_c
#define LUA_LIB
#define lua_c

#ifdef RTE_LIBRTE_API
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_cycles.h>
#include <rte_vec.h>
#include <rte_timer.h>
#include <rte_strings.h>
#include <rte_version.h>

#include <dapi.h>

#include "rte_lua.h"
#include "rte_lua_stdio.h"
#include "rte_lua_dpdk.h"
#include "rte_lua_dapi.h"
#include "rte_lua_utils.h"

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

static const char *Dapi = "Dapi";

static int
_create(lua_State *L)
{
	dapi_t **dapi;
	struct dapi *d;
	const char *name;

	validate_arg_count(L, 1);

	name = luaL_checkstring(L, 1);
	if (!name)
		luaL_error(L, "Name is empty");

	dapi = (struct dapi **)lua_newuserdata(L, sizeof(void *));

	d = dapi_create((char *)(uintptr_t)name, 0, 0);
	if (!d)
		return luaL_error(L, "create: dapi_create() failed");
	*dapi = d;

	luaL_getmetatable(L, Dapi);
	lua_setmetatable(L, -2);

	return 1;
}

static int
_destroy(lua_State *L)
{
	dapi_t **dapi;

	validate_arg_count(L, 1);

	dapi = (dapi_t **)luaL_checkudata(L, 1, Dapi);

	dapi_destroy(*dapi);

	return 0;
}

static int
_get(lua_State *L)
{
	validate_arg_count(L, 2);

	return 1;
}

static int
_put(lua_State *L)
{
	validate_arg_count(L, 2);

	return 0;
}

static int
_tostring(lua_State *L)
{
	dapi_t **dapi;
	struct dapi *d;
	char buff[64];

	dapi = (dapi_t **)luaL_checkudata(L, 1, Dapi);
	if (!dapi || !*dapi)
		return luaL_error(L, "tostring, dapi is nil");
	d = *dapi;

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "__name");

	snprintf(buff, sizeof(buff), "%s<%s>",
		lua_tostring(L, -1), d->name);

	lua_pop(L, 3);

	lua_pushstring(L, buff);
	return 1;
}

static int
_gc(lua_State *L __rte_unused)
{
	return 0;
}

static const struct luaL_Reg _methods[] = {
	{"get",		_get},
	{"put",		_put},
	{ NULL, 	NULL }
};

static const struct luaL_Reg _functions[] = {
	{ "create",	_create },
	{ "destroy",	_destroy},
	{ NULL,		NULL }
};

int
luaopen_dapi(lua_State *L)
{
	luaL_newmetatable(L, Dapi);	// create and push new table called Vec
	lua_pushvalue(L, -1);		// dup the table on the stack

	lua_setfield(L, -2, "__index");	//

	luaL_setfuncs(L, _methods, 0);

	lua_pushstring(L, "__tostring");
	lua_pushcfunction(L, _tostring);
	lua_settable(L, -3);

	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, _gc);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_getglobal(L, LUA_DPDK_LIBNAME);
	lua_newtable(L);
	luaL_setfuncs(L, _functions, 0);

	lua_setfield(L, -2, LUA_DAPI_LIBNAME);

	return 1;
}
#endif
