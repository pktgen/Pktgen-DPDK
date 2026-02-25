/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2026> Intel Corporation.
 */

/* Created 2018 by Keith Wiles @ intel.com */

#ifndef _LUA_CONFIG_H_
#define _LUA_CONFIG_H_

/**
 * @file
 *
 * Core Lua instance management for Pktgen.
 *
 * Provides functions to create, configure, and destroy Lua interpreter
 * instances (luaData_t), load and execute Lua scripts/strings, register
 * new native libraries, and manage Lua's standard I/O streams.  All
 * declarations are conditionally compiled under LUA_ENABLED.
 */

#ifdef LUA_ENABLED
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
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LUA_ENABLED

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME "lua-shell"
#endif

#if !defined(LUA_INIT)
#define LUA_INIT "LUA_INIT"
#endif

#define LUA_INITVERSION LUA_INIT "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

#define DBG(...)                           \
    do {                                   \
        fprintf(stderr, "%s: ", __func__); \
        fprintf(stderr, __VA_ARGS__);      \
    } while (0)

#define MAX_NEW_LIBS    16
#define LUA_BUFFER_SIZE 8192
#define MAX_NEW_LIBS    16
#define LUA_EOF         -1

#define IO_PREFIX  "_IO_"
#define IOPREF_LEN (sizeof(IO_PREFIX) / sizeof(char) - 1)
#define IO_INPUT   (IO_PREFIX "input")
#define IO_OUTPUT  (IO_PREFIX "output")

typedef struct luaData {
    TAILQ_ENTRY(luaData) node;
    lua_State *L;          /**< Lua State pointer */
    int32_t server_socket; /**< Server socket descriptor */
    int32_t client_socket; /**< Client socket descriptor */
    int32_t socket_port;   /**< Port address for socket */
    char *buffer;          /**< Buffer for reading Lua code */
    void *out, *in, *err;  /**< stdout, stdin, stderr */
    char *hostname;        /**< Name of host for socket */
} luaData_t;

/** Callback type for registering a new native Lua library. */
typedef void (*newlib_t)(lua_State *L);

/**
 * Allocate and initialise a new Lua interpreter instance.
 *
 * @return   Pointer to the new luaData_t, or NULL on failure.
 */
luaData_t *lua_create_instance(void);

/**
 * Register a native library constructor to be called during Lua initialisation.
 *
 * Libraries are called in ascending @p order when lua_newlibs_init() runs.
 *
 * @param n      Constructor function to register.
 * @param order  Relative initialisation order (lower values run first).
 * @return       0 on success, -1 if the registry is full.
 */
int lua_newlib_add(newlib_t n, int order);

/**
 * Call all registered library constructors for instance @p ld.
 *
 * Should be called once after lua_create_instance() to make all
 * registered native libraries available within the Lua state.
 *
 * @param ld   Lua instance to initialise libraries into.
 */
void lua_newlibs_init(luaData_t *ld);

/**
 * Call a Lua function that is already on the stack in protected mode.
 *
 * @param L     Lua state.
 * @param narg  Number of arguments on the stack.
 * @param nres  Expected number of return values.
 * @return      LUA_OK on success, or a Lua error code.
 */
int lua_docall(lua_State *L, int narg, int nres);

/**
 * Load and execute a Lua file.
 *
 * @param ld    Lua instance to use.
 * @param name  Path to the Lua source file.
 * @return      LUA_OK on success, or a Lua error code.
 */
int lua_dofile(luaData_t *ld, const char *name);

/**
 * Execute a Lua string within instance @p ld.
 *
 * @param ld    Lua instance to use.
 * @param s     Lua source code string.
 * @param name  Chunk name used in error messages (e.g. the script name).
 * @return      LUA_OK on success, or a Lua error code.
 */
int lua_dostring(luaData_t *ld, const char *s, const char *name);

/**
 * Dynamically load a Lua C library by name and open it.
 *
 * @param L     Lua state.
 * @param name  Name of the C library to load (passed to package.loadlib).
 * @return      LUA_OK on success, or a Lua error code.
 */
int lua_dolibrary(lua_State *L, const char *name);

/**
 * Execute a NUL-terminated Lua string and handle errors.
 *
 * Convenience wrapper around lua_dostring() for interactive use.
 *
 * @param ld    Lua instance to use.
 * @param str   Lua source code string.
 * @return      0 on success, non-zero on error.
 */
int lua_execute_string(luaData_t *ld, char *str);

/**
 * Close the client socket associated with instance @p ld.
 *
 * @param ld   Lua instance whose socket should be closed.
 */
void lua_execute_close(luaData_t *ld);

/**
 * Create and register a Lua stdio file object backed by FILE @p f.
 *
 * @param ld     Lua instance.
 * @param f      Underlying C FILE to wrap.
 * @param k      Lua global key name (e.g. "stdout").
 * @param fname  Lua filename string for the file object.
 */
void lua_create_stdfile(luaData_t *ld, FILE *f, const char *k, const char *fname);

/**
 * Redirect Lua's standard streams to the socket streams in @p ld.
 *
 * @param ld   Lua instance to configure.
 */
void lua_set_stdfiles(luaData_t *ld);

/**
 * Restore Lua's standard streams to the process standard streams.
 *
 * @param ld   Lua instance to reset.
 */
void lua_reset_stdfiles(luaData_t *ld);

/**
 * Return the current Lua programme name string.
 *
 * @return   Programme name as a NUL-terminated string.
 */
const char *lua_get_progname(void);

/**
 * Set the Lua programme name shown in error messages.
 *
 * @param name   New programme name string (must remain valid for the lifetime
 *               of the Lua instance).
 */
void lua_set_progname(const char *name);

/**
 * Destroy a Lua instance and free all associated resources.
 *
 * @param ld   Lua instance to destroy.
 */
void lua_destroy_instance(luaData_t *ld);

/**
 * Find the luaData_t that owns Lua state @p L.
 *
 * Searches the global registry of all created Lua instances.
 *
 * @param L   Lua state to look up.
 * @return    Pointer to the owning luaData_t, or NULL if not found.
 */
luaData_t *lua_find_luaData(lua_State *L);
#else
/** Close the Lua instance socket (stub when LUA_ENABLED is not defined). */
void lua_execute_close(void *ld);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LUA_CONFIG_H_ */
