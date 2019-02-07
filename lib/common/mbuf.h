/*-
 *   Copyright(c) <2014-2019> Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MBUF_H_
#define _MBUF_H_

#include <rte_atomic.h>
#include <rte_mbuf.h>
#include <rte_hexdump.h>

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

/**
 * Dump an mbuf structure to a file.
 *
 * Dump all fields for the given packet mbuf and all its associated
 * segments (in the case of a chained buffer).
 *
 * @param f
 *   A pointer to a file for output
 * @param m
 *   The packet mbuf.
 * @param dump_len
 *   If dump_len != 0, also dump the "dump_len" first data bytes of
 *   the packet.
 */
/* dump a mbuf on console */
static inline void
dnet_pktmbuf_dump(FILE *f, const struct rte_mbuf *m, unsigned dump_len)
{
        unsigned int len;
        unsigned int nb_segs;
	char buf[256];

        __rte_mbuf_sanity_check(m, 1);

        fprintf(f, "dump mbuf at %p, iova=%"PRIx64", buf_len=%u\n",
               m, (uint64_t)m->buf_iova, (unsigned)m->buf_len);
        fprintf(f, "  pkt_len=%"PRIu32", nb_segs=%u, "
               "in_port=%u\n", m->pkt_len,
               (unsigned)m->nb_segs, (unsigned)m->port);
	rte_get_rx_ol_flag_list(m->ol_flags, buf, sizeof(buf));
	fprintf(f, "  rx_ol_flags: %s\n", buf);
	rte_get_tx_ol_flag_list(m->ol_flags, buf, sizeof(buf));
	fprintf(f, "  tx_ol_flags: %s\n", buf);
        nb_segs = m->nb_segs;

        while (m && nb_segs != 0) {
                __rte_mbuf_sanity_check(m, 0);

                fprintf(f, "  segment at %p, data=%p, data_len=%u\n",
                        m, rte_pktmbuf_mtod(m, void *), (unsigned)m->data_len);
                len = dump_len;
                if (len > m->data_len)
                        len = m->data_len;
                if (len != 0)
                        rte_hexdump(f, NULL, rte_pktmbuf_mtod(m, void *), len);
                dump_len -= len;
                m = m->next;
                nb_segs --;
        }
}

#ifdef __cplusplus
}
#endif

#endif /* _MBUF_H_ */
