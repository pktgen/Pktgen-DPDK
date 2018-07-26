/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2018 Intel Corporation.
 */
/* Created 2011 by Keith Wiles @ intel.com */

#ifndef _LUA_SHELL_H_
#define _LUA_SHELL_H_

#define lua_c
#include "lua.h"
#include "lauxlib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lua_Shell {
	lua_State *L;
	void *private;
};

#define IO_PREFIX       "_IO_"
#define IOPREF_LEN      (sizeof(IO_PREFIX)/sizeof(char) - 1)
#define IO_INPUT        (IO_PREFIX "input")
#define IO_OUTPUT       (IO_PREFIX "output")

#if 0
/*
@@ luai_writestring/luai_writeline define how 'print' prints its results.
** They are only used in libraries and the stand-alone program. (The #if
** avoids including 'stdio.h' everywhere.)
*/
#if defined(LUA_LIB) || defined(lua_c)
#include <stdio.h>
extern void * _get_stdout(void * L);
#define lua_putstring(s)        (fwrite((s), sizeof(char), strlen(s), _get_stdout(L)), fflush(_get_stdout(L)))
#define lua_writestring(s,l)   (fwrite((s), sizeof(char), (l), _get_stdout(L)), fflush(_get_stdout(L)))
#define lua_writeline()  (lua_writestring("\n", 1), fflush(_get_stdout(L)))
#endif

/*
@@ luai_writestringerror defines how to print error messages.
** (A format string with one argument is enough for Lua...)
*/
extern void * _get_stderr(void * L);
#define lua_writestringerror(s, p) \
    (fprintf(_get_stderr(L), (s), (p)), fflush(_get_stderr(L)))
#endif

#define lua_putstring(s)        (fwrite((s), sizeof(char), strlen(s), _get_stdout(L)), fflush(_get_stdout(L)))

#define MAX_NEW_LIBS	16
typedef void (*newlib_t)(lua_State * L);

int lua_newlib_add(newlib_t n);
void lua_newlibs_init(struct lua_Shell *ls);

LUA_API void (lua_setprivate) (struct lua_Shell *ls, void * val) ;
LUA_API void * (lua_getprivate) (struct lua_Shell *ls);

void lua_callback_routine(char *);

void create_stdfile (struct lua_Shell *ls, FILE *f, const char *k, const char *fname);

int lua_dofile(lua_State *L, const char *name);
int lua_dostring(lua_State *L, const char *s, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _LUA_SHELL_H_ */
