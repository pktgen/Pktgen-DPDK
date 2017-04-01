/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CLI_HELP_H_
#define _CLI_HELP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdint.h>
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

#ifdef __cplusplus
}
#endif

#endif /* _CLI_HELP_H_ */
