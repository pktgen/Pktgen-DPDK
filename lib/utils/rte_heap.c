/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

#include <stdio.h>
#include <sys/queue.h>

#include <rte_spinlock.h>

#include "rte_heap.h"

rte_heap_t *
rte_heap_create(void *addr, size_t size)
{
	rte_heap_t *heap = NULL;
	rte_heap_entry_t *entry;

	/* Make sure the size is greater or equal to sizeof(rte_heap_entry_t) */
	if (size < sizeof(rte_heap_entry_t))
		return NULL;

	heap = calloc(1, sizeof(rte_heap_t));
	if (!heap)
		return NULL;

	rte_spinlock_init(&heap->sl);

	heap->addr = addr;
	heap->total_space = size;

	STAILQ_INIT(&heap->list);

	entry = addr;
	entry->next.stqe_next = NULL;
	entry->size = size;

	STAILQ_INSERT_TAIL(&heap->list, entry, next);

	return heap;
}

/******************************************************************************
* simpleMemDestory - Destory a heap.
*
*   heap - is the free heap structure pointer.
*/
int
rte_heap_destroy(rte_heap_t *heap)
{
	/* Free the rte_heap_t structure */
	if ( heap )
		free(heap);

	return 0;
}

int
rte_heap_free(rte_heap_t *heap, void *addr, size_t size)
{
	rte_heap_entry_t *p;
	rte_heap_entry_t *q;

	/* the size can not be zero */
	if (!heap || !addr || size == 0)
		return -1;

	rte_spinlock_lock(&heap->sl);

	p = addr;

	p->next.stqe_next = NULL;
	p->size = size;

	STAILQ_FOREACH(q, &heap->list, next) {
		/* insert into accending order */
		if (p < q->next.stqe_next)
			break;
	}

	if (q) {
		if (RTE_PTR_ADD(p, size) == q->next.stqe_next) {
			p->size += q->size;
			p->next.stqe_next = q->next.stqe_next->next.stqe_next;
		} else                      /* non contiguous regions insert p into list */
			p->next.stqe_next = q->next.stqe_next;

		/* join to lower element */
		if (RTE_PTR_ADD(q, q->size) == p) {
			/* add size of prev region */
			q->size += p->size;

			/* point to p->next region */
			q->next.stqe_next = p->next.stqe_next;
		} else                     /* not contiguous regions  insert p into list */
			q->next.stqe_next = p;
	} else
		STAILQ_INSERT_TAIL(&heap->list, p, next);

	rte_spinlock_unlock(&heap->sl);

	return 0;
}

/******************************************************************************
* simpleMemAlloc: allocate space for a tlv structure and data.
*
*   This routine will allocate a contiguous amount of memory
*   of size (size) from the free list.
*
*   This routine will search a linked list for the first
*   fit free space.
*
*   This routine will return a null pointer if the contiguous
*   free space is not found.
*
*   heap - is the pointer to the heap structure.
*   size - is the size of the requested memory.
*/
void *
rte_heap_alloc(rte_heap_t *heap, size_t size)
{
	rte_heap_entry_t *hd;		/* pointer to entry free space */
	rte_heap_entry_t *phd;		/* prev head pointer to free list */
	rte_heap_entry_t *nxt_hd;	/* temp space for pointer to entry */
	rte_heap_entry_t *ret_ptr;	/* return pointer to free space */

	if (!heap || size == 0)
		return NULL;

	rte_spinlock_lock(&heap->sl);

	size = RTE_ALIGN_CEIL(size, sizeof(rte_heap_entry_t));

	/*
	 * If the requested size is less then sizeof(rte_heap_entry_t) then set
	 * the requested size to sizeof(rte_heap_entry_t)
	 */
	if ( size < sizeof(rte_heap_entry_t) )
		size = sizeof(rte_heap_entry_t);

	ret_ptr = NULL;
	hd      = STAILQ_FIRST(&heap->list);
	phd     = (rte_heap_entry_t *)&heap->list;
	if (hd) {
		if (size <= heap->total_space) {
			do {
				/*
				 * if current free section size is equal to size then
				 * point prev head.
				 */
				if (size == hd->size) {
					/* take size out of total free space */
					heap->total_space -= size;

					/* to entry free section in list. */
					phd->next.stqe_next = hd->next.stqe_next;

					ret_ptr = hd;
					break;
				}
				/*
				 * section is larger than size build a new rte_heap_entry_t structure
				 * in the left over space in this section.
				 * nxt_hd is the new pointer to the structure after size.
				 */
				if ( size <= (hd->size - sizeof(rte_heap_entry_t)) ) {
					/* take size out of total free space */
					heap->total_space -= size;

					/* current entry is now moved */
					nxt_hd			= RTE_PTR_ADD(hd, size);
					nxt_hd->next.stqe_next	= hd->next.stqe_next;
					nxt_hd->size    	= hd->size - size;
					phd->next.stqe_next	= nxt_hd;

					ret_ptr         = hd;
					break;
				}
				phd = hd;               /* Prev head = current head */
				hd  = hd->next.stqe_next;         /* entry free space */
			} while(hd);
		}
	}

	rte_spinlock_unlock(&heap->sl);

	return ret_ptr;
}

/******************************************************************************
* simpleMemMalloc - Wrapper routine for simpleMemAlloc insert size into buffer.
*/
void *
rte_heap_malloc(rte_heap_t *heap, size_t size)
{
	uint64_t *addr;

	if (!heap || size == 0)
		return NULL;

	if (size < sizeof(rte_heap_entry_t))
		size = sizeof(rte_heap_entry_t);

	addr = (uint64_t *)rte_heap_alloc(heap, size + sizeof(uint64_t));
	if ( addr == NULL )
		return NULL;

	addr[0] = size + sizeof(uint64_t);

	return &addr[1];
}

/******************************************************************************
* simpleMemMFree - Wrapper routine for simpleMemFree extract size from buffer.
*/
int
rte_heap_mfree(rte_heap_t *heap, void *addr)
{
	uint64_t *p;
	uint64_t size;

	if (heap == NULL)
		return -1;

	if (addr == NULL)
		return 0;

	if (((uint64_t)addr & 3) != 0 )
		return -1;

	p = (uint64_t *)addr;
	p--;

	size = *p;
	if (size == 0)
		return -1;

	return rte_heap_free(heap, p, size);
}

/******************************************************************************
* debug function
*/
void
rte_heap_dump(FILE *f, rte_heap_t *heap)
{
	rte_heap_entry_t    *p;
	uint64_t       total;
	uint64_t       largest;
	uint64_t       segCount;

	if (!f)
		f = stdout;

	if (!heap) {
		fprintf(f, "Pointer to rte_heap_entry_t structure is NULL\n");
		return;
	}

	fprintf(f, "  Free Header       = %p\n", heap);
	fprintf(f, "  Address of Heap   = %p\n", heap->addr);
	fprintf(f, "  Total free space  = %lu\n", heap->total_space);

	total = 0;
	largest = 0;
	segCount = 0;

	STAILQ_FOREACH(p, &heap->list, next) {
		fprintf(f, "    Size            = %ld\n", p->size);
		total += p->size;
		if (p->size > largest)
			largest = p->size;
		segCount++;
	}
	fprintf(f, "  Total Free        = %ld\n", total);
	fprintf(f, "  Largest Free area = %ld\n", largest);
	fprintf(f, "  Segment Count     = %ld\n", segCount);
}
