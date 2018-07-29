/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
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

static int
traceback(lua_State *L)
{
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1)) { /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring")) /* try its 'tostring' metamethod */
			lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

static char *
pushline(luaData_t *ld)
{
	lua_State *L = ld->L;
	char *b;

	DBG("Entry Stack top %d\n", lua_gettop(L));
	do {
		b = lua_readline(ld);
		if (!b)
			break;
		fprintf(stderr, ">>> %s", b);		/* String contains newline */
		b = rte_lua_strtrim(b);

		/* skip blank lines or comments */
		if (!b || (b[0] == '-' && b[1] == '-'))
			continue;
		break;
	} while(1);
	DBG("Exit Stack top %d\n", lua_gettop(L));
	return b;
}

static int
loadline(luaData_t *ld)
{
	lua_State *L = ld->L;
	int status = 0;
	int firstline = 1;

	DBG("Entry Stack top %d\n", lua_gettop(L));
	do {
		char *line;
		size_t l;

		line = pushline(ld);
		if (!line)
			break;

		/* first line starts with `=' then change to 'return ' */
		if (firstline && line[0] == '=')
			snprintf(ld->buffer, LUA_BUFFER_SIZE, "return %s", line + 1);
		firstline = 0;
		l = strlen(line);

		DBG("Before luaL_loadbuffer Stack top %d len %lu\n", lua_gettop(L), l);
		status = luaL_loadbuffer(L, line, l, "=stdin");
		DBG("After luaL_loadbuffer Stack top %d status %d\n", lua_gettop(L), status);
		if (status != LUA_OK)
			traceback(L);
		break;
	} while(1);
	DBG("Exit Stack top %d\n", lua_gettop(L));
	return status;
}

static void
dotty(luaData_t *ld)
{
	lua_State *L = ld->L;
	int status;
	const char *oldprogname = lua_get_progname();

	lua_set_progname(NULL);

	DBG("Entry\n");
	while ((status = loadline(ld)) != -1) {
		if (status)
			break;
		DBG("Before lua_pcall Stack top %d\n", lua_gettop(L));
		status = lua_pcall(L, 0, LUA_MULTRET, 0);
		DBG("After lua_pcall Stack top %d status %d\n", lua_gettop(L), status);
		if (status) {
			DBG("%s\n", lua_tostring(L, -1));
			break;
		}
	}
	lua_writeline();

	lua_set_progname(oldprogname);
	DBG("Exit %s\n", lua_get_progname());
}

static void
handle_server_requests(luaData_t *ld)
{
	struct sockaddr_in ipaddr;
	socklen_t	len;

	DBG("ld %p\n", ld);
	ld->client_socket = -1;

	do {
		len = sizeof(struct sockaddr_in);
		DBG("Wait accept\n");
		if ( (ld->client_socket = accept(ld->server_socket,
		                                 (struct sockaddr *)&ipaddr, &len)) < 0) {
			perror("accept failed");
			break;
		}

		DBG("Accept found fd %d\n", ld->client_socket);
		if (ld->client_socket > 0) {
			DBG("Socket Open\n");
			_socket_open(ld);
			dotty(ld);
			_socket_close(ld);
			DBG("Socket Closed\n");

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

	if (server_startup((luaData_t *)arg))
		fprintf(stderr, "server_startup() failed!\n");

	handle_server_requests((luaData_t *)arg);

	return NULL;
}

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
