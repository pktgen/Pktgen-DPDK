/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2017 Intel Corporation. All rights reserved.
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

/* Created 2017 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <rte_config.h>
#include <rte_atomic.h>
#include <rte_malloc.h>
#include <rte_spinlock.h>

#include "cli_scrn.h"

RTE_DEFINE_PER_LCORE(struct cli_scrn *, scrn);

void
scrn_printf(int16_t r, int16_t c, const char *fmt, ...)
{
	va_list vaList;

	if ( (r != 0) && (c != 0) )
		scrn_pos(r, c);
	va_start(vaList, fmt);
	vfprintf(this_scrn->fd_out, fmt, vaList);
	va_end(vaList);
	fflush(this_scrn->fd_out);
}

void
scrn_cprintf(int16_t r, int16_t ncols, const char *fmt, ...)
{
	va_list vaList;
	char str[512];

	if (ncols == -1)
		ncols = this_scrn->ncols;

	va_start(vaList, fmt);
	vsnprintf(str, sizeof(str), fmt, vaList);
	va_end(vaList);

	scrn_pos(r, scrn_center_col(ncols, str));
	scrn_puts("%s", str);
}

void
scrn_fprintf(int16_t r, int16_t c, FILE *f, const char *fmt, ...)
{
	va_list vaList;

	if ( (r != 0) && (c != 0) )
		scrn_pos(r, c);
	va_start(vaList, fmt);
	vfprintf(f, fmt, vaList);
	va_end(vaList);
	fflush(f);
}

static void
scrn_set_io(FILE *in, FILE *out)
{
	struct cli_scrn *scrn = this_scrn;

	if (scrn) {
		if (scrn->fd_in && (scrn->fd_in != stdin))
			fclose(scrn->fd_in);

		if (scrn->fd_out && (scrn->fd_out != stdout))
			fclose(scrn->fd_in);

		scrn->fd_in = in;
		scrn->fd_out = out;
	}
}

static int
scrn_stdin_setup(void)
{
	struct cli_scrn *scrn = this_scrn;
	struct termios term;

	if (!scrn)
		return -1;

	scrn_set_io(stdin, stdout);

	tcgetattr(fileno(scrn->fd_in), &scrn->oldterm);
	memcpy(&term, &scrn->oldterm, sizeof(term));

	term.c_lflag &= ~(ICANON | ECHO | ISIG);
	tcsetattr(0, TCSANOW, &term);
	setbuf(scrn->fd_in, NULL);

	return 0;
}

static void
scrn_stdin_restore(void)
{
	struct cli_scrn *scrn = this_scrn;

	if (!scrn || !scrn->fd_in)
		return;

	tcsetattr(fileno(scrn->fd_in), TCSANOW, &scrn->oldterm);
}

int
scrn_create(int scrn_type, int16_t nrows, int16_t ncols, int theme)
{
	struct cli_scrn *scrn = this_scrn;

	if (!scrn) {
		scrn = malloc(sizeof(struct cli_scrn));
		if (!scrn)
			return -1;
		memset(scrn, 0, sizeof(struct cli_scrn));

		this_scrn = scrn;
	}

	rte_atomic32_set(&scrn->pause, SCRN_SCRN_PAUSED);

	scrn->nrows = nrows;
	scrn->ncols = ncols;
	scrn->theme = theme;
	scrn_type   = scrn_type;

	if (scrn_type == SCRN_STDIN_TYPE) {
		if (scrn_stdin_setup()) {
			free(scrn);
			return -1;
		}
	} else {
		free(scrn);
		return -1;
	}

	scrn_color(SCRN_DEFAULT_FG, SCRN_DEFAULT_BG, SCRN_OFF);

	scrn_erase(nrows);

	return 0;
}

int
scrn_create_with_defaults(int theme)
{
	return scrn_create(SCRN_STDIN_TYPE,
					   SCRN_DEFAULT_ROWS, SCRN_DEFAULT_COLS,
					   (theme)? SCRN_THEME_ON : SCRN_THEME_OFF);
}

void
scrn_destroy(void)
{
	if (this_scrn && (this_scrn->type == SCRN_STDIN_TYPE))
		scrn_stdin_restore();
}
