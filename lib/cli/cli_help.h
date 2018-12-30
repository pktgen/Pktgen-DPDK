/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2019> Intel Corporation.
 */

#ifndef _CLI_HELP_H_
#define _CLI_HELP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/queue.h>

/**
 * @file
 * RTE Command line interface
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_HELP_PAUSE		"<<PauseOutput>>"
#define CLI_HELP_NAME_LEN	32

struct help_node {
	TAILQ_ENTRY(help_node) next;     /**< link list of help nodes */
	char group[CLI_HELP_NAME_LEN];
	struct cli_map	*map;
	const char **help_data;
};

/**
 * Find the help group section defined by the group string.
 *
 * @note Uses thread variable this_cli.
 *
 * @param group
 *   The group string name to find.
 * @return
 *   NULL if not found or pointer to struct cli_info.
 */
struct help_node *
cli_help_find_group(const char *group);

/**
 * Show the map table entries
 *
 * @param msg
 *   Pointer to a message to print first.
 * @return
 *   0 on success or -1 on error
 */
int cli_help_show_all(const char *msg);

/**
 * Show the help message for the user.
 *
 * @note Uses thread variable this_cli.
 *
 * @param data
 *   Pointer to the cli_info structure.
 */
int cli_help_show_group( const char *group);

/**
 * Add help string group to a help structure
 *
 * @param
 *   The help pointer structure
 * @param group
 *   The group name for this help list
 * @param map
 *   The pointer to the MAP structure if present.
 * @param help_data
 *   The array of string pointers for the help group
 * @returns
 *   0 on OK and -1 on error
 */
int cli_help_add(const char *group, struct cli_map *map, const char **hd);

/**
 * Find if the last item is a help request.
 *
 * @param argc
 *   Number of args in the argv list.
 * @param argv
 *   List of strings to parser
 * @return
 *   1 if true or 0 if false
 */
static inline int
is_help(int argc, char **argv)
{
	if (argc == 0)
		return 0;

	return !strcmp("-?", argv[argc - 1]) || !strcmp("?", argv[argc - 1]);
}

/**
 * Iterate over the help messages calling a given function.
 *
 * @param func
 *   A function to call for all help lines.
 * @param arg
 *   Argument pointer for function call.
 * @return
 *   N/A
 */
void cli_help_foreach(void (*func)(void *arg, const char **h), void *arg);

int cli_cmd_error(const char * msg, const char *group, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_HELP_H_ */
