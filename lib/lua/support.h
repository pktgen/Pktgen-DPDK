/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _LUA_SUPPORT_H_
#define _LUA_SUPPORT_H_

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

#define LUA_BUFFER_SIZE		2048
#define MAX_NEW_LIBS		16
#define LUA_EOF			-1

#define IO_PREFIX       	"_IO_"
#define IOPREF_LEN      	(sizeof(IO_PREFIX)/sizeof(char) - 1)
#define IO_INPUT        	(IO_PREFIX "input")
#define IO_OUTPUT       	(IO_PREFIX "output")

#define DBG(...)		do { \
					fprintf(stderr, "%s: ", __func__); \
					fprintf(stderr, __VA_ARGS__); \
				} while(0)

typedef struct {
	lua_State *L;
	int32_t server_socket;	/**< Server socket descriptor */
	int32_t client_socket;	/**< Client socket descriptor */
	int32_t socket_port;
	char *buffer;
	void *out, *in, *err;
	char *hostname;
} luaData_t;

typedef void (*newlib_t)(luaData_t *ld);

luaData_t *lua_create_instance(void);
int lua_start_socket(luaData_t *ld, pthread_t *pthread, char *hostname, int port);

void _lua_openlib(luaData_t *ld);
void *_get_stdout(luaData_t *ld);
void *_get_stdin(luaData_t *ld);
void *_get_stderr(luaData_t *ld);
void _set_stdfiles(luaData_t *ld);
void _reset_stdfiles(luaData_t *ld);

int lua_shell(luaData_t *ld);
int lua_newlib_add(newlib_t n);
void lua_newlibs_init(luaData_t *ld);
void lua_callback_routine(void *func);
int lua_dofile(luaData_t *ld, const char *name);
int lua_dostring(luaData_t *ld, const char *s, const char *name);
int lua_dolibrary(luaData_t *ld, const char *name);

void create_stdfile (luaData_t *ld, FILE *f, const char *k, const char *fname);

void lua_set_stdfiles(luaData_t *ld);
void lua_reset_stdfiles(luaData_t *ld);
int execute_lua_string(luaData_t *ld, char * str);
void execute_lua_close(luaData_t *ld);

static inline void
lua_putstring(const char *s)
{
	fwrite((s), sizeof(char), strlen(s), stdout);
	fflush(stdout);
}

#ifdef __cplusplus
}
#endif

#endif /* _LUA_SUPPORT_H_ */
