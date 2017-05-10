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

#include "cli.h"

int
cli_help_add(const char *group, struct cli_map *map, const char **help_data)
{
	struct help_node *node;

	if (!group)
		return -1;

	node = malloc(sizeof(struct help_node));
	if (!node)
		return -1;

	memset(node, 0, sizeof(struct help_node));

	node->map = map;
	node->help_data = help_data;
	snprintf(node->group, sizeof(node->group), "%s", group);

	TAILQ_INSERT_TAIL(&this_cli->help_nodes, node, next);

	return 0;
}

static int
_show_help_lines(const char **h, int allow_pause)
{
	int j;
	char key;

	for (j = 0; h[j] != NULL; j++) {
		if (!strcmp(h[j], CLI_HELP_PAUSE)) {
			if (allow_pause) {
				key = cli_pause("\n   <Press Return to Continue or ESC>", NULL);
				if ((key == vt100_escape) ||
				    (key == 'q') || (key == 'Q'))
					return -1;
			}
			continue;
		}
		cli_printf("%s\n", h[j]);
	}

	return 0;
}

static void
_cli_help_title(const char *msg)
{
	scrn_pos(1,1);
	scrn_cls();

	if (msg)
		scrn_cprintf(1, -1, "%s\n", msg);
}

int
cli_help_show_all(const char *msg)
{
	struct help_node *n;

	_cli_help_title(msg);

	TAILQ_FOREACH(n, &this_cli->help_nodes, next) {
		if (_show_help_lines(n->help_data, 1))
			return -1;
		_cli_help_title(msg);
	}

	return 0;
}


struct help_node *
cli_help_find_group(const char *group)
{
	struct help_node *n;

	TAILQ_FOREACH(n, &this_cli->help_nodes, next) {
		if (!strcmp(group, n->group))
			return n;
	}
	return NULL;
}

int
cli_help_show_group(const char *group)
{
	struct help_node *n;

	n = cli_help_find_group(group);
	if (!n)
		return -1;

	return _show_help_lines(n->help_data, 0);
}
