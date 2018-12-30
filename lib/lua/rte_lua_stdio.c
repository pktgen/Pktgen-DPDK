/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/* Created 2018 by Keith Wiles @ intel.com */

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
#include "rte_lua_stdio.h"

#define tolstream(L)    ((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

typedef luaL_Stream LStream;

void *
lua_get_stdout(luaData_t *ld)
{
	if (!ld || !ld->out)
		return stdout;

	return ld->out;
}

void *
lua_get_stdin(luaData_t *ld)
{
	if (!ld || !ld->in)
		return stdin;

	return ld->in;
}

void *
lua_get_stderr(luaData_t *ld)
{
	if (!ld || !ld->err)
		return stderr;

	return ld->err;
}

void
lua_signal_set_stdfiles(luaData_t *ld)
{
	lua_set_stdfiles(ld);
	signal(SIGPIPE, SIG_IGN);
}

void
lua_signal_reset_stdfiles(luaData_t *ld)
{
	signal(SIGPIPE, SIG_DFL);
	lua_reset_stdfiles(ld);
}

void
lua_set_stdfiles(luaData_t *ld)
{
	lua_State *L = ld->L;

	luaL_getmetatable(L, LUA_FILEHANDLE);

	if (lua_isnil(L, -1)) {
		DBG("luaL_getmetatable() returned NIL\n");
		return;
	}

	/* create (and set) default files */
	lua_create_stdfile(ld, ld->in, IO_INPUT, "stdin");
	lua_create_stdfile(ld, ld->out, IO_OUTPUT, "stdout");
	lua_create_stdfile(ld, ld->err, NULL, "stderr");
}

void
lua_reset_stdfiles(luaData_t *ld)
{
	lua_State *L = ld->L;

	luaL_getmetatable(L, LUA_FILEHANDLE);

	if (lua_isnil(L, -1))
		return;

	/* create (and set) default files */
	lua_create_stdfile(ld, stdin, IO_INPUT, "stdin");
	lua_create_stdfile(ld, stdout, IO_OUTPUT, "stdout");
	lua_create_stdfile(ld, stderr, NULL, "stderr");
}

/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
static int
io_noclose(lua_State *L)
{
	LStream *p = tolstream(L);
	p->closef = &io_noclose;  /* keep file opened */
	lua_pushnil(L);
	lua_pushliteral(L, "cannot close standard file");
	return 2;
}

static LStream *
newprefile(lua_State *L)
{
	LStream *p = (LStream *)lua_newuserdata(L, sizeof(LStream));
	p->closef = NULL;  /* mark file handle as 'closed' */
	luaL_setmetatable(L, LUA_FILEHANDLE);
	return p;
}

void
lua_create_stdfile(luaData_t *ld, FILE *f, const char *k, const char *fname)
{
	lua_State *L = ld->L;
	LStream *p = newprefile(L);

	p->f = f;
	p->closef = &io_noclose;
	if (k != NULL) {
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
	}
	lua_setfield(L, -2, fname);  /* add file to module */
}
