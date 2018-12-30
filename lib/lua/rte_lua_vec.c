 /* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#define rte_lua_dpdk_c
#define LUA_LIB
#define lua_c

#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_cycles.h>
#include <rte_vec.h>
#include <rte_timer.h>
#include <rte_strings.h>
#include <rte_version.h>

#include "rte_lua.h"
#include "rte_lua_stdio.h"
#include "rte_lua_dpdk.h"
#include "rte_lua_vec.h"
#include "rte_lua_utils.h"

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

static const char *Vec	= "Vec";

static int
vec_new(lua_State *L)
{
	vec_t *v;
	int size = 0, top = lua_gettop(L);

	if (top > 1)
                return luaL_error(L, "new, Invalid arg count should be 0 or 1");

	if (top >= 1)
		size = luaL_checkint(L, 1);
	if (size == 0)
		size = rte_vec_calc_size(0);

	v = (struct rte_vec *)lua_newuserdata(L,
		(sizeof(struct rte_vec) * (size * sizeof(void *))));
	rte_vec_init(v, size, VEC_CREATE_FLAG);

	luaL_getmetatable(L, Vec);
	lua_setmetatable(L, -2);

	return 1;
}

static int
vec_add1(lua_State *L)
{
	struct rte_vec *v;

	validate_arg_count(L, 2);

	v = (struct rte_vec *)luaL_checkudata(L, 1, Vec);

	if (v->len >= v->tlen)
		return 0;

	v->list[v->len++] = lua_touserdata(L, 2);

	return 1;
}

static int
vec_tostring(lua_State *L)
{
	struct rte_vec *v;
	char buff[64];

	v = (struct rte_vec *)luaL_checkudata(L, 1, Vec);

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "__name");

	snprintf(buff, sizeof(buff), "%s<%d,%d,%s,0x%04x>",
		lua_tostring(L, -1),
		v->len, v->tlen, (v->vpool)? "V" : "_", v->flags);
	lua_pop(L, 3);

	lua_pushstring(L, buff);
	return 1;
}

static int
vec_gc(lua_State *L)
{
	struct rte_vec *v;
	struct rte_mbuf *m;
	int i;

	if (lua_gettop(L) != 1)
                return luaL_error(L, "vec.gc, Invalid arg count should be 1");
	v = (struct rte_vec *)lua_touserdata(L, 1);

	rte_vec_foreach(i, m, v) {
		if (m)
			rte_pktmbuf_free(m);
	}
	rte_vec_free(v);

	return 0;
}

static const struct luaL_Reg vec_methods[] = {
	{ "add1",	vec_add1 },
	{ NULL, 	NULL }
};

static const struct luaL_Reg vec_functions[] = {
	{ "new",	vec_new },
	{ NULL,		NULL }
};

int
luaopen_vec(lua_State *L)
{
	luaL_newmetatable(L, Vec);	// create and push new table called Vec
	lua_pushvalue(L, -1);		// dup the table on the stack

	lua_setfield(L, -2, "__index");	//

	luaL_setfuncs(L, vec_methods, 0);

	lua_pushstring(L, "__tostring");
	lua_pushcfunction(L, vec_tostring);
	lua_settable(L, -3);

	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, vec_gc);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_getglobal(L, LUA_DPDK_LIBNAME);
	lua_newtable(L);
	luaL_setfuncs(L, vec_functions, 0);
	lua_setfield(L, -2, LUA_VEC_LIBNAME);

	return 1;
}
