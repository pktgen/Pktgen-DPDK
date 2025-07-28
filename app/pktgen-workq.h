/*-
 * Copyright(c) <2025-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2025 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_WORKQ_H_
#define _PKTGEN_WORKQ_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { WORKQ_RX, WORKQ_TX, WORKQ_MAX } workq_type_t;

#define MAX_WORKQ_ENTRIES 16 /** Maximum size of workqueue items */
typedef void (*workq_fn)(struct port_info_s *info, uint16_t qid);

typedef struct {
    workq_fn func[MAX_WORKQ_ENTRIES]; /**< Workq for each port */
    uint16_t cnt;                     /**< Counter to track the number of workq items */
} workq_entry_t;

typedef struct workq_port_s {
    workq_entry_t rx; /**< Workq for receiving packets */
    workq_entry_t tx; /**< Workq for transmitting packets */
    void *arg;        /**< Pointer to user defined argument */
} workq_port_t;

typedef struct workq_s {
    workq_port_t *ports[RTE_MAX_ETHPORTS]; /**< Array of workqueues per port */
} workq_t;

workq_port_t *workq_port_get(uint16_t pid);

int workq_add(workq_type_t wqt, uint16_t pid, workq_fn func);

int workq_del(workq_type_t wqt, uint16_t pid, workq_fn func);

int workq_reset(workq_type_t wqt, uint16_t pid);

int workq_run(workq_type_t wqt, uint16_t pid, uint16_t qid);

int workq_port_create(uint16_t pid);

int workq_port_destroy(uint16_t pid);

int workq_port_arg_set(uint16_t pid, void *arg);

int workq_create(void);

int workq_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_WORKQ_H_ */
