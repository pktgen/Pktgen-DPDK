/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2024> Intel Corporation.
 */

#ifndef __HEAP_H
#define __HEAP_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <pthread.h>

#include <rte_common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct heap_entry {
    STAILQ_ENTRY(heap_entry) next; /**< pointer to next entry */
    size_t size;                   /**< size of free entry */
} heap_entry_t;

typedef struct heap {
    STAILQ_HEAD(, heap_entry) list; /**< Heap entry list */
    void *addr;                     /**< Base Heap address pointer */
    size_t total_space;             /**< total space in heap */
    pthread_spinlock_t sl;          /**< Spinlock for this heap */
} heap_t;

/**
 * FUNCTION PROTOTYPES.
 */

/* prototypes */
heap_t *heap_create(void *addr, size_t size);
int heap_destroy(heap_t *si);

void *heap_alloc(heap_t *si, size_t size);
int heap_free(heap_t *si, void *addr, size_t size);

void *heap_malloc(heap_t *si, size_t size);
int heap_mfree(heap_t *si, void *addr);

/**
 * Debug Utility Functions
 */
void heap_dump(FILE *f, heap_t *si);

#ifdef __cplusplus
}
#endif

#endif /* __HEAP_H */
