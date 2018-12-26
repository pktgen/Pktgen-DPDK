/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2014 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

#include "utils.h"
#include "lscpu.h"

static lscpu_t  *lscpu;

static __inline__ void
num_cpus(__attribute__((unused)) action_t *act, char *line) {
	lscpu->num_cpus = atoi(line);
}

static __inline__ void
threads_per_core(__attribute__((unused)) action_t *act, char *line) {
	lscpu->threads_per_core = atoi(line);
}

static __inline__ void
cores_per_socket(__attribute__((unused)) action_t *act, char *line) {
	lscpu->cores_per_socket = atoi(line);
}

static __inline__ void
numa_nodes(__attribute__((unused)) action_t *act, char *line) {
	lscpu->numa_nodes = atoi(line);
}

static __inline__ void
cpu_mhz(action_t *act, char *line)
{
	if ( (act->flags & ONLY_ONCE_FLAG) == 0) {
		lscpu->cpu_mhz = pg_strdupf(lscpu->cpu_mhz, line);
		act->flags |= ONLY_ONCE_FLAG;
	}
}

static void
numa_nodeX_cpus(action_t *act, char *line)
{
	int n, i;
	char    *arr[32], *p;
	int first, last;

	memset(arr, 0, sizeof(arr));

	n = pg_strparse(line, ",", arr, (sizeof(arr) / sizeof(char *)));
	if (n > 0)
		for (i = 0; i < n; i++) {
			if (arr[i] == NULL)
				continue;
			if ( (p = strchr(arr[i], '-')) )
				p++;	/* Point to second port number. */
			else
				p = arr[i];	/* Only one port number, make the same port. */
			first = atoi(arr[i]);
			last = atoi(p);
			while (first <= last)
				lscpu->numa_cpus[act->arg][first++] = 1;
		}
}

static __inline__ void
cache_size(action_t *act, char *line)
{
	if ( (act->flags & ONLY_ONCE_FLAG) == 0) {
		lscpu->cache_size = pg_strdupf(lscpu->cache_size, line);
		act->flags |= ONLY_ONCE_FLAG;
	}
}

static __inline__ void
model_name(action_t *act, char *line)
{
	if ( (act->flags & ONLY_ONCE_FLAG) == 0) {
		lscpu->model_name = pg_strdupf(lscpu->model_name, line);
		act->flags |= ONLY_ONCE_FLAG;
	}
}

static __inline__ void
cpu_flags(action_t *act, char *line)
{
	if ( (act->flags & ONLY_ONCE_FLAG) == 0) {
		lscpu->cpu_flags = pg_strdupf(lscpu->cpu_flags, line);
		act->flags |= ONLY_ONCE_FLAG;
	}
}

static action_t *
lscpu_match_action(char *line)
{
	static action_t actions[] = {
		{ "CPU(s)",                 num_cpus, 0, 0 },
		{ "Thread(s) per core",     threads_per_core, 0, 0 },
		{ "Core(s) per socket",     cores_per_socket, 0, 0 },
		{ "NUMA node(s)",           numa_nodes, 0, 0 },
		{ "CPU MHz",                cpu_mhz, 0, 0 },
		{ "NUMA node0 CPU(s)",      numa_nodeX_cpus, 0, 0 },
		{ "NUMA node1 CPU(s)",      numa_nodeX_cpus, 1, 0 },
		{ "NUMA node2 CPU(s)",      numa_nodeX_cpus, 2, 0 },
		{ "NUMA node3 CPU(s)",      numa_nodeX_cpus, 3, 0 },
		{ NULL, NULL, 0, 0 }
	};
	action_t *act;

	for (act = actions; act->str != NULL; ++act)
		if (strncmp(act->str, line, strlen(act->str)) == 0)
			break;

	return act;
}

static action_t *
cpu_proc_match_action(char *line)
{
	static action_t actions[] = {
		{ "cache size",             cache_size, 0, 0 },
		{ "model name",             model_name, 0, 0 },
		{ "flags",                  cpu_flags, 0, 0 },
		{ NULL, NULL, 0, 0 }
	};
	action_t *act;

	for (act = actions; act->str != NULL; ++act)
		if (strncmp(act->str, line, strlen(act->str)) == 0)
			break;

	return act;
}

static void
lscpu_info_get(const char *lscpu_path)
{
	FILE    *f = popen(lscpu_path, "r");
	char    *line = NULL, *p;
	action_t    *act;
	size_t line_size;

	if (f == NULL) {
		printf("Unable to run 'lscpu' command\n");
		return;
	}

	line_size = 0;
	while (getline(&line, &line_size, f) > 0) {
		line[strlen(line) - 1] = '\0';
		act = lscpu_match_action(line);
		if (act->str) {
			p = strchr(line, ':');
			if (p) p++;
			act->func(act, pg_strtrim(p));
		}
	}

	fclose(f);
	free(line);
}

static void
cpu_proc_info(const char *proc_path)
{
	FILE    *f = popen(proc_path, "r");
	char    *line = NULL, *p;
	action_t    *act;
	size_t line_size;

	if (f == NULL) {
		printf("Unable to run 'CPU proc' command\n");
		return;
	}

	line_size = 0;
	while (getline(&line, &line_size, f) > 0) {
		line[strlen(line) - 1] = '\0';
		act = cpu_proc_match_action(line);
		if (act->str) {
			p = strchr(line, ':');
			if (p) p++;
			act->func(act, pg_strtrim(p));
		}
	}

	fclose(f);
	free(line);
}

lscpu_t *
lscpu_info(const char *lscpu_path, const char *proc_path)
{
	if (lscpu == NULL)
		lscpu = calloc(1, sizeof(lscpu_t));

	if (lscpu_path == NULL)
		lscpu_path = LSCPU_PATH;
	if (proc_path == NULL)
		proc_path = CPU_PROC_PATH;

	lscpu_info_get(lscpu_path);
	cpu_proc_info(proc_path);

	return lscpu;
}
