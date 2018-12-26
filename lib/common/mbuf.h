/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MBUF_H_
#define _MBUF_H_

#include <rte_atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

union pktgen_data {
	uint64_t udata;
	RTE_STD_C11
	struct {
		uint16_t data_len;
		uint16_t buf_len;
		uint32_t pkt_len;
	};
};

static inline void
pktmbuf_reset(struct rte_mbuf *m)
{
	union pktgen_data d;

	d.udata = m->udata64;	/* Save the original value */

	rte_pktmbuf_reset(m);

	m->data_len = d.data_len;
	m->pkt_len = d.pkt_len;
	m->buf_len = d.buf_len;
}

/**
 * Allocate a bulk of mbufs, initialize refcnt and reset the fields to default
 * values.
 *
 *  @param pool
 *    The mempool from which mbufs are allocated.
 *  @param mbufs
 *    Array of pointers to mbufs
 *  @param count
 *    Array size
 *  @return
 *   - 0: Success
 */
static inline int
pg_pktmbuf_alloc_bulk(struct rte_mempool *pool,
		      struct rte_mbuf **mbufs, unsigned count)
{
	unsigned idx = 0;
	int rc;

	rc = rte_mempool_get_bulk(pool, (void **)mbufs, count);
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
			pktmbuf_reset(mbufs[idx]);
			idx++;
			/* fall-through */
	case 3:
			pktmbuf_reset(mbufs[idx]);
			idx++;
			/* fall-through */
	case 2:
			pktmbuf_reset(mbufs[idx]);
			idx++;
			/* fall-through */
	case 1:
			pktmbuf_reset(mbufs[idx]);
			idx++;
			/* fall-through */
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _MBUF_H_ */
