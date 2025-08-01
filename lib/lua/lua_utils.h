/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
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

#include <lua_config.h>

#define lua_c

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#ifdef __cplusplus
extern "C" {
#endif

char *lua_strtrim(char *str);

static inline void
lua_putstring(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);

    fflush(stdout);
}

static inline char *
lua_readline(luaData_t *ld)
{
    *ld->buffer = '\0';
    return fgets(ld->buffer, LUA_BUFFER_SIZE, lua_get_stdin(ld));
}

static inline void
l_message(const char *pname, const char *msg)
{
    if (pname)
        lua_writestringerror("%s: ", pname);
    lua_writestringerror("%s\n", msg);
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static inline int
report(lua_State *L, int status)
{
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);

        l_message(lua_get_progname(), msg);
        lua_pop(L, 1); /* remove message */
    }
    return status;
}

#ifdef __cplusplus
}
#endif

#endif /* _LUA_SUPPORT_H_ */
