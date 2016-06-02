/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2015-2016 Intel Corporation. All rights reserved.
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
    int     argc;
    char    *argv[XARGS_MAX_TOKENS + 1];
} cmdline_args_t;

struct cmdline_token_args {
    struct cmdline_token_hdr    hdr;
    struct cmdline_args         args;
};

typedef struct cmdline_token_args cmdline_parse_token_args_t;

extern struct cmdline_token_ops cmdline_token_args_ops;

int cmdline_parse_args(cmdline_parse_token_hdr_t *tk,
                       const char *srcbuf, void *res, unsigned tk_len);
int cmdline_get_help_args(cmdline_parse_token_hdr_t *tk,
                          char *dstbuf, unsigned int size);

#define TOKEN_ARGS_INITIALIZER(structure, field)                        \
    {                                                           \
        /* hdr */                                               \
        {                                                       \
            &cmdline_token_args_ops,    /* ops */           \
            offsetof(structure, field), /* offset */        \
        },                                                      \
        /* args */                                                      \
        {                                                       \
            0,                                                                                      \
        },                                                      \
    }

extern void cmdline_args_free(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif                                      /* _PARSE_XARGS_H_ */
