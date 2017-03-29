/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
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

/*
 * Copyright (c) 2009, Olivier MATZ <zer0@droids-corp.org>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cli.h"
#include "cli_vt100.h"
#include "cli_vt100_keys.h"

static int
vt100_find_cmd(char *buf, unsigned int size)
{
	struct vt100_cmds *cmd;
	size_t cmdlen;
	int i;

	for (i = 0, cmd = vt100_cmd_list; cmd->str; cmd++, i++) {
		cmdlen = strnlen(cmd->str, VT100_BUF_SIZE);
		if ((size == cmdlen) && !strncmp(buf, cmd->str, cmdlen))
			return i;
	}

	return VT100_DONE;
}

int
vt100_parse_input(struct cli_vt100 *vt, uint8_t c)
{
	uint32_t size;

	RTE_ASSERT(vt != NULL);

	if ((vt->bufpos == VT100_INITIALIZE) ||
	    (vt->bufpos >= VT100_BUF_SIZE)) {
		vt->state = VT100_INIT;
		vt->bufpos = 0;
	}

	vt->buf[vt->bufpos++] = c;
	size = vt->bufpos;

	switch (vt->state) {
	case VT100_INIT:
		if (c == vt100_escape)
			vt->state = VT100_ESCAPE;
		else {
			vt->bufpos = VT100_INITIALIZE;
			return vt100_find_cmd(vt->buf, size);
		}
		break;

	case VT100_ESCAPE:
		if (c == vt100_open_square)
			vt->state = VT100_ESCAPE_CSI;
		else if (c >= '0' && c <= vt100_del) {
			vt->bufpos = VT100_INITIALIZE;
			return vt100_find_cmd(vt->buf, size);
		}
		break;

	case VT100_ESCAPE_CSI:
		if (c >= '@' && c <= '~') {
			vt->bufpos = VT100_INITIALIZE;
			return vt100_find_cmd(vt->buf, size);
		}
		break;

	default:
		vt->bufpos = VT100_INITIALIZE;
		break;
	}

	return VT100_CONTINUE;
}

struct cli_vt100 *
vt100_create(void)
{
	struct cli_vt100 *vt;

	vt = calloc(1, sizeof(struct cli_vt100));
	if (!vt)
		return NULL;

	vt->bufpos = -1;

	return vt;
}

void
vt100_destroy(struct cli_vt100 *vt)
{
	if (vt)
		free(vt);
}
