/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
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

#include "rte_lua.h"
#include "rte_lua_utils.h"

static lua_State *globalL;

static const char *progname = LUA_PROGNAME;

static newlib_t	newlibs[MAX_NEW_LIBS];
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
lua_newlib_add(newlib_t n)
{
	if ( newlibs_idx >= MAX_NEW_LIBS )
		return -1;

	newlibs[newlibs_idx++] = n;

	return 0;
}

void
lua_newlibs_init(luaData_t *ld)
{
	int i;

	for(i = 0; i < newlibs_idx; i++)
		newlibs[i](ld->L);
}

luaData_t *
lua_create_instance(void)
{
	luaData_t *ld;

	ld = (luaData_t *)malloc(sizeof(luaData_t));
	if (ld) {
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

		lua_newlibs_init(ld);

		// Make sure we display the copyright string for Lua.
		lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
		lua_writeline();
	}

	return ld;
}

static void
lstop(lua_State *L, lua_Debug *ar)
{
	(void) ar; /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
	luaL_error(L, "interrupted!");
}

static void
laction(int i)
{
	signal(i, SIG_DFL ); /* if another SIGINT happens before lstop,
				terminate process (default action) */
	if (globalL)
		lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int
_k(lua_State *L, int status, lua_KContext ctx)
{
	DBG("Entry\n");

	(void)L;
	(void)ctx;

	signal(SIGINT, SIG_DFL );
	globalL = NULL;

	DBG("Exit status %d\n", status);
	return status;
}

static int
lua_docall(lua_State *L, int narg, int nres)
{
	int status;

	DBG("Entry narg %d, nres %d\n", narg, nres);
	globalL = L;				/* to be available to 'laction' */
	signal(SIGINT, laction);

	status = _k(L, lua_pcallk(L, narg, nres, 0, 0, _k), 0);

	DBG("Exit status %d\n", status);
	return status;
}

static void
l_message(const char *pname, const char *msg)
{
	if (pname)
		lua_writestringerror("%s: ", pname);
	lua_writestringerror("%s\n", msg);
}

static int
report(lua_State *L, int status)
{
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		if (msg == NULL )
			msg = "(error object is not a string)";
		l_message(progname, msg);
		lua_pop(L, 1);
	}
	return status;
}

#if 0
/* the next function is called unprotected, so it must avoid errors */
static void
finalreport(lua_State *L, int status)
{
	if (status != LUA_OK) {
		const char *msg = (lua_type(L, -1) == LUA_TSTRING) ?
		                  lua_tostring(L, -1) : NULL;
		if (msg == NULL )
			msg = "(error object is not a string)";
		l_message(progname, msg);
		lua_pop(L, 1);
	}
}
#endif

int
lua_dofile(luaData_t *ld, const char *name)
{
	int status;

	DBG("Entry (%s)\n", name);
	status = luaL_loadfile(ld->L, name);

	DBG("Here 0 status %d\n", status);
	if (status == LUA_OK)
		status = lua_docall(ld->L, 0, 0);

	DBG("Call report with status %d\n", status);
	return report(ld->L, status);
}

int
lua_dostring(luaData_t *ld, const char *s, const char *name)
{
	int status;

	DBG("s (%s), name (%s)\n", s, name);
	status = luaL_loadbuffer(ld->L, s, strlen(s), name);

	DBG("Here 0 status %d\n", status);
	if (status == LUA_OK)
		status = lua_docall(ld->L, 0, 0);

	DBG("Call report with status %d\n", status);
	return report(ld->L, status);
}

int
lua_dolibrary(lua_State *L, const char *name)
{
	int status;

	DBG("Entry %s\n", name);

	lua_getglobal(L, "require");
	lua_pushstring(L, name);

	DBG("Here 0\n");
	status = lua_docall(L, 1, 1); /* call 'require(name)' */
	if (status == LUA_OK)
		lua_setglobal(L, name); /* global[name] = require return */

	DBG("Call report with status %d\n", status);
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
