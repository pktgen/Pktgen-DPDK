/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#include "coremap.h"

static char *model_name;

typedef struct lcore {
	struct lcore *next;
	lc_info_t u;
} lcore_t;

typedef lcore_t *(*do_line_fn)(const char *line, lcore_t *);
typedef unsigned (*getter_fn)(const lcore_t *);
typedef void (*setter_fn)(lcore_t *, unsigned new_val);

typedef struct action {
	const char *desc;
	do_line_fn fn;
} action_t;

static unsigned
get_socket_id(const lcore_t *lc)
{
	return lc->u.s.socket_id;
}

static void
set_socket_id(lcore_t *lc, unsigned v)
{
	lc->u.s.socket_id = v;
}

static unsigned
get_core_id(const lcore_t *lc)
{
	return lc->u.s.core_id;
}

static void
set_core_id(lcore_t *lc, unsigned v)
{
	lc->u.s.core_id = v;
}

static unsigned
get_thread_id(const lcore_t *lc)
{
	return lc->u.s.thread_id;
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
	lcore_t *lc = calloc(1, sizeof *lc);

	lc->next = rest;
	lc->u.s.id   = as_int(line);
	return lc;
}

static lcore_t *
set_raw_socket_id(const char *line, lcore_t *lc)
{
	lc->u.s.socket_id = as_int(line);
	return lc;
}

static lcore_t *
set_raw_core_id(const char *line, lcore_t *lc)
{
	lc->u.s.core_id = as_int(line);
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
	if (lc->u.s.socket_id == socket_id && lc->u.s.core_id == core_id)
		return lc->u.s.thread_id + 1;
	return get_next_thread_id(lc->next, socket_id, core_id);
}

static lcore_t *
set_thread_id_str(const char *unused, lcore_t *lc)
{
	(void)unused;
	lc->u.s.thread_id = get_next_thread_id(lc->next,
					       lc->u.s.socket_id,
					       lc->u.s.core_id);
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
		{ "processor", new_lcore },
		{ "physical id", set_raw_socket_id },
		{ "core id", set_raw_core_id },
		{ "model name", set_model_name },
		{ "\n", set_thread_id_str },
		{ NULL, NULL }
	};
	struct action *action;

	for (action = actions; action->fn != NULL; ++action)
		if (strncmp(action->desc, line, strlen(action->desc)) == 0)
			return action->fn;
	return ignore_line;
}

/*
 * Remaps a property value from 'form' to 'to'. This is done for all
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

/*
 * Returns the number of unique values that exist of the property
 * returned by the function 'get'
 */
static __inline__ unsigned
cnt(const lcore_t *lc, getter_fn get)
{
	unsigned cnt = 0;

	while (lc) {
		if (cnt < get(lc))
			cnt = get(lc);
		lc = lc->next;
	}
	return cnt + 1;
}

static void
print_and_free_lcores(lcore_t *lc)
{
	if (lc) {
		print_and_free_lcores(lc->next);
		printf("%u/%u/%u\t%u\n",
		       lc->u.s.socket_id,
		       lc->u.s.core_id,
		       lc->u.s.thread_id,
		       lc->u.s.id);
		free(lc);
	}
}

static lc_info_t *
get_and_free_lcores(lcore_t *lc, lc_info_t *get, int cnt)
{
	if ((lc == NULL) || (cnt == 0))
		return NULL;

	get_and_free_lcores(lc->next, get + 1, cnt - 1);
	get->word = lc->u.word;
	return get;
}

static void
print_matching_lcores(lcore_t *lc, unsigned s, unsigned c, unsigned t)
{
	if (lc) {
		print_matching_lcores(lc->next, s, c, t);
		if (lc->u.s.socket_id == s && lc->u.s.core_id == c &&
		    lc->u.s.thread_id == t)
			printf("%u ", lc->u.s.id);
	}
}

static __inline__ void
free_lcores(lcore_t *lc)
{
	if (lc) {
		free_lcores(lc->next);
		free(lc);
	}
}

static void
print_core_map(lcore_t *lcores, unsigned mode, unsigned balanced)
{
	unsigned s, c;

	/*
	 * For unpaired mode print HT0 on each physical core.
	 *
	 * For paired mode print lcore for HT1 as well,
	 * except for physical core 0. This is because
	 * the main thread runs on core 0, HT 0.
	 *
	 * For paired mode, add HT1 on physical core 0:socket 0
	 * last, in case the user wants to use all logical cores
	 * in the system.
	 *
	 */

	if (balanced) {
		/*
		 * Balanced map. Print cores in the following order:
		 * Core 0 one socket 0, core 0 on socket 1 etc.
		 */
		for (c = 0; c < cnt(lcores, get_core_id); c++)
			for (s = 0; s < cnt(lcores, get_socket_id); s++) {
				print_matching_lcores(lcores, s, c, 0);

				if (mode == 0 && (c != 0 || s != 0))
					print_matching_lcores(lcores, s, c, 1);
			}
	} else {
		/*
		 * Unbalanced map. Print cores in the following order:
		 * Core 0 one socket 0, core 1 on socket 0...
		 */
		for (s = 0; s < cnt(lcores, get_socket_id); s++)
			for (c = 0; c < cnt(lcores, get_core_id); c++) {
				print_matching_lcores(lcores, s, c, 0);
				if (mode == 0 && (c != 0 || s != 0))
					print_matching_lcores(lcores, s, c, 1);
			}
	}

	if (mode == 0)
		print_matching_lcores(lcores, 0, 0, 1);

	free_lcores(lcores);
	printf("\n");
}

static __inline__ int
count_cores(lcore_t *lcores)
{
	int num = 0;

	while (lcores) {
		lcores = lcores->next;
		num++;
	}
	return num;
}

int
coremap(const char *arg,
	lc_info_t *get_lcores,
	int max_cnt,
	const char *proc_cpuinfo)
{
	FILE         *f;
	char         *line = NULL;
	size_t line_sz = 0;
	unsigned print_map = 0;
	unsigned balanced = 1;
	unsigned mode = 0;
	lcore_t      *lcores = NULL;

	if (proc_cpuinfo == NULL)
		proc_cpuinfo = PROC_CPUINFO;

	if ( (f = fopen(proc_cpuinfo, "r")) == NULL) {
		fprintf(stderr, "Cannot open %s on this system\n",
			proc_cpuinfo);
		return -1;
	}

	while (getline(&line, &line_sz, f) >= 0)
		lcores = get_matching_action(line) (line, lcores);

	if (f) fclose(f);
	if (line) free(line);

	zero_base(lcores, get_socket_id, set_socket_id);
	zero_base(lcores, get_core_id, set_core_id);

	if (strcmp(arg, "info") != 0) {
		print_map = 1;

		if (strcmp(arg, "paired_balanced") == 0) {
			mode = 0; balanced = 1;
		} else if (strcmp(arg, "unpaired_balanced") == 0) {
			mode = 1; balanced = 1;
		} else if (strcmp(arg, "paired_unbalanced") == 0) {
			mode = 0; balanced = 0;
		} else if (strcmp(arg, "unpaired_unbalanced") == 0) {
			mode = 1; balanced = 0;
		} else if (strcmp(arg, "array") == 0) {
			int num_cores = count_cores(lcores);
			if ( (get_lcores != NULL) || (max_cnt > 0) )
				get_and_free_lcores(lcores,
						    &get_lcores[0],
						    max_cnt);
			return num_cores;
		}
	}
	if (print_map) {
		print_core_map(lcores, mode, balanced);
		return 0;
	}

	printf("CPU : %s", model_name);
	printf("%u socket%s, %u core%s per socket and %u thread%s per core\n",
	       cnt(lcores, get_socket_id),
	       cnt(lcores, get_socket_id) > 1 ? "s" : "",
	       cnt(lcores, get_core_id),
	       cnt(lcores, get_core_id) > 1 ? "s" : "",
	       cnt(lcores, get_thread_id),
	       cnt(lcores, get_thread_id) > 1 ? "s" : "");
	print_and_free_lcores(lcores);

	return 0;
}

typedef unsigned (*_getter_fn)(const lc_info_t *);

static unsigned
_get_socket_id(const lc_info_t *lc)
{
	return lc->s.socket_id;
}

static unsigned
_get_core_id(const lc_info_t *lc)
{
	return lc->s.core_id;
}

static unsigned
_get_thread_id(const lc_info_t *lc)
{
	return lc->s.thread_id;
}

static unsigned
_get_lcore_id(const lc_info_t *lc)
{
	return lc->s.id;
}

/*
 * Returns the number of unique values that exist of the property
 * returned by the function 'get'
 */
unsigned
coremap_cnt(const lc_info_t *lc, unsigned max_cnt, unsigned t)
{
	_getter_fn get;
	_getter_fn _type[] =
	{ _get_socket_id, _get_core_id, _get_thread_id, _get_lcore_id, NULL };
	unsigned cnt = 0, i;

	get = _type[t];
	for (i = 0; i < max_cnt; i++, lc++)
		if (cnt < get(lc))
			cnt = get(lc);
	return cnt + 1;
}
