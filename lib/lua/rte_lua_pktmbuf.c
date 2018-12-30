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
#include "rte_lua_pktmbuf.h"
#include "rte_lua_utils.h"

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

static int lua_inst;
static const char *Pktmbuf = "Pktmbuf";

static int
_new(lua_State *L)
{
	pktmbuf_t **mbp;
	struct rte_mempool *mp;
	const char *name;
	uint32_t n, size, csize;
	char poolname[RTE_MEMPOOL_NAMESIZE];

	validate_arg_count(L, 4);

	name = luaL_checkstring(L, 1);
	if (!name)
		luaL_error(L, "Name is empty");
	n = luaL_checkint(L, 2);
	if (n == 0)
		luaL_error(L, "Number of entries is zero");
	size = luaL_checkint(L, 3);
	if (size == 0)
		luaL_error(L, "Size of entries is zero");
	csize = luaL_checkint(L, 4);

	mbp = (pktmbuf_t **)lua_newuserdata(L, sizeof(void *));

	snprintf(poolname, sizeof(poolname), "%s-%d", name, lua_inst++);
	mp = rte_pktmbuf_pool_create(poolname, n, csize, 0, size, rte_socket_id());
	if (mp == NULL)
		luaL_error(L, "Failed to create MBUF Pool");
	*mbp = mp;

	luaL_getmetatable(L, Pktmbuf);
	lua_setmetatable(L, -2);

	return 1;
}

static int
_destroy(lua_State *L)
{
	pktmbuf_t **mbp;

	validate_arg_count(L, 1);

	mbp = (pktmbuf_t **)luaL_checkudata(L, 1, Pktmbuf);

	rte_mempool_free(*mbp);

	return 0;
}

static int
_get(lua_State *L)
{
	pktmbuf_t **mbp;
	struct rte_mbuf *m;

	validate_arg_count(L, 1);

	mbp = luaL_checkudata(L, 1, Pktmbuf);

	m = rte_pktmbuf_alloc(*mbp);

	if (m)
		lua_pushlightuserdata(L, m);

	return 1;
}

static int
_put(lua_State *L)
{
	struct rte_mbuf *m;

	validate_arg_count(L, 2);

	m = lua_touserdata(L, 2);

	rte_pktmbuf_free(m);

	return 0;
}

static int
_tostring(lua_State *L)
{
	pktmbuf_t **mbp;
	struct rte_mempool *mp;
	char buff[64];

	mbp = (pktmbuf_t **)luaL_checkudata(L, 1, Pktmbuf);
	if (!mbp || !*mbp)
		return luaL_error(L, "tostring, pktmbuf is nil");

	mp = *mbp;

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "__name");

	snprintf(buff, sizeof(buff), "%s<%s,%d,%d,%d,%d>",
		lua_tostring(L, -1),
		mp->name, mp->size, mp->elt_size,
		mp->cache_size, mp->socket_id);
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
	{ "new",	_new },
	{ "destroy",	_destroy},
	{ NULL,		NULL }
};

int
luaopen_pktmbuf(lua_State *L)
{
	luaL_newmetatable(L, Pktmbuf);	// create and push new table called Vec
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

	lua_setfield(L, -2, LUA_PKTMBUF_LIBNAME);

	return 1;
}
