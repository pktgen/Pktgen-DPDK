/*-
 *   Copyright(c) 2015-2016 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <netinet/in.h>
#ifndef __linux__
#include <net/socket.h>
#endif

#include <rte_common.h>

#include "cmdline_parse.h"
#include "cmdline_parse_args.h"

struct cmdline_token_ops cmdline_token_args_ops = {
	.parse              = cmdline_parse_args,
	.complete_get_nb    = NULL,
	.complete_get_elt   = NULL,
	.get_help           = cmdline_get_help_args,
};

static char orig_cmdline[256];

void
cmdline_args_free(int argc, char **argv)
{
	int i;

	if (argc <= 0)
		return;

	for (i = 0; i < argc; i++)
		if (argv[i])
			free(argv[i]);
}

int
cmdline_parse_args(cmdline_parse_token_hdr_t *tk __rte_unused,
		   const char *buf, void *res, unsigned tk_len __rte_unused)
{
	unsigned int token_len = 0, len = 0;
	char args_str[XARGS_TOKEN_SIZE + 1];
	cmdline_args_t  *pl;

	if (!buf)
		buf = " ";

	if (res == NULL)
		return -1;

	pl          = res;
	pl->argc    = 1;	/* Leave the zero entry empty */

	snprintf(orig_cmdline, sizeof(orig_cmdline), "%s", buf);
	do {
		while (!cmdline_isendoftoken(buf[token_len]) &&
		       (token_len < XARGS_TOKEN_SIZE)) {
			token_len++;
			len++;
		}
		if (token_len == 0)
			break;

		if (token_len >= XARGS_TOKEN_SIZE)
			return -1;

		snprintf(args_str, token_len + 1, "%s", buf);
		buf += token_len;
		if (*buf == ' ') {
			buf++;
			len++;
		}
		token_len = 0;

		pl->argv[pl->argc++] = strdup(args_str);
	} while ( (*buf != '\n') && (pl->argc < XARGS_MAX_TOKENS) );

	pl->argv[pl->argc] = NULL;
	pl->cmdline = orig_cmdline;

	return len;
}

int
cmdline_get_help_args(__rte_unused cmdline_parse_token_hdr_t *tk,
		      char *dstbuf, unsigned int size)
{
	int ret;

	ret = snprintf(dstbuf, size, "argc/argv list of arguments");
	if (ret < 0)
		return -1;
	return 0;
}
