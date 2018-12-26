/*-
 * Copyright(c) 2016-2017 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cli.h"
#include "cli_input.h"

int
cli_help_add(const
char *group, struct cli_map *map, const char **help_data)
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
		if (strcmp(h[j], CLI_HELP_PAUSE)) {
			cli_printf("%s\n", h[j]);
			continue;
		}
		if (allow_pause) {
			key = cli_pause("\n  Return to Continue or ESC:", NULL);
			if ((key == vt100_escape) ||
			    (key == 'q') || (key == 'Q'))
				return -1;
		}
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

void
cli_help_foreach(void (*func)(void *arg, const char **h), void *arg)
{
	struct help_node *n;

	TAILQ_FOREACH(n, &this_cli->help_nodes, next) {
		func(arg, n->help_data);
	}
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

int
cli_cmd_error(const char * msg, const char *group, int argc, char **argv)
{
        int n;

        if (group)
                cli_help_show_group(group);
        if (msg)
                cli_printf("%s:\n", msg);

        cli_printf("  Invalid line: <");
        for(n = 0; n < argc; n++)
                cli_printf("%s ", argv[n]);
        cli_printf(">\n");

        return -1;
}
