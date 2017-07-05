
/**
 * Copyright (c) <2010-2017>, Wind River Systems, Inc. All rights reserved.
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
/*
 * Prints a CPU core map on the form
 * "S/C/T L"
 * where
 * - S is the CPU socket ID
 * - C is the physical CPU core ID
 * - T is the hyper-thread ID
 * - L is the logical core ID
 *
 * This tool parses the information from "/proc/cpuinfo" which should
 * be present on all Linux systems.
 *
 * NOTE: this tool has only been tested on systems with x86/x86_64
 * CPUs so far.
 *
 * Written 2011 by Kenneth Jonsson, WindRiver.
 * Adapted to DPDK by Keith Wiles, WindRiver 2013-01-08
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "cli_cmap.h"

static char *model_name;

char *
cmap_cpu_model(void)
{
	return model_name;
}

static const char *
as_str(const char *line)
{
	if (*line != ':')
		return as_str(line + 1);
	return line + 1;
}

static unsigned
as_int(const char *line)
{
	return atoi(as_str(line));
}

static lcore_t *
new_lcore(const char *line, lcore_t *rest)
{
	lcore_t *lc = calloc(1, sizeof(lcore_t));

	lc->next  = rest;
	lc->u.lid  = as_int(line);

	return lc;
}

static lcore_t *
set_raw_socket_id(const char *line, lcore_t *lc)
{
	lc->u.sid = as_int(line);
	return lc;
}

static lcore_t *
set_raw_core_id(const char *line, lcore_t *lc)
{
	lc->u.cid = as_int(line);
	return lc;
}

static lcore_t *
set_model_name(const char *line, lcore_t *lc)
{
	if (!model_name)
		model_name = strdup(as_str(line));
	return lc;
}

static unsigned
get_next_thread_id(const lcore_t *lc, unsigned socket_id, unsigned core_id)
{
	if (lc == NULL)
		return 0;
	if (lc->u.sid == socket_id && lc->u.cid == core_id)
		return lc->u.tid + 1;
	return get_next_thread_id(lc->next, socket_id, core_id);
}

static lcore_t *
set_thread_id_str(const char *unused, lcore_t *lc)
{
	(void)unused;
	lc->u.tid = get_next_thread_id(lc->next, lc->u.sid, lc->u.cid);
	return lc;
}

static lcore_t *
ignore_line(const char *unused, lcore_t *lc)
{
	(void)unused;
	return lc;
}

static do_line_fn
get_matching_action(const char *line)
{
	static struct action actions[] = {
		{ "processor",      new_lcore },
		{ "physical id",    set_raw_socket_id },
		{ "core id",        set_raw_core_id },
		{ "model name",     set_model_name },
		{ "\n",             set_thread_id_str },
		{ NULL, NULL }
	};
	struct action *action;

	for (action = actions; action->fn != NULL; ++action)
		if (strncmp(action->desc, line, strlen(action->desc)) == 0)
			return action->fn;

	return ignore_line;
}

/*
 * Remaps a property value from 'from' to 'to'. This is done for all
 * logical cores.
 */
static void
remap(lcore_t *lc,
      unsigned from,
      unsigned to,
      getter_fn get,
      setter_fn set)
{
	if (lc) {
		if (get(lc) == from)
			set(lc, to);

		remap(lc->next, from, to, get, set);
	}
}

/*
 * Returns the first entry that is equal to or as close as possible to
 * 'v' in the property returned by 'get'.
 */
static lcore_t *
closest_gte(lcore_t *lc, lcore_t *sel, unsigned v, getter_fn get)
{
	if (lc == NULL)
		return sel;

	if (get(lc) >= v && (sel == NULL || get(sel) - v > get(lc) - v))
		return closest_gte(lc->next, lc, v, get);

	return closest_gte(lc->next, sel, v, get);
}

/*
 * Makes the property returned and set by 'get'/'set' start from zero
 * and increase by one for each unique value that propery has.
 * Ex: core id "0,1,4,5,0,1,4,5" -> "0,1,2,3,0,1,2,3"
 */
static void
zero_base(lcore_t *head, getter_fn get, setter_fn set)
{
	unsigned id = 0;
	lcore_t *lc;

	while ((lc = closest_gte(head, NULL, id, get)) != NULL) {
		remap(lc, get(lc), id, get, set);
		++id;
	}
}

static void
get_and_free_lcore_info(lcore_t *lc, lc_info_t *get)
{
	if (lc) {
		get_and_free_lcore_info(lc->next, get + 1);
		get->word = lc->u.word;
		free(lc);
	}
}

static inline int
count_cores(lcore_t *lcores)
{
	int num = 0;

	while (lcores) {
		lcores = lcores->next;
		num++;
	}
	return num;
}

static int
my_getline(char **line, size_t *line_sz, int fd)
{
	char *l, c;
	size_t sz, i;

	if (*line == NULL) {
		if (*line_sz == 0)
			*line_sz = MAX_LINE_SIZE;
		l = malloc(*line_sz);
		if (l == NULL)
			return -1;
		*line = l;
	} else
		l = *line;

	for (i = 0, sz = 0; i < *line_sz; i++) {
		if (read(fd, &c, 1) != 1)
			return -1;
		*l++ = c;
		sz++;
		if (c == '\n')
			break;
	}
	*l = '\0';
	return sz;
}

#define MAX_CNT     256
static lc_info_t lcore_info[MAX_CNT];

struct cmap *
cmap_create(void)
{
	int fd;
	char         *line = NULL;
	struct cmap *cmap;
	lc_info_t *lc_info = &lcore_info[0];
	size_t line_sz = 0;
	lcore_t *lcores = NULL;

	memset(lcore_info, '\0', sizeof(lcore_info));

	cmap = malloc(sizeof(struct cmap));
	if (!cmap)
		return NULL;

	if ( (fd = open(PROC_CPUINFO, O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot open %s on this system\n", PROC_CPUINFO);
		free(cmap);
		return NULL;
	}

	while (my_getline(&line, &line_sz, fd) >= 0)
		lcores = get_matching_action(line) (line, lcores);

	if (fd) close(fd);
	if (line) free(line);

	zero_base(lcores, cmap_socket_id, cmap_set_socket_id);
	zero_base(lcores, cmap_core_id, cmap_set_core_id);

	cmap->linfo = lc_info;

	cmap->model = model_name;
	cmap->num_cores = count_cores(lcores);
	cmap->sid_cnt = cmap_cnt(lcores, cmap_socket_id);
	cmap->cid_cnt = cmap_cnt(lcores, cmap_core_id);
	cmap->tid_cnt = cmap_cnt(lcores, cmap_thread_id);

	get_and_free_lcore_info(lcores, lc_info);

	return cmap;
}

void
cmap_free(struct cmap *cmap)
{
	free(cmap);
}

/* Helper for building log strings.
 * The macro takes an existing string, a printf-like format string and optional
 * arguments. It formats the string and appends it to the existing string, while
 * avoiding possible buffer overruns.
 */
#define strncatf(dest, fmt, ...) do {					\
		char _buff[1024];					\
		snprintf(_buff, sizeof(_buff), fmt, ## __VA_ARGS__);	\
		strncat(dest, _buff, sizeof(dest) - strlen(dest) - 1);	\
} while (0)

static __inline__ uint8_t
sct(struct cmap *cm, uint8_t s, uint8_t c, uint8_t t) {
	lc_info_t   *lc = cm->linfo;
	uint8_t i;

	for (i = 0; i < cm->num_cores; i++, lc++)
		if (lc->sid == s && lc->cid == c && lc->tid == t)
			return lc->lid;

	return 0;
}

void
cmap_dump(FILE *f)
{
	struct cmap *c = cmap_create();
	int i;

	if (!f)
		f = stdout;

	fprintf(f, "CPU : %s", c->model);
	fprintf(f, "      %d lcores, %u socket%s, %u core%s per socket and "
		   "%u thread%s per core\n",
		c->num_cores,
		c->sid_cnt, c->sid_cnt > 1 ? "s" : "",
		c->cid_cnt, c->cid_cnt > 1 ? "s" : "",
		c->tid_cnt, c->tid_cnt > 1 ? "s" : "");

	fprintf(f, "Socket     : ");
	for (i = 0; i < c->sid_cnt; i++)
		fprintf(f, "%4d      ", i);

	for (i = 0; i < c->cid_cnt; i++) {
		fprintf(f, "  Core %3d : [%2d,%2d]   ", i,
			sct(c, 0, i, 0), sct(c, 0, i, 1));
		if (c->sid_cnt > 1)
			fprintf(f, "[%2d,%2d]   ",
				sct(c, 1, i, 0), sct(c, 1, i, 1));
		if (c->sid_cnt > 2)
			fprintf(f, "[%2d,%2d]   ",
				sct(c, 2, i, 0), sct(c, 2, i, 1));
		if (c->sid_cnt > 3)
			fprintf(f, "[%2d,%2d]   ",
				sct(c, 3, i, 0), sct(c, 3, i, 1));
		fprintf(f, "\n");
	}
	cmap_free(c);
}
