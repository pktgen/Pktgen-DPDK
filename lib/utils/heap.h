/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2026> Intel Corporation.
 */

#ifndef __HEAP_H
#define __HEAP_H

/**
 * @file
 *
 * Fixed-size heap allocator backed by a caller-supplied memory region.
 *
 * The heap manages a contiguous block of memory supplied at creation time.
 * Allocations and frees are protected by a spinlock.  The implementation
 * is intended for use in low-latency DPDK data-plane paths where dynamic
 * system allocators are undesirable.
 */

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
 * Create a heap over a caller-supplied memory region.
 *
 * @param addr
 *   Base address of the memory region to manage.
 * @param size
 *   Total size of the region in bytes.
 * @return
 *   Pointer to the new heap_t descriptor, or NULL on failure.
 */
heap_t *heap_create(void *addr, size_t size);

/**
 * Destroy a heap and release the heap_t descriptor.
 *
 * The underlying memory region is NOT freed; the caller is responsible for
 * managing the lifetime of the memory passed to heap_create().
 *
 * @param si
 *   Heap to destroy.
 * @return
 *   0 on success, -1 on error.
 */
int heap_destroy(heap_t *si);

/**
 * Allocate @p size bytes from heap @p si.
 *
 * The caller must track @p size to pass to heap_free().
 *
 * @param si
 *   Heap to allocate from.
 * @param size
 *   Number of bytes to allocate.
 * @return
 *   Pointer to the allocated region, or NULL if the heap has insufficient space.
 */
void *heap_alloc(heap_t *si, size_t size);

/**
 * Return a previously allocated block back to heap @p si.
 *
 * @param si
 *   Heap that owns the block.
 * @param addr
 *   Pointer returned by heap_alloc().
 * @param size
 *   Original allocation size passed to heap_alloc().
 * @return
 *   0 on success, -1 on error.
 */
int heap_free(heap_t *si, void *addr, size_t size);

/**
 * Allocate @p size bytes from @p si, storing the size internally for later use by heap_mfree().
 *
 * @param si
 *   Heap to allocate from.
 * @param size
 *   Number of bytes to allocate.
 * @return
 *   Pointer to the allocated region, or NULL on failure.
 */
void *heap_malloc(heap_t *si, size_t size);

/**
 * Free a block allocated with heap_malloc().
 *
 * The allocation size is read from internal metadata; the caller does not
 * need to track it separately (unlike heap_free()).
 *
 * @param si
 *   Heap that owns the block.
 * @param addr
 *   Pointer returned by heap_malloc().
 * @return
 *   0 on success, -1 on error.
 */
int heap_mfree(heap_t *si, void *addr);

/**
 * Dump a human-readable description of the heap's free-list to @p f.
 *
 * @param f    Output file (e.g. stdout or stderr).
 * @param si   Heap to dump.
 */
void heap_dump(FILE *f, heap_t *si);

#ifdef __cplusplus
}
#endif

#endif /* __HEAP_H */
