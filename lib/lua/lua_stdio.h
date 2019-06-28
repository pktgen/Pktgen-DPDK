/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_STDIO_H_
#define _RTE_LUA_STDIO_H_

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

void *lua_get_stdout(luaData_t *ld);
void *lua_get_stdin(luaData_t *ld);
void *lua_get_stderr(luaData_t *ld);
void lua_create_stdfile(luaData_t *ld, FILE *f, const char *k, const char *fname);
void lua_set_stdfiles(luaData_t *ld);
void lua_reset_stdfiles(luaData_t *ld);
void lua_signal_set_stdfiles(luaData_t *ld);
void lua_signal_reset_stdfiles(luaData_t *ld);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_STDIO_H_ */
