/*-
 * Copyright (c) <2010-2017>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Indentifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _LUA_SOCKET_H_
#define _LUA_SOCKET_H_
#include <pthread.h>
#include "lua.h"
#include "lua_shell.h"

#define MAX_LUA_BUFFER_SIZE		1024

typedef struct luaServer_s {
	int32_t		server_socket;			/**< Server socket descriptor */
	int32_t		client_socket;			/**< Client socket descriptor */
	int32_t		socket_port;
	void	  * out, * in, * err;
	char	  * hostname;
	char		data[MAX_LUA_BUFFER_SIZE];
	struct lua_Shell ls;
} luaServer_t;

void _lua_openlib(lua_State *L);
void * _get_stdout(void * arg);
void * _get_stdin(void * arg);
void * _get_stderr(void * arg);
void _set_stdfiles(lua_State * L, luaServer_t * pServer);
void _reset_stdfiles(lua_State * L);

int lua_init_socket(struct lua_Shell *ls, pthread_t * pthread, char * hostname, int port);
void * lua_create_instance(void);

#endif /* _LUA_SOCKET_H_ */
