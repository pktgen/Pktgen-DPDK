/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#ifndef __RTE_VEC_H
#define __RTE_VEC_H

#include <rte_malloc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VEC_DEFAULT_SIZE	8

struct rte_vec {
	uint16_t flags;		/**< Flags for Vec structure */
	uint16_t len;		/**< Number of pointers in vector list */
	uint16_t tlen;		/**< Total number of vectors */
	void *vpool;		/**< pool from were the vector was allocated */
	uint16_t pad0[20];	/**< force to cache line size */
	void	*list[0];	/**< Vector list of pointer place holder */
};

enum {
	VEC_FREE_FLAG	   = 0x0001,
	VEC_COMPACT_FLAG   = 0x0002,
	VEC_DONT_FREE_FLAG = 0x0004,
	VEC_CREATE_FLAG    = 0x0008,
	VEC_COND_WAIT_FLAG = 0x8000,
	VEC_RESET_MASK     = (VEC_DONT_FREE_FLAG | VEC_CREATE_FLAG),
	VEC_CLEAR_FLAGS	   = 0x0000
};

#define rte_vec_foreach(idx, var, vec)					\
	for(idx = 0, var = rte_vec_at_index((vec), idx);		\
		idx < rte_vec_len((vec));				\
		idx++, var = rte_vec_at_index((vec), idx))

static inline void
rte_vec_init(struct rte_vec *v, unsigned int n, uint16_t flags)
{
	v->len   = 0;
	v->tlen  = n;
	v->flags = flags;
	v->vpool = NULL;
}

static inline unsigned int
rte_vec_calc_size(unsigned int cnt)
{
	unsigned int size;

	if (cnt == 0)
		cnt = VEC_DEFAULT_SIZE;
	size = (cnt * sizeof(void *)) + sizeof(struct rte_vec);

	return RTE_ALIGN_CEIL(size, RTE_CACHE_LINE_SIZE);
}

static inline uint32_t
rte_vec_calc_count(struct rte_mempool *mp)
{
	uint32_t size = mp->elt_size;

	size = (size - sizeof(struct rte_vec) - sizeof(void *)) / sizeof(void *);

	return size;
}

static inline void
rte_vec_set_free(struct rte_vec *v)
{
	v->flags |= VEC_FREE_FLAG;
}

static inline int
rte_vec_is_free(struct rte_vec *v)
{
	return v->flags & VEC_FREE_FLAG;
}

static inline int
rte_vec_is_dont_free(struct rte_vec *vec)
{
	return vec->flags & VEC_DONT_FREE_FLAG;
}

static inline void
rte_vec_set_dont_free(struct rte_vec *vec)
{
	vec->flags |= VEC_DONT_FREE_FLAG;
}

static inline void
rte_vec_clr_dont_free(struct rte_vec *vec)
{
	vec->flags &= ~VEC_DONT_FREE_FLAG;
}

static inline __rte_always_inline uint16_t
rte_vec_len(struct rte_vec *v)
{
	return v->len;
}

static inline __rte_always_inline int
rte_vec_byte_len(struct rte_vec *v)
{
	return v->len * sizeof(void *);
}

static inline __rte_always_inline void
rte_vec_set_len(struct rte_vec *v, uint16_t n)
{
	v->len = n;
}

static inline __rte_always_inline void
rte_vec_set_max_len(struct rte_vec *v, uint16_t n)
{
	v->tlen = n;
}

static inline __rte_always_inline void
rte_vec_dec_len(struct rte_vec *v)
{
	v->len--;
}

static inline __rte_always_inline void
rte_vec_inc_len(struct rte_vec *v)
{
	v->len++;
}

static inline __rte_always_inline uint16_t
rte_vec_max_len(struct rte_vec *v)
{
	return v->tlen;
}

static inline __rte_always_inline struct rte_mbuf **
rte_vec_list(struct rte_vec *v)
{
	return (struct rte_mbuf * *)&v->list[0];
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

/* return -1 on full and index value if OK */
static inline __rte_always_inline int
rte_vec_add1(struct rte_vec *vec, void *val)
{
	if (vec->len >= vec->tlen)
		return -1;

	vec->list[vec->len++] = val;
	return vec->len - 1;
}

static inline __rte_always_inline int
rte_vec_add_at_index(struct rte_vec *vec, void *val, uint16_t n)
{
	if (vec->len >= vec->tlen)
		return -1;
	vec->list[n] = val;
	vec->len++;
	return 0;
}

static inline __rte_always_inline void *
rte_vec_at_index(struct rte_vec *vec, uint16_t n)
{
	if (n >= vec->len)
		return NULL;
	return vec->list[n];
}

static inline __rte_always_inline void
rte_vec_set_at_index(struct rte_vec *vec, uint16_t idx, void *val)
{
	if (idx < vec->tlen)
		vec->list[idx] = val;
}

static inline __rte_always_inline struct rte_mbuf **
rte_vec_addr(struct rte_vec *vec, uint16_t n)
{
	return (struct rte_mbuf * *)&vec->list[n];
}

static inline __rte_always_inline struct rte_mbuf **
rte_vec_end(struct rte_vec *vec)
{
	return (struct rte_mbuf * *)&vec->list[vec->len];
}

static inline __rte_always_inline int
rte_vec_len_remaining(struct rte_vec *vec)
{
	return vec->tlen - vec->len;
}

static inline __rte_always_inline int
rte_vec_is_full(struct rte_vec *v)
{
	return (v->len == v->tlen);
}

#pragma GCC diagnostic pop

static inline void
rte_vec_reset(struct rte_mempool *mp, struct rte_vec *vec)
{
	uint16_t flags = vec->flags;

	vec->flags = (flags & VEC_RESET_MASK);
	vec->vpool = mp;
	rte_vec_set_max_len(vec, rte_vec_calc_count(mp));
	rte_vec_set_len(vec, 0);
}

static inline int
rte_vec_find_index(struct rte_vec *vec, void *v)
{
	int i;

	for (i = 0; i < rte_vec_len(vec); i++) {
		if (rte_vec_at_index(vec, i) == v)
			return i;
	}
	return -1;
}

static inline struct rte_vec *
rte_vec_alloc(struct rte_mempool *mp)
{
	struct rte_vec *vec;

	if (rte_mempool_get(mp, (void * *)&vec))
		return NULL;

	return vec;
}

static inline void
rte_vec_free(struct rte_vec *vec)
{
	struct rte_mempool *mp;

	if (!vec)
		return;

	if (rte_vec_is_free(vec))
		return;

	rte_vec_set_len(vec, 0);
	if (vec->flags & VEC_RESET_MASK) {
		vec->flags = (vec->flags & VEC_RESET_MASK);
		return;
	}

	vec->flags = (vec->flags & VEC_RESET_MASK);
	vec->flags |= VEC_FREE_FLAG;

	mp = vec->vpool;

	rte_vec_reset(mp, vec);

	rte_mempool_put(mp, vec);
}

/**
 * Allocate a bulk of vecs, reset the fields to default values.
 *
 *  @param pool
 *    The objpool from which vec are allocated.
 *  @param vecs
 *    Array of pointers to vec
 *  @param count
 *    Array size
 *  @return
 *   - 0: Success
 */
static inline int
rte_vec_alloc_bulk(struct rte_mempool *mp,
		    struct rte_vec **vecs, unsigned count)
{
	unsigned idx = 0;
	int	 rc;

	rc = rte_mempool_get_bulk(mp, (void * *)vecs, count);
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
			rte_vec_reset(mp, vecs[idx++]);
			/* fall-through */
		case 3:
			rte_vec_reset(mp, vecs[idx++]);
			/* fall-through */
		case 2:
			rte_vec_reset(mp, vecs[idx++]);
			/* fall-through */
		case 1:
			rte_vec_reset(mp, vecs[idx++]);
			/* fall-through */
		}
	}
	return 0;
}

static inline void
rte_vec_free_bulk(struct rte_vec **vecs, uint32_t n)
{
	uint32_t i;

	for (i = 0; i < n; i++)
		rte_vec_free(vecs[i]);
}

static inline void
rte_vec_free_mbufs(struct rte_vec *vec)
{
	unsigned	  idx = 0, count;
	struct rte_mbuf **mbufs;

	if (!vec)
		return;
	count = rte_vec_len(vec);
	mbufs = rte_vec_list(vec);
	rte_vec_set_len(vec, 0);

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

static inline void
rte_vec_clr_at_index(struct rte_vec *vec, uint16_t idx)
{
	/* Assume idx and vec are valid for vec array */
	vec->list[idx] = NULL;
	vec->flags    |= VEC_COMPACT_FLAG;
}

static inline void
rte_vec_compact(struct rte_vec *vec)
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

	len = vec->len;
	vec->len = 0;

	for(i = 0; i < len; i++) {
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

static inline void
rte_vec_find_delete(struct rte_vec *vec, void *val)
{
	int idx = rte_vec_find_index(vec, val);

	if (idx != -1) {
		rte_vec_clr_at_index(vec, idx);
		rte_vec_compact(vec);
	}
}

static inline int
rte_vec_pop(struct rte_vec *vec, void **val)
{
	RTE_ASSERT(vec && val);

	if (vec->len) {
		*val = rte_vec_at_index(vec, 0);
		rte_vec_clr_at_index(vec, 0);
		rte_vec_compact(vec);
		return 1;
	}
	return 0;
}

static inline void
rte_vec_free_mbuf_at_index(struct rte_vec *vec, uint16_t idx)
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

static inline int
rte_vec_move_at_index(struct rte_vec *to, struct rte_vec *from, uint16_t idx)
{
	void *v;
	int   rc;

	v  = rte_vec_at_index(from, idx);
	rc = rte_vec_add1(to, v);
	if (rc >= 0)
		rte_vec_clr_at_index(from, idx);

	return rc;
}

static inline void
rte_vec_copy_at_index(struct rte_vec *to, struct rte_vec *from, uint16_t idx)
{
	void *v;

	v = rte_vec_at_index(from, idx);
	rte_vec_add1(to, v);
}

static inline void
rte_vec_move_mbuf(struct rte_vec *to, struct rte_vec *from, uint16_t idx)
{
	if (rte_vec_move_at_index(to, from, idx))
		rte_pktmbuf_free(rte_vec_at_index(from, idx));
}

static inline struct rte_mbuf *
rte_vec_get_and_clr(struct rte_vec *vec, uint16_t idx)
{
	struct rte_mbuf *m;

	m = rte_vec_at_index(vec, idx);
	rte_vec_clr_at_index(vec, idx);

	return m;
}

struct rte_vec *rte_vec_create(const char *name, unsigned int n, uint16_t flags);
void rte_vec_destroy(struct rte_vec *vec);

struct rte_mempool *rte_vec_create_pool(const char *name, unsigned int n,
				unsigned int entries, unsigned int cache_size);
void rte_vec_destroy_pool(struct rte_mempool *obj);

int rte_vec_to_data(struct rte_vec *v, char *buf, size_t len);

void rte_vec_print(FILE *f, const char *msg, struct rte_vec *vec);

#ifdef __cplusplus
}
#endif

#endif /* __RTE_VEC_H */
