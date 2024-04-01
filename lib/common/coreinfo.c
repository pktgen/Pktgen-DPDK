/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2023-2024 Intel Corporation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

#include <rte_common.h>

#include <hmap.h>
#include "coreinfo.h"

#define MAX_LINE_SZ      4096
#define CI_SCT_FORMAT    "S:%04d,C:%04d,T:%04d" /* socket_id/core_id/thread_id */
#define CI_LCORE_FORMAT  "L:%04d"               /* logical core id */
#define CI_NUM_LCORES    "num_lcores"
#define CI_NUM_CORES     "num_cores"
#define CI_NUM_CPU_CORES "num_cpu_cores"
#define CI_NUM_SOCKETS   "num_sockets"
#define CI_NUM_THREADS   "num_threads"
#define CI_NUM_SIBLINGS  "num_siblings"
#define CI_MODEL_NAME    "model_name"

#define CI_LOGICAL_ID   "processor"
#define CI_MODEL_ID     "model name"
#define CI_SOCKET_ID    "physical id"
#define CI_CORE_ID      "core id"
#define CI_CPU_CORES_ID "cpu cores"
#define CI_SIBLINGS_ID  "siblings"
#define CI_TERMINAL_ID  "\n"

typedef struct {
    uint16_t socket_id;
    uint16_t core_id;
    uint16_t thread_id;
} lcore_t;

typedef struct {
    char *model_name;       /**< CPU model string */
    uint16_t max_socket_id; /**< Max sockets */
    uint16_t max_threads;   /**< Max threads per socket */
    coreinfo_t core;        /**< Core information */
    hmap_t *map;            /**< coreinfo map */
} coreinfo_data_t;
static coreinfo_data_t coreinfo_data, *cid = &coreinfo_data;

typedef void (*do_line_fn)(const char *line);
typedef unsigned (*getter_fn)(void);
typedef void (*setter_fn)(unsigned new_val);

static char *
get_value(char *line)
{
    if (*line == '\0' || *line == '\n')
        return line;

    line[strlen(line) - 1] = '\0'; /* remove newline */

    while (*line != ':')
        line++;
    *line++ = '\0';

    while (*line == ' ')
        line++;

    return line;
}

static void
set_lcore_id(const char *line)
{
    cid->core.lcore_id = atoi(line);
}

static void
set_socket_id(const char *line)
{
    cid->core.socket_id = atoi(line);
}

static void
set_max_socket_id(const char *line)
{
    uint16_t socket_id;

    socket_id = atoi(line);
    if (socket_id > cid->max_socket_id)
        cid->max_socket_id = socket_id;
}

static void
set_core_id(const char *line)
{
    cid->core.core_id = atoi(line);
}

static void
set_model_name(const char *line)
{
    if (!cid->model_name)
        cid->model_name = strdup(line);
}

static void
set_siblings(const char *line)
{
    if (hmap_add_u16(cid->map, NULL, CI_NUM_SIBLINGS, atoi(line)) < 0)
        printf("%s: Failed to add siblings to map\n", __func__);
}

static void
set_cpu_cores(const char *line)
{
    if (hmap_add_u16(cid->map, NULL, CI_NUM_CPU_CORES, atoi(line)) < 0)
        printf("%s: Failed to add siblings to map\n", __func__);
}

static void
lcore_terminator(const char *unused __attribute__((unused)))
{
    coreinfo_t *ci;
    char buffer[64];
    uint16_t num_cpu_cores = 0, num_sockets = 0;

    ci = calloc(1, sizeof(coreinfo_t));
    if (!ci) {
        printf("%s: Failed to allocate memory for core info\n", __func__);
        return;
    }
    ci->socket_id = cid->core.socket_id;
    ci->core_id   = cid->core.core_id;
    ci->lcore_id  = cid->core.lcore_id;

    hmap_get_u16(cid->map, NULL, CI_NUM_CPU_CORES, &num_cpu_cores);
    hmap_get_u16(cid->map, NULL, CI_NUM_SOCKETS, &num_sockets);
    if (num_sockets == 0)
        num_sockets = 1;
    ci->thread_id       = (cid->core.lcore_id / num_cpu_cores) / num_sockets;
    cid->core.thread_id = ci->thread_id;

    if (cid->core.thread_id > cid->max_threads)
        cid->max_threads = cid->core.thread_id;

    snprintf(buffer, sizeof(buffer), CI_LCORE_FORMAT, cid->core.lcore_id);
    hmap_add_pointer(cid->map, NULL, buffer, ci);

    snprintf(buffer, sizeof(buffer), CI_SCT_FORMAT, cid->core.socket_id, cid->core.core_id,
             cid->core.thread_id);
    hmap_add_u16(cid->map, NULL, buffer, cid->core.lcore_id);

    hmap_inc_u16(cid->map, NULL, CI_NUM_LCORES, 1);
}

static void
ignore_line(const char *line __attribute__((unused)))
{
}

static do_line_fn
get_matching_action(const char *line)
{
    // clang-format off
    static struct action {
        const char *desc;
        do_line_fn fn;
    } actions[] = {
        {CI_LOGICAL_ID, set_lcore_id}, /* lcore ID */
        {CI_SOCKET_ID, set_socket_id}, /* Socket ID */
        {CI_CORE_ID, set_core_id}, /* Core ID */
        {CI_MODEL_ID, set_model_name}, /* Model Name */
        {CI_SIBLINGS_ID, set_siblings}, /* Number of siblings */
        {CI_CPU_CORES_ID, set_cpu_cores}, /* Number of CPU cores */
        {CI_TERMINAL_ID, lcore_terminator}, /* move to next lcore ID */
        {NULL, NULL}
    };
    // clang-format on
    struct action *action;

    for (action = actions; action->fn != NULL; action++) {
        if (!strncmp(action->desc, line, strlen(action->desc)))
            return action->fn;
    }

    return ignore_line;
}

coreinfo_t *
coreinfo_get(uint16_t lcore_id)
{
    coreinfo_t *ci = NULL;
    char buffer[64];
    uint16_t num_lcores = 0, num_sockets = 0, num_cores = 0, num_threads = 0;

    hmap_get_u16(cid->map, NULL, CI_NUM_LCORES, &num_lcores);
    hmap_get_u16(cid->map, NULL, CI_NUM_CPU_CORES, &num_cores);
    hmap_get_u16(cid->map, NULL, CI_NUM_SOCKETS, &num_sockets);
    hmap_get_u16(cid->map, NULL, CI_NUM_THREADS, &num_threads);

    snprintf(buffer, sizeof(buffer), CI_LCORE_FORMAT, lcore_id);
    ci = hmap_get_pointer(cid->map, NULL, buffer);

    return ci;
}

static int
coreinfo_create(void)
{
    FILE *f;
    char *line, *value;
    size_t line_sz = MAX_LINE_SZ;

    cid->map = hmap_create("core_map", 0, NULL);
    if (cid->map == NULL)
        printf("%s: Failed to create lcore map\n", __func__);

    line = malloc(line_sz);
    if (!line) {
        printf("%s: Failed to allocate memory for line\n", __func__);
        return -1;
    }

    if ((f = fopen(PROC_CPUINFO, "r")) == NULL) {
        fprintf(stderr, "Cannot open %s on this system\n", PROC_CPUINFO);
        free(line);
        hmap_destroy(cid->map);
        return -1;
    }

    /* find number of sockets */
    do {
        if (getline(&line, &line_sz, f) == EOF)
            break;

        value = get_value(line);
        if (value != NULL && strncmp(line, CI_SOCKET_ID, strlen(CI_SOCKET_ID)) == 0)
            set_max_socket_id(value);
    } while (1);

    hmap_add_u16(cid->map, NULL, CI_NUM_SOCKETS, cid->max_socket_id + 1);

    rewind(f);
    /* process the rest of the information */
    do {
        if (getline(&line, &line_sz, f) == EOF)
            break;

        value = get_value(line);
        if (value != NULL)
            get_matching_action(line)(value);
    } while (1);

    hmap_add_u16(cid->map, NULL, CI_NUM_THREADS, cid->max_threads + 1);

    free(line);
    fclose(f);

    return 0;
}

static uint16_t
coreinfo_cnt(ci_type_t typ)
{
    uint16_t cnt = 0;

    switch (typ) {
    case CI_NUM_LCORES_TYPE:
        hmap_get_u16(cid->map, NULL, CI_NUM_LCORES, &cnt);
        break;
    case CI_NUM_CORES_TYPE:
        hmap_get_u16(cid->map, NULL, CI_NUM_CORES, &cnt);
        break;
    case CI_NUM_SOCKETS_TYPE:
        hmap_get_u16(cid->map, NULL, CI_NUM_SOCKETS, &cnt);
        break;
    case CI_NUM_THREADS_TYPE:
        hmap_get_u16(cid->map, NULL, CI_NUM_THREADS, &cnt);
        break;
    case CI_NUM_SIBLINGS_TYPE:
        hmap_get_u16(cid->map, NULL, CI_NUM_SIBLINGS, &cnt);
        break;
    case CI_NUM_CPU_CORES_TYPE:
        hmap_get_u16(cid->map, NULL, CI_NUM_CPU_CORES, &cnt);
    default:
        break;
    }
    return cnt;
}

uint16_t
coreinfo_lcore_cnt(void)
{
    return coreinfo_cnt(CI_NUM_LCORES_TYPE);
}

uint16_t
coreinfo_core_cnt(void)
{
    return coreinfo_cnt(CI_NUM_CORES_TYPE);
}

uint16_t
coreinfo_socket_cnt(void)
{
    return coreinfo_cnt(CI_NUM_SOCKETS_TYPE);
}

uint16_t
coreinfo_thread_cnt(void)
{
    return coreinfo_cnt(CI_NUM_THREADS_TYPE);
}

uint16_t
coreinfo_siblings_cnt(void)
{
    return coreinfo_cnt(CI_NUM_SIBLINGS_TYPE);
}

uint16_t
coreinfo_cpu_cores_cnt(void)
{
    return coreinfo_cnt(CI_NUM_CPU_CORES_TYPE);
}

RTE_INIT(coreinfo_init)
{
    memset(&coreinfo_data, 0, sizeof(coreinfo_data));
    coreinfo_create();
}
