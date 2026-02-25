/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2026> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef __VEC_H
#define __VEC_H

/**
 * @file
 *
 * DPDK mempool-backed pointer vector container.
 *
 * A vec is a fixed-capacity array of void pointers allocated from an
 * rte_mempool object.  It is used throughout Pktgen for batching
 * rte_mbuf pointers across lcore-to-port boundaries.
 */

#include <rte_malloc.h>
#include <pg_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VEC_DEFAULT_SIZE 8

struct vec {
    uint16_t flags;    /**< Flags for Vec structure */
    uint16_t len;      /**< Number of pointers in vector list */
    uint16_t tlen;     /**< Total number of vectors */
    void *vpool;       /**< pool from were the vector was allocated */
    uint16_t pad0[20]; /**< force to cache line size */
    void *list[0];     /**< Vector list of pointer place holder */
};

/** Flag bits stored in struct vec::flags. */
enum {
    VEC_FREE_FLAG      = 0x0001, /**< Vec has been returned to its mempool */
    VEC_COMPACT_FLAG   = 0x0002, /**< Vec contains NULL gaps; compact needed */
    VEC_DONT_FREE_FLAG = 0x0004, /**< Do not return vec to mempool on free */
    VEC_CREATE_FLAG    = 0x0008, /**< Vec was created via vec_create() */
    VEC_COND_WAIT_FLAG = 0x8000, /**< Internal: conditional-wait in use */
    VEC_RESET_MASK  = (VEC_DONT_FREE_FLAG | VEC_CREATE_FLAG), /**< Flags preserved across reset */
    VEC_CLEAR_FLAGS = 0x0000                                  /**< Clear all flags */
};

/**
 * Iterate over all entries in a vec.
 *
 * @param idx   Integer index variable (declared by caller).
 * @param var   Pointer variable set to each element on each iteration.
 * @param vec   Pointer to the struct vec to iterate over.
 */
#define vec_foreach(idx, var, vec)                                      \
    for (idx = 0, var = vec_at_index((vec), idx); idx < vec_len((vec)); \
         idx++, var   = vec_at_index((vec), idx))

/**
 * Initialise an already-allocated vec in place.
 *
 * @param v
 *   Pointer to the vec to initialise.
 * @param n
 *   Maximum number of elements (total length).
 * @param flags
 *   Initial flag bits (see VEC_* constants).
 */
static inline void
vec_init(struct vec *v, unsigned int n, uint16_t flags)
{
    v->len   = 0;
    v->tlen  = n;
    v->flags = flags;
    v->vpool = NULL;
}

/**
 * Calculate the allocation size in bytes for a vec with @p cnt entries.
 *
 * The result is cache-line aligned. If @p cnt is zero, VEC_DEFAULT_SIZE is used.
 *
 * @param cnt
 *   Desired element capacity.
 * @return
 *   Byte size of the vec object, aligned to RTE_CACHE_LINE_SIZE.
 */
static inline unsigned int
vec_calc_size(unsigned int cnt)
{
    unsigned int size;

    if (cnt == 0)
        cnt = VEC_DEFAULT_SIZE;
    size = (cnt * sizeof(void *)) + sizeof(struct vec);

    return RTE_ALIGN_CEIL(size, RTE_CACHE_LINE_SIZE);
}

/**
 * Calculate how many vec entries fit within one mempool element.
 *
 * @param mp
 *   Mempool whose element size determines the entry count.
 * @return
 *   Number of void-pointer entries available in a single vec element.
 */
static inline uint32_t
vec_calc_count(struct rte_mempool *mp)
{
    uint32_t size = mp->elt_size;

    size = (size - sizeof(struct vec) - sizeof(void *)) / sizeof(void *);

    return size;
}

/** Mark vec @p v as already returned to its mempool. */
static inline void
vec_set_free(struct vec *v)
{
    v->flags |= VEC_FREE_FLAG;
}

/** Return non-zero if vec @p v has been freed (VEC_FREE_FLAG set). */
static inline int
vec_is_free(struct vec *v)
{
    return v->flags & VEC_FREE_FLAG;
}

/** Return non-zero if VEC_DONT_FREE_FLAG is set on @p vec. */
static inline int
vec_is_dont_free(struct vec *vec)
{
    return vec->flags & VEC_DONT_FREE_FLAG;
}

/** Set VEC_DONT_FREE_FLAG on @p vec so it is not returned to its mempool on free. */
static inline void
vec_set_dont_free(struct vec *vec)
{
    vec->flags |= VEC_DONT_FREE_FLAG;
}

/** Clear VEC_DONT_FREE_FLAG on @p vec, re-enabling normal mempool return. */
static inline void
vec_clr_dont_free(struct vec *vec)
{
    vec->flags &= ~VEC_DONT_FREE_FLAG;
}

/** Return the current number of elements in vec @p v. */
static __rte_always_inline uint16_t
vec_len(struct vec *v)
{
    return v->len;
}

/** Return the used size of vec @p v in bytes (len * sizeof(void*)). */
static __rte_always_inline int
vec_byte_len(struct vec *v)
{
    return v->len * sizeof(void *);
}

/** Set the current element count of vec @p v to @p n. */
static __rte_always_inline void
vec_set_len(struct vec *v, uint16_t n)
{
    v->len = n;
}

/** Set the maximum element capacity of vec @p v to @p n. */
static __rte_always_inline void
vec_set_max_len(struct vec *v, uint16_t n)
{
    v->tlen = n;
}

/** Decrement the element count of vec @p v by one. */
static __rte_always_inline void
vec_dec_len(struct vec *v)
{
    v->len--;
}

/** Increment the element count of vec @p v by one. */
static __rte_always_inline void
vec_inc_len(struct vec *v)
{
    v->len++;
}

/** Return the maximum element capacity of vec @p v. */
static __rte_always_inline uint16_t
vec_max_len(struct vec *v)
{
    return v->tlen;
}

/** Return a pointer to the start of the rte_mbuf pointer array in @p v. */
static __rte_always_inline struct rte_mbuf **
vec_list(struct vec *v)
{
    return (struct rte_mbuf **)&v->list[0];
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

/**
 * Append one pointer to @p vec.
 *
 * @param vec   Vec to append to.
 * @param val   Pointer value to append.
 * @return      Index of the inserted element, or -1 if the vec is full.
 */
static __rte_always_inline int
vec_add1(struct vec *vec, void *val)
{
    if (vec->len >= vec->tlen)
        return -1;

    vec->list[vec->len++] = val;
    return vec->len - 1;
}

/**
 * Store @p val at a specific index @p n in @p vec and increment the length.
 *
 * @param vec   Vec to write into.
 * @param val   Pointer value to store.
 * @param n     Target index.
 * @return      0 on success, -1 if the vec is already full.
 */
static __rte_always_inline int
vec_add_at_index(struct vec *vec, void *val, uint16_t n)
{
    if (vec->len >= vec->tlen)
        return -1;
    vec->list[n] = val;
    vec->len++;
    return 0;
}

/**
 * Return the pointer at index @p n in @p vec.
 *
 * @param vec   Vec to read from.
 * @param n     Element index.
 * @return      Pointer value, or NULL if @p n is out of range.
 */
static __rte_always_inline void *
vec_at_index(struct vec *vec, uint16_t n)
{
    if (n >= vec->len)
        return NULL;
    return vec->list[n];
}

/**
 * Overwrite the entry at index @p idx in @p vec without changing the length.
 *
 * @param vec   Vec to modify.
 * @param idx   Target index (must be within tlen).
 * @param val   New pointer value.
 */
static __rte_always_inline void
vec_set_at_index(struct vec *vec, uint16_t idx, void *val)
{
    if (idx < vec->tlen)
        vec->list[idx] = val;
}

/**
 * Return a pointer to the rte_mbuf pointer at index @p n in @p vec.
 *
 * @param vec   Vec to address.
 * @param n     Element index.
 * @return      Pointer to element @p n.
 */
static __rte_always_inline struct rte_mbuf **
vec_addr(struct vec *vec, uint16_t n)
{
    return (struct rte_mbuf **)&vec->list[n];
}

/**
 * Return a pointer one past the last valid element of @p vec.
 *
 * @param vec   Vec to address.
 * @return      Pointer to the slot after the last element.
 */
static __rte_always_inline struct rte_mbuf **
vec_end(struct vec *vec)
{
    return (struct rte_mbuf **)&vec->list[vec->len];
}

/**
 * Return the number of unused slots remaining in @p vec.
 *
 * @param vec   Vec to query.
 * @return      tlen - len.
 */
static __rte_always_inline int
vec_len_remaining(struct vec *vec)
{
    return vec->tlen - vec->len;
}

/**
 * Return non-zero if @p v has no remaining capacity (len == tlen).
 *
 * @param v   Vec to test.
 * @return    Non-zero when full.
 */
static __rte_always_inline int
vec_is_full(struct vec *v)
{
    return (v->len == v->tlen);
}

#pragma GCC diagnostic pop

/**
 * Reset @p vec to an empty state, preserving sticky flags (DONT_FREE, CREATE).
 *
 * @param mp    Mempool that owns @p vec (stored in vec->vpool).
 * @param vec   Vec to reset.
 */
static inline void
vec_reset(struct rte_mempool *mp, struct vec *vec)
{
    uint16_t flags = vec->flags;

    vec->flags = (flags & VEC_RESET_MASK);
    vec->vpool = mp;
    vec_set_max_len(vec, vec_calc_count(mp));
    vec_set_len(vec, 0);
}

/**
 * Search @p vec for pointer @p v and return its index.
 *
 * @param vec   Vec to search.
 * @param v     Pointer value to find.
 * @return      Index of the first matching entry, or -1 if not found.
 */
static inline int
vec_find_index(struct vec *vec, void *v)
{
    int i;

    for (i = 0; i < vec_len(vec); i++) {
        if (vec_at_index(vec, i) == v)
            return i;
    }
    return -1;
}

/**
 * Allocate a single vec from @p mp.
 *
 * @param mp    Mempool to allocate from.
 * @return      Pointer to the allocated vec, or NULL on failure.
 */
static inline struct vec *
vec_alloc(struct rte_mempool *mp)
{
    struct vec *vec;

    if (rte_mempool_get(mp, (void **)&vec))
        return NULL;

    return vec;
}

/**
 * Return @p vec to its owning mempool (unless VEC_DONT_FREE_FLAG is set).
 *
 * If @p vec is NULL, already freed (VEC_FREE_FLAG), or has VEC_RESET_MASK
 * flags set, the function resets it in place without returning it to the pool.
 *
 * @param vec   Vec to free. May be NULL.
 */
static inline void
vec_free(struct vec *vec)
{
    struct rte_mempool *mp;

    if (!vec)
        return;

    if (vec_is_free(vec))
        return;

    vec_set_len(vec, 0);
    if (vec->flags & VEC_RESET_MASK) {
        vec->flags = (vec->flags & VEC_RESET_MASK);
        return;
    }

    vec->flags = (vec->flags & VEC_RESET_MASK);
    vec->flags |= VEC_FREE_FLAG;

    mp = vec->vpool;

    vec_reset(mp, vec);

    rte_mempool_put(mp, vec);
}

/**
 * Allocate @p count vecs from @p mp in one call and reset each to its default state.
 *
 * Uses Duff's device loop unrolling for efficiency.
 *
 * @param mp
 *   Mempool from which vecs are allocated.
 * @param vecs
 *   Caller-supplied array of pointers, filled with allocated vec objects.
 * @param count
 *   Number of vecs to allocate.
 * @return
 *   0 on success, non-zero if rte_mempool_get_bulk() fails.
 */
static inline int
vec_alloc_bulk(struct rte_mempool *mp, struct vec **vecs, unsigned count)
{
    unsigned idx = 0;
    int rc;

    rc = rte_mempool_get_bulk(mp, (void **)vecs, count);
    if (unlikely(rc))
        return rc;

    /* To understand duff's device on loop unwinding optimization, see
     * https://en.wikipedia.org/wiki/Duff's_device.
     * Here while() loop is used rather than do() while{} to avoid extra
     * check if count is zero.
     */
    switch (count % 4) {
    case 0:
        while (idx != count) {
            vec_reset(mp, vecs[idx++]);
            /* fall-through */
        case 3:
            vec_reset(mp, vecs[idx++]);
            /* fall-through */
        case 2:
            vec_reset(mp, vecs[idx++]);
            /* fall-through */
        case 1:
            vec_reset(mp, vecs[idx++]);
            /* fall-through */
        }
    }
    return 0;
}

/**
 * Free @p n vecs from the @p vecs array by calling vec_free() on each.
 *
 * @param vecs   Array of vec pointers to free.
 * @param n      Number of entries in @p vecs.
 */
static inline void
vec_free_bulk(struct vec **vecs, uint32_t n)
{
    uint32_t i;

    for (i = 0; i < n; i++)
        vec_free(vecs[i]);
}

/**
 * Free every rte_mbuf stored in @p vec and reset its length to zero.
 *
 * Uses Duff's device loop unrolling. The vec itself is NOT returned to its
 * mempool; call vec_free() separately if that is desired.
 *
 * @param vec   Vec whose mbuf entries should be freed. May be NULL.
 */
static inline void
vec_free_mbufs(struct vec *vec)
{
    unsigned idx = 0, count;
    struct rte_mbuf **mbufs;

    if (!vec)
        return;
    count = vec_len(vec);
    mbufs = vec_list(vec);
    vec_set_len(vec, 0);

    /* To understand duff's device on loop unwinding optimization, see
     * https://en.wikipedia.org/wiki/Duff's_device.
     * Here while() loop is used rather than do() while{} to avoid extra
     * check if count is zero.
     */
    switch (count % 4) {
    case 0:
        while (idx != count) {
            rte_pktmbuf_free(mbufs[idx++]);
            /* fall-through */
        case 3:
            rte_pktmbuf_free(mbufs[idx++]);
            /* fall-through */
        case 2:
            rte_pktmbuf_free(mbufs[idx++]);
            /* fall-through */
        case 1:
            rte_pktmbuf_free(mbufs[idx++]);
            /* fall-through */
        }
    }
}

/**
 * Set the entry at @p idx to NULL and mark @p vec for compaction.
 *
 * @param vec   Vec to modify.
 * @param idx   Index to clear (must be a valid index).
 */
static inline void
vec_clr_at_index(struct vec *vec, uint16_t idx)
{
    /* Assume idx and vec are valid for vec array */
    vec->list[idx] = NULL;
    vec->flags |= VEC_COMPACT_FLAG;
}

/**
 * Remove NULL gaps from @p vec by shifting remaining entries down.
 *
 * Only runs if VEC_COMPACT_FLAG is set; clears the flag when done.
 *
 * @param vec   Vec to compact in place.
 */
static inline void
vec_compact(struct vec *vec)
{
    void **l1, **l2;
    uint16_t len;
    int i;

    if ((vec->flags & VEC_COMPACT_FLAG) == 0)
        return;

    vec->flags &= ~VEC_COMPACT_FLAG;

    if (!vec->len)
        return;

    /* Compress out the NULL entries */
    l1 = l2 = vec->list;

    len      = vec->len;
    vec->len = 0;

    for (i = 0; i < len; i++) {
        if (*l1) {
            if (l1 == l2)
                l2++;
            else
                *l2++ = *l1;
            vec->len++;
        }
        l1++;
    }
}

/**
 * Find and remove the first occurrence of @p val from @p vec.
 *
 * Compacts the vec after removal.
 *
 * @param vec   Vec to search and modify.
 * @param val   Pointer value to find and remove.
 */
static inline void
vec_find_delete(struct vec *vec, void *val)
{
    int idx = vec_find_index(vec, val);

    if (idx != -1) {
        vec_clr_at_index(vec, idx);
        vec_compact(vec);
    }
}

/**
 * Remove and return the first element from @p vec.
 *
 * @param vec   Vec to pop from.
 * @param val   Output: set to the first element's pointer value.
 * @return      1 if an element was popped, 0 if @p vec was empty.
 */
static inline int
vec_pop(struct vec *vec, void **val)
{
    RTE_ASSERT(vec && val);

    if (vec->len) {
        *val = vec_at_index(vec, 0);
        vec_clr_at_index(vec, 0);
        vec_compact(vec);
        return 1;
    }
    return 0;
}

/**
 * Free the rte_mbuf at index @p idx in @p vec and NULL out its slot.
 *
 * Sets VEC_COMPACT_FLAG; out-of-range @p idx is silently ignored.
 *
 * @param vec   Vec containing the mbuf.
 * @param idx   Index of the mbuf to free.
 */
static inline void
vec_free_mbuf_at_index(struct vec *vec, uint16_t idx)
{
    void *val;

    if (idx >= vec->len)
        return;

    /* Assume idx is valid for vec array */
    val = vec->list[idx];

    vec->list[idx] = NULL;

    rte_pktmbuf_free((struct rte_mbuf *)val);

    vec->flags |= VEC_COMPACT_FLAG;
}

/**
 * Move the entry at @p idx from @p from to @p to.
 *
 * On success, the source slot is cleared (compaction deferred). On failure
 * (destination full), the source entry is left intact.
 *
 * @param to    Destination vec.
 * @param from  Source vec.
 * @param idx   Index in @p from to move.
 * @return      Index assigned in @p to on success, -1 if @p to is full.
 */
static inline int
vec_move_at_index(struct vec *to, struct vec *from, uint16_t idx)
{
    void *v;
    int rc;

    v  = vec_at_index(from, idx);
    rc = vec_add1(to, v);
    if (rc >= 0)
        vec_clr_at_index(from, idx);

    return rc;
}

/**
 * Copy the entry at @p idx from @p from into @p to (source is not cleared).
 *
 * @param to    Destination vec.
 * @param from  Source vec.
 * @param idx   Index in @p from to copy.
 */
static inline void
vec_copy_at_index(struct vec *to, struct vec *from, uint16_t idx)
{
    void *v;

    v = vec_at_index(from, idx);
    vec_add1(to, v);
}

/**
 * Move the mbuf at @p idx from @p from to @p to; free the mbuf if @p to is full.
 *
 * @param to    Destination vec.
 * @param from  Source vec.
 * @param idx   Index of the mbuf in @p from to move.
 */
static inline void
vec_move_mbuf(struct vec *to, struct vec *from, uint16_t idx)
{
    if (vec_move_at_index(to, from, idx))
        rte_pktmbuf_free(vec_at_index(from, idx));
}

/**
 * Retrieve the mbuf at @p idx from @p vec and clear its slot.
 *
 * @param vec   Vec to read from.
 * @param idx   Element index.
 * @return      Pointer to the rte_mbuf at @p idx (may be NULL if already cleared).
 */
static inline struct rte_mbuf *
vec_get_and_clr(struct vec *vec, uint16_t idx)
{
    struct rte_mbuf *m;

    m = vec_at_index(vec, idx);
    vec_clr_at_index(vec, idx);

    return m;
}

/**
 * Create a standalone vec (allocated with rte_zmalloc, not from a mempool).
 *
 * @param name   Logical name used for allocation tracking (passed to rte_zmalloc).
 * @param n      Element capacity of the new vec.
 * @param flags  Initial flag bits (see VEC_* constants).
 * @return       Pointer to the new vec, or NULL on allocation failure.
 */
struct vec *vec_create(const char *name, unsigned int n, uint16_t flags);

/**
 * Destroy a vec previously created with vec_create().
 *
 * @param vec   Vec to free. If NULL, the call is a no-op.
 */
void vec_destroy(struct vec *vec);

/**
 * Create a DPDK mempool sized to hold vecs with @p entries elements each.
 *
 * @param name         Name for the new mempool.
 * @param n            Number of vec objects in the pool.
 * @param entries      Element capacity of each vec object.
 * @param cache_size   Per-lcore cache size for the mempool.
 * @return             Pointer to the new rte_mempool, or NULL on failure.
 */
struct rte_mempool *vec_create_pool(const char *name, unsigned int n, unsigned int entries,
                                    unsigned int cache_size);

/**
 * Free a mempool previously created with vec_create_pool().
 *
 * @param obj   Mempool to free. If NULL, the call is a no-op.
 */
void vec_destroy_pool(struct rte_mempool *obj);

/**
 * Serialise the pointer values in @p v into a human-readable string.
 *
 * @param v    Vec to serialise.
 * @param buf  Destination buffer.
 * @param len  Size of @p buf in bytes.
 * @return     0 on success, -1 if @p buf is too small.
 */
int vec_to_data(struct vec *v, char *buf, size_t len);

/**
 * Print a summary of @p vec to file @p f with optional prefix @p msg.
 *
 * @param f     Output file (e.g. stdout).
 * @param msg   Optional prefix message string, or NULL.
 * @param vec   Vec to print.
 */
void vec_print(FILE *f, const char *msg, struct vec *vec);

#ifdef __cplusplus
}
#endif

#endif /* __VEC_H */
