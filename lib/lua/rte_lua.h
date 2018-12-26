/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_H_
#define _RTE_LUA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/queue.h>

#include <rte_tailq.h>

#define lua_c

#include <lua.h>
#define LUA_COMPAT_APIINTCASTS
#include <lauxlib.h>
#include <lualib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua-shell"
#endif

#if !defined(LUA_INIT)
#define LUA_INIT		"LUA_INIT"
#endif

#define LUA_INITVERSION  \
	LUA_INIT "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

#define DBG(...)		do { \
					fprintf(stderr, "%s: ", __func__); \
					fprintf(stderr, __VA_ARGS__); \
				} while(0)

#define MAX_NEW_LIBS		16
#define LUA_BUFFER_SIZE		2048
#define MAX_NEW_LIBS		16
#define LUA_EOF			-1

#define IO_PREFIX       	"_IO_"
#define IOPREF_LEN      	(sizeof(IO_PREFIX)/sizeof(char) - 1)
#define IO_INPUT        	(IO_PREFIX "input")
#define IO_OUTPUT       	(IO_PREFIX "output")

typedef struct luaData {
	TAILQ_ENTRY(luaData) node;
	lua_State *L;		/**< Lua State pointer */
	int32_t server_socket;	/**< Server socket descriptor */
	int32_t client_socket;	/**< Client socket descriptor */
	int32_t socket_port;	/**< Port address for socket */
	char *buffer;		/**< Buffer for reading Lua code */
	void *out, *in, *err;	/**< stdout, stdin, stderr */
	char *hostname;		/**< Name of host for socket */
} luaData_t;

typedef void (*newlib_t)(lua_State *L);

luaData_t *lua_create_instance(void);

int lua_newlib_add(newlib_t n, int order);
void lua_newlibs_init(luaData_t *ld);

int lua_docall(lua_State *L, int narg, int nres);

int lua_dofile(luaData_t *ld, const char *name);
int lua_dostring(luaData_t *ld, const char *s, const char *name);
int lua_dolibrary(lua_State *L, const char *name);

int lua_execute_string(luaData_t *ld, char *str);
void lua_execute_close(luaData_t *ld);

void lua_create_stdfile (luaData_t *ld, FILE *f, const char *k, const char *fname);

void lua_set_stdfiles(luaData_t *ld);
void lua_reset_stdfiles(luaData_t *ld);

const char *lua_get_progname(void);
void lua_set_progname(const char *name);

void lua_destroy_instance(luaData_t *ld);
luaData_t *lua_find_luaData(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_H_ */
