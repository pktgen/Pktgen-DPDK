/*-
 * Copyright (c) <2010>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014-2018 by Keith Wiles @ intel.com */

#ifndef __LSCPU_H
#define __LSCPU_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct action_s {
	const char  *str;
	void (*func)(struct action_s *action, char *line);
	int arg;
	int flags;
} action_t;

#define ONLY_ONCE_FLAG  0x0001

#define MAX_LINE_SIZE   1024

typedef struct {
	int num_cpus;
	int threads_per_core;
	int cores_per_socket;
	int numa_nodes;
	char      *cpu_mhz;
	char      *model_name;
	char      *cpu_flags;
	char      *cache_size;
	/* char	  * dummy; */
	short numa_cpus[RTE_MAX_NUMA_NODES][RTE_MAX_LCORE];
} lscpu_t;

#define LSCPU_PATH      "/usr/bin/lscpu"
#define CPU_PROC_PATH   "cat /proc/cpuinfo"

lscpu_t *lscpu_info(const char *lscpu_path, const char *proc_path);

#ifdef __cplusplus
}
#endif

#endif
