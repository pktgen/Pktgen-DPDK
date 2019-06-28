/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_SOCKET_H_
#define _RTE_LUA_SOCKET_H_

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

int lua_start_socket(luaData_t *ld, pthread_t *pthread, char *hostname, int port);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_SOCKET_H_ */
