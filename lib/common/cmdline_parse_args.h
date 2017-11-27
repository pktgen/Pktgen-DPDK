/*-
 *   Copyright(c) 2015-2016 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PARSE_XARGS_H_
#define _PARSE_XARGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* size of a parsed string */
#define XARGS_TOKEN_SIZE        256
#define XARGS_MAX_TOKENS        32

typedef struct cmdline_args {
	char   *cmdline;
	int argc;
	char    *argv[XARGS_MAX_TOKENS + 1];
} cmdline_args_t;

struct cmdline_token_args {
	struct cmdline_token_hdr hdr;
	struct cmdline_args args;
};

typedef struct cmdline_token_args cmdline_parse_token_args_t;

extern struct cmdline_token_ops cmdline_token_args_ops;

int cmdline_parse_args(cmdline_parse_token_hdr_t *tk,
		       const char *srcbuf, void *res, unsigned tk_len);
int cmdline_get_help_args(cmdline_parse_token_hdr_t *tk,
			  char *dstbuf, unsigned int size);

#define TOKEN_ARGS_INITIALIZER(structure, field)			\
	{							    \
		/* hdr */						\
		{							\
			&cmdline_token_args_ops,	/* ops */	    \
			offsetof(structure, field),	/* offset */	    \
		},							\
		/* args */							\
		{							\
			0,											\
		},							\
	}

void cmdline_args_free(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif                                      /* _PARSE_XARGS_H_ */
