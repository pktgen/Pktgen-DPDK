/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2016-2025>, Intel Corporation.
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
 * CLI help subsystem for grouped command documentation.
 *
 * Manages named help groups, each consisting of a cli_map table and an array
 * of descriptive strings. Groups are registered via cli_help_add() and can be
 * displayed individually or all at once.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_HELP_PAUSE    "<<PauseOutput>>"
#define CLI_HELP_NAME_LEN 32

struct help_node {
    TAILQ_ENTRY(help_node) next; /**< link list of help nodes */
    char group[CLI_HELP_NAME_LEN];
    struct cli_map *map;
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
struct help_node *cli_help_find_group(const char *group);

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
int cli_help_show_group(const char *group);

/**
 * Register a named help group with an optional command map.
 *
 * Adds the group to the per-lcore help list and registers all first-token
 * command names from @p map so that auto-complete can find them.
 *
 * @param group
 *   Name string for this help group (e.g. "Range", "Set").
 * @param map
 *   Pointer to the cli_map table for this group, or NULL if none.
 * @param hd
 *   NULL-terminated array of help strings for this group.
 * @return
 *   0 on success or -1 on error
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

/**
 * Print an error message and show the help group for a failed command.
 *
 * @param msg
 *   Short error description to print.
 * @param group
 *   Help group name whose entries will be displayed after the error.
 * @param argc
 *   Number of arguments in @p argv (for context in the error message).
 * @param argv
 *   Argument strings from the failed command invocation.
 * @return
 *   Always returns -1 (for use as a one-liner return value).
 */
int cli_cmd_error(const char *msg, const char *group, int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* _CLI_HELP_H_ */
