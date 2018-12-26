/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/* Created 2010 by Keith Wiles @ intel.com */

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
#include "rte_lua_utils.h"
#include "rte_lua_socket.h"

static int
server_startup(luaData_t *ld)
{
	char *err_msg = NULL;
	struct sockaddr_in ipaddr;
	struct hostent *pHost;
	int linger = 1;

	pthread_detach(pthread_self());

	err_msg = "gethostbyname failed";
	if( (pHost = gethostbyname(ld->hostname)) == NULL )
		goto error_exit;

	err_msg = "Socket create failed";
	if ( (ld->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		goto error_exit;

	memset(&ipaddr, 0, sizeof(ipaddr));
	ipaddr.sin_family = AF_INET;
	ipaddr.sin_port = htons(ld->socket_port);
	ipaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	err_msg = "Setsockopt failed";
	if (setsockopt(ld->server_socket, SOL_SOCKET, SO_REUSEADDR, &linger,
	                sizeof(linger)) == -1)
		goto error_exit;

	err_msg = "Bind failed";
	if (bind(ld->server_socket, (struct sockaddr *) &ipaddr,
	                sizeof(ipaddr)) < 0)
		goto error_exit;

	err_msg = "Listen failed";
	if (listen(ld->server_socket, 5) < 0)
		goto error_exit;

	return 0;

error_exit:
	if (ld->server_socket != -1)
		close(ld->server_socket);
	ld->server_socket = -1;
	if (err_msg) {
		perror(err_msg);
		fflush(stdout);
	}
	return -1;
}

static void
_socket_open(luaData_t *ld)
{
	if (ld) {
		ld->in  = fdopen(ld->client_socket, "r");
		ld->out = fdopen(ld->client_socket, "w");
		ld->err = fdopen(ld->client_socket, "w");
	}
}

static void
_socket_close(luaData_t *ld)
{
	if (ld) {
		fclose(ld->in);
		fclose(ld->out);
		fclose(ld->err);
	}
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)

/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static int
incomplete(lua_State * L, int status)
{
	if (status == LUA_ERRSYNTAX) {
		size_t lmsg;
		const char *msg = lua_tolstring(L, -1, &lmsg);

		if (lmsg >= marklen && !strcmp(msg + lmsg - marklen, EOFMARK)) {
			lua_pop(L, 1);
			return 1;
		}
	}
	return 0;		/* else... */
}

/*
** Read a line, and push it into the Lua stack.
*/
static int
pushline(luaData_t *ld, int firstline)
{
	lua_State *L = ld->L;
	size_t l;
	char *b = lua_readline(ld);

	if (!b)
		return 0;

	l = strlen(b);
	if (l > 0 && b[l - 1] == '\n')	/* line ends with newline? */
		b[--l] = '\0';	/* remove it */
	if (firstline && b[0] == '=')	/* for compatibility with 5.2, ... */
		lua_pushfstring(L, "return %s", b + 1);	/* change '=' to 'return' */
	else
		lua_pushlstring(L, b, l);
	return 1;
}

/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int
addreturn(lua_State * L)
{
	const char *line = lua_tostring(L, -1);	/* original line */
	const char *retline = lua_pushfstring(L, "return %s;", line);
	int status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");

	if (status == LUA_OK)
		lua_remove(L, -2);	/* remove modified line */
	else
		lua_pop(L, 2);	/* pop result from 'luaL_loadbuffer' and modified line */

	return status;
}

/*
** Read multiple lines until a complete Lua statement
*/
static int
multiline(luaData_t *ld)
{
	lua_State *L = ld->L;

	for (;;) {		/* repeat until gets a complete statement */
		size_t len;
		const char *line = lua_tolstring(L, 1, &len);	/* get what it has */
		int status = luaL_loadbuffer(L, line, len, "=stdin");	/* try it */

		if (!incomplete(L, status) || !pushline(ld, 0))
			return status;	/* cannot or should not try to add continuation line */

		lua_pushliteral(L, "\n");	/* add newline... */
		lua_insert(L, -2);	/* ...between the two lines */
		lua_concat(L, 3);	/* join them */
	}
}

/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int
loadline(luaData_t *ld)
{
	lua_State *L = ld->L;
	int status;

	lua_settop(L, 0);
	if (!pushline(ld, 1))
		return -1;	/* no input <EOF> */

	status = addreturn(L);

	if (status != LUA_OK)	/* 'return ...' did not work? */
		status = multiline(ld);	/* try as command, maybe with continuation lines */

	lua_remove(L, 1);	/* remove line from the stack */

	return status;
}

/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void
l_print(lua_State * L)
{
	int n = lua_gettop(L);

	if (n > 0) {		/* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK,
				"too many results to print");
		if (lua_isfunction(L, 1))
			return;

		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK)
			l_message(lua_get_progname(),
				lua_pushfstring(L,
						"error calling 'print' (%s)",
						lua_tostring(L, -1)));
	}
}

static void
doREPL(luaData_t *ld)
{
	lua_State *L = ld->L;
	int status;
	const char *oldprogname = lua_get_progname();

	lua_set_progname(NULL);
	while ((status = loadline(ld)) != -1) {
		if (status == LUA_OK)
			status = lua_docall(L, 0, LUA_MULTRET);
		if (status == LUA_OK)
			l_print(L);
		else
			report(L, status);
	}
	if (lua_gettop(L))
		lua_settop(L, 0);	/* clear stack */

	lua_set_progname(oldprogname);
}

static void
handle_server_requests(luaData_t *ld)
{
	struct sockaddr_in ipaddr;
	socklen_t	len;

	ld->client_socket = -1;

	do {
		len = sizeof(struct sockaddr_in);
		if ( (ld->client_socket = accept(ld->server_socket,
		                                 (struct sockaddr *)&ipaddr, &len)) < 0) {
			perror("accept failed");
			break;
		}

		if (ld->client_socket > 0) {
			_socket_open(ld);
			lua_set_stdfiles(ld);

			doREPL(ld);

			lua_reset_stdfiles(ld);
			_socket_close(ld);

			close(ld->client_socket);
			ld->client_socket = -1;
		}
	} while(1);

	if (ld->server_socket > 0) {
		close(ld->server_socket);
		ld->server_socket = -1;
	}
}

static void *
lua_server(void *arg)
{
	luaData_t *ld = arg;

	if (server_startup(ld))
		fprintf(stderr, "server_startup() failed!\n");

	handle_server_requests(ld);

	return NULL;
}

int rte_thread_setname(pthread_t id, const char *name);

int
lua_start_socket(luaData_t *ld, pthread_t *pthread, char *hostname, int port)
{
	int r;

	ld->client_socket = -1;
	ld->server_socket = -1;
	ld->socket_port = port;
	ld->hostname = strdup( (hostname) ? hostname : "localhost" );

	/* Split assert and function because using NDEBUG define will remove function */
	r = pthread_create(pthread, NULL, lua_server, ld);
	if (r)
		return -1;

	rte_thread_setname(*pthread, "pktgen-socket");

	return 0;
}
