/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/* Create from lua.c 2018 by Keith Wiles @ intel.com */

#include <sys/queue.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>

#include <rte_eal.h>
#include <rte_eal_memconfig.h>
#include <rte_rwlock.h>

#include "rte_lua.h"
#include "rte_lua_stdio.h"
#include "rte_lua_utils.h"

TAILQ_HEAD(rte_luaData_list, rte_tailq_entry);

static struct rte_tailq_elem rte_luaData_tailq = {
	.name = "RTE_LUADATA",
};

EAL_REGISTER_TAILQ(rte_luaData_tailq);

static const char *progname = LUA_PROGNAME;

static struct newlib_info {
	newlib_t lib_func;
	int order;
} newlibs[MAX_NEW_LIBS];

static int newlibs_idx = 0;

const char *
lua_get_progname(void)
{
	return progname;
}

void
lua_set_progname(const char *name)
{
	progname = name;
}

int
lua_newlib_add(newlib_t n, int order)
{
	if (newlibs_idx >= MAX_NEW_LIBS)
		return -1;

	newlibs[newlibs_idx].order = order;
	newlibs[newlibs_idx++].lib_func = n;

	return 0;
}

static int
cmp_libs(const void *p1, const void *p2)
{
	const struct newlib_info *ni1 = p1;
	const struct newlib_info *ni2 = p2;

	return (ni1->order - ni2->order);
}

void
lua_newlibs_init(luaData_t *ld)
{
	struct newlib_info _libs[MAX_NEW_LIBS];
	int i;

	memcpy(_libs, newlibs, sizeof(newlibs));

	qsort(_libs, newlibs_idx, sizeof(struct newlib_info), cmp_libs);

	for(i = 0; i < newlibs_idx; i++)
		_libs[i].lib_func(ld->L);
}

static int
handle_luainit(luaData_t *ld)
{
	const char *name;
	const char *init;

	name = "=" LUA_INITVERSION;
	init = getenv(&name[1]);

	if (!init) {
		name = "=" LUA_INIT;
		init = getenv(&name[1]); /* try alternative name */
	}

	if (!init)
		return LUA_OK;

	if (init[0] == '@')
		return lua_dofile(ld, init + 1);
	else
		return lua_dostring(ld, init, name);
}

luaData_t *
lua_create_instance(void)
{
	luaData_t *ld;
	struct rte_luaData_list *luaData_list = NULL;
	struct rte_tailq_entry *te;

	ld = (luaData_t *)malloc(sizeof(luaData_t));
	if (!ld)
		return NULL;

	memset(ld, 0, sizeof(luaData_t));

	ld->client_socket = -1;
	ld->server_socket = -1;

	ld->buffer = malloc(LUA_BUFFER_SIZE);
	if (!ld->buffer) {
		free(ld);
		return NULL;
	}
	memset(ld->buffer, 0, LUA_BUFFER_SIZE);

	ld->L = luaL_newstate();
	if (!ld->L) {
		free(ld);
		return NULL;
	}

	ld->in = stdin;
	ld->out = stdout;
	ld->err = stderr;

	if (handle_luainit(ld)) {
		free(ld);
		DBG("handle_luainit() failed\n");
		return NULL;
	}

	luaL_openlibs(ld->L);
	lua_newlibs_init(ld);

	luaData_list = RTE_TAILQ_CAST(rte_luaData_tailq.head, rte_luaData_list);

	/* try to allocate tailq entry */
	te = malloc(sizeof(*te));
	if (te == NULL) {
		DBG("Cannot allocate tailq entry!\n");
		lua_close(ld->L);
		free(ld);
		return NULL;
	}
	memset(te, 0, sizeof(*te));
	te->data = ld;

	rte_rwlock_write_lock(RTE_EAL_TAILQ_RWLOCK);
	TAILQ_INSERT_TAIL(luaData_list, te, next);
	rte_rwlock_write_unlock(RTE_EAL_TAILQ_RWLOCK);

	// Make sure we display the copyright string for Lua.
	lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
	lua_writeline();

	return ld;
}

void
lua_destroy_instance(luaData_t *ld)
{
	struct rte_luaData_list *luaData_list = NULL;
	struct rte_tailq_entry *te;

	if (!ld)
		return;

	luaData_list = RTE_TAILQ_CAST(rte_luaData_tailq.head, rte_luaData_list);

	rte_rwlock_write_lock(RTE_EAL_TAILQ_RWLOCK);

	TAILQ_FOREACH(te, luaData_list, next) {
		if (te->data == (void *)ld)
			break;
	}
	if (te) {
		TAILQ_REMOVE(luaData_list, te, next);
		free(te);
	}

	rte_rwlock_write_unlock(RTE_EAL_TAILQ_RWLOCK);

	free(ld);
}

luaData_t *
lua_find_luaData(lua_State *L)
{
	struct rte_luaData_list *luaData_list = NULL;
	struct rte_tailq_entry *te;
	luaData_t *ret_ld = NULL;

	if (!L)
		return NULL;

	luaData_list = RTE_TAILQ_CAST(rte_luaData_tailq.head, rte_luaData_list);

	rte_rwlock_write_lock(RTE_EAL_TAILQ_RWLOCK);

	TAILQ_FOREACH(te, luaData_list, next) {
		luaData_t *ld = (luaData_t *)te->data;
		if (ld->L == L) {
			ret_ld = ld;
			break;
		}
	}

	rte_rwlock_write_unlock(RTE_EAL_TAILQ_RWLOCK);

	return ret_ld;
}

/*
** Message handler used to run all chunks
*/
static int
msghandler(lua_State *L)
{
	const char *msg = lua_tostring(L, 1);

	if (msg == NULL) {	/* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&	/* does it have a metamethod */
		                lua_type(L, -1) == LUA_TSTRING) {	/* that produces a string? */
			DBG("No metadata to produce a string on object\n");
			return 1;	/* that is the message */
		} else {
			msg = lua_pushfstring(L,
			                      "(error object is a %s value)",
			                      luaL_typename(L, 1));
		}
	}
	luaL_traceback(L, L, msg, 1);	/* append a standard traceback */
	return 1;		/* return the traceback */
}

static int
_k(lua_State *L, int status, lua_KContext ctx)
{
	(void)L;
	(void)ctx;

	return status;
}

int
lua_docall(lua_State *L, int narg, int nres)
{
	int status;
	int base = 0;

	base = lua_gettop(L);

	lua_pushcfunction(L, msghandler);
	lua_insert(L, base);

	status = _k(L, lua_pcallk(L, narg, nres, base, 0, _k), 0);

	return status;
}

int
lua_dofile(luaData_t *ld, const char *name)
{
	int status;

	status = luaL_loadfile(ld->L, name);

	if (status == LUA_OK)
		status = lua_docall(ld->L, 0, 0);
	else
		printf("lua_docall(%s) failed\n", name);

	return report(ld->L, status);
}

int
lua_dostring(luaData_t *ld, const char *s, const char *name)
{
	int status;

	status = luaL_loadbuffer(ld->L, s, strlen(s), name);

	if (status == LUA_OK)
		status = lua_docall(ld->L, 0, 0);

	return report(ld->L, status);
}

int
lua_dolibrary(lua_State *L, const char *name)
{
	int status;

	lua_getglobal(L, "require");
	lua_pushstring(L, name);

	status = lua_docall(L, 1, 1); /* call 'require(name)' */
	if (status == LUA_OK)
		lua_setglobal(L, name); /* global[name] = require return */

	return report(L, status);
}

int
lua_execute_string(luaData_t *ld, char *buffer )
{
	lua_State *L = ld->L;

	if (!L)
		return -1;

	buffer = rte_lua_strtrim(buffer);
	if (!buffer)
		return -1;

	if ( luaL_dostring(L, buffer) != 0 ) {
		DBG("%s\n", lua_tostring(L,-1));
		return -1;
	}

	return 0;
}

void
lua_execute_close(luaData_t *ld)
{
	if ( ld->L )
		lua_close(ld->L);
}
