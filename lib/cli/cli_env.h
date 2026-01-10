/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2025>, Intel Corporation.
 */
/* Created by Keith Wiles @ intel.com */

#include <sys/queue.h>

#ifndef _CLI_ENV_H_
#define _CLI_ENV_H_

/**
 * @file
 * CLI environment variables.
 *
 * Supports simple key/value variables and dynamic variables backed by a
 * callback. Variables can be expanded in user input (e.g. $(FOO)).
 */

#ifdef __cplusplus
extern "C" {
#endif
struct cli;

typedef char *(*cli_sfunc_t)(const char *str);
/**< Callback to compute a dynamic environment variable string. */

struct env_node {
    TAILQ_ENTRY(env_node) next;
    const char *var;   /**< Variable name */
    const char *val;   /**< Variable value (owned by env) */
    cli_sfunc_t sfunc; /**< Optional callback to compute value */
};

struct cli_env {
    TAILQ_HEAD(, env_node) head; /**< link list of vars */
    int count;                   /**< Number of variables */
};

/**
 * Create an environment variable container.
 *
 * @return
 *   NULL on error or cli_env pointer
 */
struct cli_env *cli_env_create(void);

/**
 * Destroy an environment variable container.
 *
 * @param env
 *   Pointer to the environment structure
 */
void cli_env_destroy(struct cli_env *env);

/**
 * Set an environment variable.
 *
 * Adds or replaces a variable with a string value.
 *
 * @param env
 *   The cli_env pointer
 * @param var
 *   Pointer to the variable name const string
 * @param val
 *   Pointer to the string assigned to the variable
 * @return
 *   0 is OK was added or replaced or -1 if not valid
 */
int cli_env_set(struct cli_env *env, const char *var, const char *val);

/**
 * Set an environment variable with an optional callback.
 *
 * If @p sfunc is non-NULL, it may be used to generate a dynamic string value.
 *
 * @param env
 *   The cli_env pointer
 * @param var
 *   Pointer to the variable name const string.
 * @param sfunc
 *   Pointer to function (optional)
 * @param val
 *   Pointer to the string assigned to the variable
 * @return
 *   0 is OK was added or replaced or -1 if not valid
 */
int cli_env_string(struct cli_env *env, const char *var, cli_sfunc_t sfunc, const char *val);

/**
 * Get an environment variable value.
 *
 * @param env
 *   The cli_env pointer
 * @param var
 *   The const string variable name
 * @return
 *   NULL if not found or the const string
 */
const char *cli_env_get(struct cli_env *env, const char *var);

/**
 * Remove an environment variable.
 *
 * @param env
 *   The cli_env pointer
 * @param var
 *   The const string variable name
 * @return
 *   0 is OK or -1 if not found.
 */
int cli_env_del(struct cli_env *env, const char *var);

/**
 * Perform environment variable substitution on a command line.
 *
 * Expands occurrences of $(VAR) using values in @p env.
 *
 * @param env
 *   Pointer to the enviroment structure
 * @param line
 *   Pointer to the line to parse
 * @param sz
 *   Number of total characters the line can hold.
 * @return
 *   N/A
 */
void cli_env_substitution(struct cli_env *env, char *line, int sz);

/**
 * Get the number of variables in the environment.
 *
 * @param env
 *   Pointer to environment structure
 * @return
 *   Number of environment variables
 */
static inline int
cli_env_count(struct cli_env *env)
{
    return env->count;
}

/**
 * Get a snapshot list of environment variables.
 *
 * @param env
 *   Pointer to environment list
 * @param list
 *   Array of env_node pointers to be returned
 * @param max_size
 *   Max size of the list array
 */
int cli_env_get_all(struct cli_env *env, struct env_node **list, int max_size);

/**
 * Print all environment variables.
 *
 * @param env
 *   Pointer to the cli_env structure.
 */
void cli_env_show(struct cli_env *env);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_ENV_H_ */
