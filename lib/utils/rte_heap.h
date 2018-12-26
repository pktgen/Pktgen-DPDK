/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

#ifndef __RTE_HEAP_H
#define __RTE_HEAP_H

#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/queue.h>

#include <rte_spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct heap_entry {
	STAILQ_ENTRY(heap_entry) next;		/**< pointer to next entry */
	size_t size;				/**< size of free entry */
} rte_heap_entry_t;

typedef struct rte_heap {
	STAILQ_HEAD(, heap_entry) list;		/**< Heap entry list */
	void *addr;				/**< Base Heap address pointer */
	size_t total_space;			/**< total space in heap */
	rte_spinlock_t sl;			/**< Spinlocl for this heap */
} rte_heap_t;

/**
 * FUNCTION PROTOTYPES.
 */

/* prototypes */
rte_heap_t *rte_heap_create(void *addr, size_t size);
int rte_heap_destroy(rte_heap_t *si);

void *rte_heap_alloc(rte_heap_t *si, size_t size);
int rte_heap_free(rte_heap_t *si, void *addr, size_t size);

void *rte_heap_malloc(rte_heap_t *si, size_t size);
int rte_heap_mfree(rte_heap_t *si, void *addr);

/**
 * Debug Utility Functions
 */
void rte_heap_dump(FILE *f, rte_heap_t *si);

#ifdef __cplusplus
}
#endif

#endif /* __RTE_HEAP_H */
