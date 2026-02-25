/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2026> Intel Corporation.
 */

/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _RTE_LUA_STDIO_H_
#define _RTE_LUA_STDIO_H_

/**
 * @file
 *
 * Lua stdio stream redirection helpers.
 *
 * These functions allow Lua's stdin/stdout/stderr to be redirected to
 * arbitrary FILE objects, which is needed when Lua scripts run over a
 * TCP socket rather than the process's standard streams.
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
 * Return the FILE pointer used as Lua's stdout for instance @p ld.
 *
 * @param ld   Lua instance.
 * @return     FILE pointer for stdout, or NULL if not set.
 */
void *lua_get_stdout(luaData_t *ld);

/**
 * Return the FILE pointer used as Lua's stdin for instance @p ld.
 *
 * @param ld   Lua instance.
 * @return     FILE pointer for stdin, or NULL if not set.
 */
void *lua_get_stdin(luaData_t *ld);

/**
 * Return the FILE pointer used as Lua's stderr for instance @p ld.
 *
 * @param ld   Lua instance.
 * @return     FILE pointer for stderr, or NULL if not set.
 */
void *lua_get_stderr(luaData_t *ld);

/**
 * Create and register a Lua stdio file object backed by FILE @p f.
 *
 * @param ld     Lua instance.
 * @param f      Underlying C FILE to wrap.
 * @param k      Lua global key name for this file (e.g. "stdout").
 * @param fname  Lua filename string associated with the file object.
 */
void lua_create_stdfile(luaData_t *ld, FILE *f, const char *k, const char *fname);

/**
 * Redirect Lua's stdin/stdout/stderr to the socket streams in @p ld.
 *
 * @param ld   Lua instance whose socket streams should become the std files.
 */
void lua_set_stdfiles(luaData_t *ld);

/**
 * Restore Lua's stdin/stdout/stderr to the process standard streams.
 *
 * @param ld   Lua instance to reset.
 */
void lua_reset_stdfiles(luaData_t *ld);

/**
 * Set Lua's stdio streams from within a signal handler context.
 *
 * Signal-safe variant of lua_set_stdfiles().
 *
 * @param ld   Lua instance.
 */
void lua_signal_set_stdfiles(luaData_t *ld);

/**
 * Reset Lua's stdio streams from within a signal handler context.
 *
 * Signal-safe variant of lua_reset_stdfiles().
 *
 * @param ld   Lua instance.
 */
void lua_signal_reset_stdfiles(luaData_t *ld);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LUA_STDIO_H_ */
