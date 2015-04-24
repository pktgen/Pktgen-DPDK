/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <rte_config.h>
#include <rte_atomic.h>
#include <rte_malloc.h>
#include <rte_spinlock.h>

#include "wr_scrn.h"

	wr_scrn_t	* __scrn;		/**< Global screen structure pointer, their can be only one */

void
wr_scrn_center(int16_t r, int16_t ncols, const char * fmt, ...)
{
	va_list	vaList;
	char	str[512];

	if ( ncols == -1 )
		ncols = __scrn->ncols;
	va_start(vaList, fmt);
	vsnprintf(str, sizeof(str), fmt, vaList);
	va_end(vaList);
	wr_scrn_pos(r, wr_scrn_center_col(ncols, str));
	printf("%s", str);
	fflush(stdout);
}

void
wr_scrn_printf(int16_t r, int16_t c, const char * fmt, ...)
{
	va_list	vaList;
	
	if ( (r != 0) && (c != 0) )
		wr_scrn_pos(r, c);
	va_start(vaList, fmt);
	vprintf(fmt, vaList);
	va_end(vaList);
	fflush(stdout);
}

void
wr_scrn_fprintf(int16_t r, int16_t c, FILE * f, const char * fmt, ...)
{
	va_list	vaList;

	if ( (r != 0) && (c != 0) )
		wr_scrn_pos(r, c);
	va_start(vaList, fmt);
	vfprintf(f, fmt, vaList);
	va_end(vaList);
	fflush(f);
}

wr_scrn_t *
wr_scrn_init(int16_t nrows, int16_t ncols, int theme)
{
	wr_scrn_t *	scrn;

	if ( __scrn != NULL ) {
		free(__scrn);
		__scrn = NULL;
	}

	scrn = malloc(sizeof(wr_scrn_t));
	if ( scrn  ) {
		rte_atomic32_set(&scrn->pause, SCRN_PAUSED);

		scrn->nrows		= nrows;
		scrn->ncols		= ncols;
		scrn->theme		= theme;
		wr_scrn_color(DEFAULT_FG, DEFAULT_BG, OFF);

		wr_scrn_erase(nrows);
	}

	/* Save the global wr_scrn_t pointer */
	__scrn = scrn;

	return scrn;
}
