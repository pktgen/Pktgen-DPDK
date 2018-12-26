/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */
/* Create by: Keith Wiles @ intel.com */

#include <sys/queue.h>

#ifndef _CLI_ENV_H_
#define _CLI_ENV_H_

/**
 * @file
 * RTE Command line interface
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
struct cli;

typedef char * (*cli_sfunc_t)(const char *str);
    /**< CLI function pointer type for a Environment node  */

struct env_node {
	TAILQ_ENTRY(env_node) next;
	const char *var;
	const char *val;
    cli_sfunc_t sfunc;
};

struct cli_env {
    TAILQ_HEAD(, env_node) head;     /**< link list of vars */
    int count;
};

/**
 * Create a environment for the cli
 *
 * @return
 *   NULL on error or cli_env pointer
 */
struct cli_env *cli_env_create(void);

/**
 * Delete the environment for the CLI
 *
 * @param cli
 *   The pointer to the enviroment structure
 */
void cli_env_destroy(struct cli_env *env);

/**
 * Set a environment variable for the CLI
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
 * Set a environment variable for the CLI with a function pointer
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
int cli_env_string(struct cli_env *env, const char *var,
			       cli_sfunc_t sfunc, const char *val);

/**
 * Get the environment variable from the cli
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
 * Remove the environment variable from the cli
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
 * Do enviroment variable subsitution on the line.
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
 * Get the number of variables in the environment
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
 * Get all of the environment variables
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
 * Show all enviroment variable
 *
 * @param env
 *   Pointer to the cli_env structure.
 */
void cli_env_show(struct cli_env *env);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_ENV_H_ */
