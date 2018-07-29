/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */

/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_UTILS_H_
#define _RTE_LUA_UTILS_H_

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

#include <rte_lua.h>
#include <rte_lua_stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

char *rte_lua_strtrim(char *str);

static inline void
lua_putstring(const char *s)
{
	fwrite((s), sizeof(char), strlen(s), stdout);
	fflush(stdout);
}

static inline char *
lua_readline(luaData_t *ld)
{
	*ld->buffer = '\0';
	return fgets(ld->buffer, LUA_BUFFER_SIZE, lua_get_stdin(ld));
}

#ifdef __cplusplus
}
#endif

#endif /* _LUA_SUPPORT_H_ */
