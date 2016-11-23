/**
 * BSD LICENSE
 *
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed in the application must contain the copyright notice as
 * defined above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MBUF_H_
#define _MBUF_H_

#ifdef __cplusplus
extern "C" {
#endif

static inline void
pktmbuf_reset(struct rte_mbuf *m)
{
	m->next = NULL;
	m->nb_segs = 1;
	m->port = 0xff;

	m->data_off = (RTE_PKTMBUF_HEADROOM <= m->buf_len) ?
		RTE_PKTMBUF_HEADROOM : m->buf_len;
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
#ifdef RTE_ASSERT
			RTE_ASSERT(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#else
			RTE_VERIFY(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#endif
			rte_mbuf_refcnt_set(mbufs[idx], 1);
			pktmbuf_reset(mbufs[idx]);
			idx++;
	case 3:
#ifdef RTE_ASSERT
			RTE_ASSERT(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#else
			RTE_VERIFY(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#endif
			rte_mbuf_refcnt_set(mbufs[idx], 1);
			pktmbuf_reset(mbufs[idx]);
			idx++;
	case 2:
#ifdef RTE_ASSERT
			RTE_ASSERT(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#else
			RTE_VERIFY(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#endif
			rte_mbuf_refcnt_set(mbufs[idx], 1);
			pktmbuf_reset(mbufs[idx]);
			idx++;
	case 1:
#ifdef RTE_ASSERT
			RTE_ASSERT(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#else
			RTE_VERIFY(rte_mbuf_refcnt_read(mbufs[idx]) == 0);
#endif
			rte_mbuf_refcnt_set(mbufs[idx], 1);
			pktmbuf_reset(mbufs[idx]);
			idx++;
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _MBUF_H_ */
