/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_SOCKET_H_
#define _RTE_LUA_SOCKET_H_

/**
 * @file
 *
 * Lua TCP socket server for remote script execution.
 *
 * Starts a background thread that listens on a TCP port and executes
 * Lua code received from connected clients, enabling external control
 * of a running Pktgen instance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define lua_c

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the Lua TCP socket server in a background pthread.
 *
 * The server accepts connections on @p port and executes received Lua
 * strings within the @p ld Lua instance.
 *
 * @param ld
 *   Lua instance to use for executing received scripts.
 * @param pthread
 *   Output: handle of the created pthread.
 * @param hostname
 *   Hostname or IP address string to bind to.
 * @param port
 *   TCP port number to listen on.
 * @return
 *   0 on success, -1 on error.
 */
int lua_start_socket(luaData_t *ld, pthread_t *pthread, char *hostname, int port);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_SOCKET_H_ */
