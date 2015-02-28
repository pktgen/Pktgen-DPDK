/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
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
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>

#include <rte_config.h>
#include <rte_version.h>

#include <rte_log.h>
#include <rte_tailq.h>
#include <rte_common.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_timer.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#define lua_c
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lua-socket.h"
#include "lua_shell.h"

extern void * lua_get_private(void * L);
extern void lua_shell(void *);
extern void lua_set_stdfiles(lua_State * L, FILE * in, FILE * out, FILE * err);
extern void lua_reset_stdfiles(lua_State * L);
extern int execute_lua_string(lua_State * L, char * str);
extern void execute_lua_close(lua_State * L);

static void
_socket_open(luaServer_t * pServer)
{
	if ( pServer != NULL ) {
		pServer->in		= fdopen(pServer->client_socket, "r");
		pServer->out	= fdopen(pServer->client_socket, "w");
		pServer->err	= fdopen(pServer->client_socket, "w");
	}
}

static void
_socket_close(luaServer_t * pServer)
{
	if ( pServer != NULL ) {
		fclose(pServer->in);
		fclose(pServer->out);
		fclose(pServer->err);
	}
}

void *
_get_stdout(void * arg)
{
	luaServer_t * s;

	if ( arg == NULL )
		return stdout;
	s = luaL_getprivate(arg);
	if ( (s == NULL) || (s->out == NULL) )
		return stdout;
	return s->out;
}

void *
_get_stdin(void * arg)
{
	luaServer_t * s;

	if ( arg == NULL )
		return stdin;
	s = luaL_getprivate(arg);
	if ( (s == NULL) || (s->in == NULL) )
		return stdin;
	return s->in;
}

void *
_get_stderr(void * arg)
{
	luaServer_t * s;

	if ( arg == NULL )
		return stderr;
	s = luaL_getprivate(arg);
	if ( (s == NULL) || (s->err == NULL) )
		return stderr;
	return s->err;
}

void
_set_stdfiles(lua_State * L, luaServer_t * pServer)
{
	lua_set_stdfiles(L, pServer->in, pServer->out, pServer->err);
	signal(SIGPIPE, SIG_IGN);
}

void
_reset_stdfiles(lua_State * L)
{
	signal(SIGPIPE, SIG_DFL);
	lua_reset_stdfiles(L);
}

/**************************************************************************//**
*
* process_server_requests - Process a GUI or socket request.
*
* DESCRIPTION
* Process the socket request using the list of commands above.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
process_server_requests( luaServer_t * pServer )
{
	struct sockaddr_in ipaddr;
	socklen_t	len;

	pServer->client_socket = -1;
	for(;;) {
		len = sizeof(struct sockaddr_in);
		if ( (pServer->client_socket = accept(pServer->server_socket, (struct sockaddr *)&ipaddr, &len)) < 0) {
			perror("accept failed");
			break;
		}

		if ( pServer->client_socket > 0 ) {

			_socket_open(pServer);
			lua_shell(pServer);
			_socket_close(pServer);

			close(pServer->client_socket);
			pServer->client_socket = -1;
		}
	}

	if ( pServer->server_socket > 0 ) {
		close(pServer->server_socket);
		pServer->server_socket = -1;
	}
}

/**************************************************************************//**
*
* lua_server_create - Create the Lua server connection
*
* DESCRIPTION
* Create the Lua Server connection.
*
* RETURNS: Return zero if OK or one if error.
*
* SEE ALSO:
*/

static int
server_create( void * arg )
{
	luaServer_t	  * pServer = (luaServer_t *)arg;
	char	* err_msg = NULL;
	struct sockaddr_in ipaddr;
	struct hostent * pHost;
	int		linger = 1;

	err_msg = "gethostbyname failed";
	if( (pHost = gethostbyname(pServer->hostname)) == NULL )
		goto error_exit;

	err_msg = "Socket create failed";
	if ( (pServer->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		goto error_exit;

	memset(&ipaddr, 0, sizeof(ipaddr));
	ipaddr.sin_family		= AF_INET;
	ipaddr.sin_port			= htons(pServer->socket_port);
	ipaddr.sin_addr.s_addr	= htonl(INADDR_ANY);

	err_msg = "Setsockopt failed";
	if (setsockopt(pServer->server_socket, SOL_SOCKET, SO_REUSEADDR, &linger, sizeof(linger)) == -1)
		goto error_exit;

	err_msg = "Bind failed";
	if (bind(pServer->server_socket, (struct sockaddr *) &ipaddr, sizeof(ipaddr)) < 0)
		goto error_exit;

	err_msg = "Listen failed";
	if (listen(pServer->server_socket, 5) < 0)
		goto error_exit;

	process_server_requests(pServer);

	return 0;

error_exit:
	if ( pServer->server_socket != -1 )
		close( pServer->server_socket );
	pServer->server_socket = -1;
	if ( err_msg ) {
		perror(err_msg);
		fflush(stdout);
	}
	return 1;
}

/**************************************************************************//**
*
* lua_server - Lua socket server thread
*
* DESCRIPTION
* Lua socket server for Pktgen.
*
* RETURNS: void *
*
* SEE ALSO:
*/

static void *
lua_server(void * arg) {

	pthread_detach( pthread_self() );

	server_create((luaServer_t *)arg);

	return NULL;
}

/**************************************************************************//**
*
* lua_init_socket - Setup the system to process socket requests.
*
* DESCRIPTION
* Zero out the socket structure and setup the callback to process commands.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

int
lua_init_socket(lua_State * L, pthread_t * pthread, char * hostname, int port)
{
	luaServer_t * p;
	int		r;

	p = luaL_getprivate(L);
	if ( p == NULL )
		return -1;

	p->client_socket	= -1;
	p->server_socket	= -1;
	p->socket_port		= port;
	p->hostname			= strdup( (hostname) ? hostname : "localhost" );

	/* Split assert and function because using NDEBUG define will remove function */
	r = pthread_create(pthread, NULL, lua_server, p);
	assert( r == 0 );
	return 0;
}

/**************************************************************************//**
*
* lua_create_instance - Open a Lua instance to be used later.
*
* DESCRIPTION
* Create a Lua instance and init all of the libraries.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void *
lua_create_instance(void)
{
	luaServer_t * p;
	lua_State * L;

	p = (luaServer_t *)calloc(1, sizeof(luaServer_t));
	if (p == NULL)
		return NULL;

	L = luaL_newstate();
	if ( L == NULL ) {
		free(p);
		return NULL;
	}

	p->client_socket	= -1;
	p->server_socket	= -1;
	p->in				= NULL;
	p->out				= NULL;
	p->err				= NULL;

	luaL_setprivate(L, p);

	// Make sure we display the copyright string for Lua.
	luai_writestring(L, LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
	luai_writeline(L);

	lua_newlibs_init(L);

    return L;
}
