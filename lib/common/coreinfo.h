/*-
 * Copyright(c) <2014-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#ifndef __COREINFO_H
#define __COREINFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROC_CPUINFO "/proc/cpuinfo"

// clang-format off
typedef enum {
    CI_NUM_LCORES_TYPE,
    CI_NUM_SOCKETS_TYPE,
    CI_NUM_CORES_TYPE,
    CI_NUM_THREADS_TYPE,
    CI_NUM_SIBLINGS_TYPE,
    CI_NUM_CPU_CORES_TYPE
} ci_type_t;
// clang-format on

typedef struct coreinfo_s {
    __extension__ union {
        struct {
            uint16_t lcore_id;  /* Logical core ID */
            uint16_t core_id;   /* Physical CPU core ID */
            uint16_t socket_id; /* CPU socket ID */
            uint16_t thread_id; /* Hyper-thread ID */
        };
        uint64_t word;
    };
} coreinfo_t;

coreinfo_t *coreinfo_get(uint16_t lcore_id);
uint16_t coreinfo_lcore_cnt(void);
uint16_t coreinfo_core_cnt(void);
uint16_t coreinfo_socket_cnt(void);
uint16_t coreinfo_thread_cnt(void);
uint16_t coreinfo_siblings_cnt(void);
uint16_t coreinfo_cpu_cores_cnt(void);

#ifdef __cplusplus
}
#endif

#endif /*_COREINFO_H */
