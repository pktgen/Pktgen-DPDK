/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2023> Intel Corporation.
 */
/* Created 2018 by Keith Wiles @ intel.com */

#include <rte_memory.h>
#include <rte_timer.h>
#include <rte_prefetch.h>
#include <rte_mbuf.h>

#include "vec.h"

static void
vec_obj_init(struct rte_mempool *mp, void *uarg __rte_unused, void *obj, unsigned idx __rte_unused)
{
    struct vec *v = (struct vec *)obj;

    vec_reset(mp, v);
}

struct rte_mempool *
vec_create_pool(const char *name, unsigned int n, unsigned int entires, unsigned int cache_size)
{
    struct rte_mempool *mp = NULL;
    unsigned int size      = vec_calc_size(entires);

    mp = rte_mempool_create(name, n, size, cache_size, 0, NULL, NULL, NULL, NULL, rte_socket_id(),
                            0);
    if (mp)
        rte_mempool_obj_iter(mp, vec_obj_init, NULL);

    return mp;
}

void
vec_destroy_pool(struct rte_mempool *obj)
{
    rte_mempool_free(obj);
}

struct vec *
vec_create(const char *name, unsigned int n, uint16_t flags)
{
    struct vec *vec;
    uint32_t vec_size;

    vec_size = vec_calc_size(n);

    vec = rte_zmalloc_socket(name, vec_size, RTE_CACHE_LINE_SIZE, rte_socket_id());
    if (vec) {
        vec->tlen  = n;
        vec->flags = flags;
        vec->flags |= VEC_CREATE_FLAG;
    }

    return vec;
}

void
vec_destroy(struct vec *vec)
{
    rte_free(vec);
}

int
vec_to_data(struct vec *v, char *buf, size_t len)
{
    size_t count = 0, cnt;
    uint16_t vlen;
    int i;

    vlen = vec_len(v);
    for (i = 0; i < vlen && len; i++) {
        struct rte_mbuf *m = vec_at_index(v, i);

        /* set cnt to number of bytes that will fit in buffer */
        cnt = RTE_MIN(rte_pktmbuf_pkt_len(m), len);

        /* Copy the data from mbuf to buffer for cnt bytes */
        rte_memcpy(buf, rte_pktmbuf_mtod(m, char *), cnt);

        rte_pktmbuf_adj(m, cnt); /* reduce the pkt size */

        count += cnt;
        len -= cnt;
        buf += cnt;

        if (rte_pktmbuf_pkt_len(m) == 0)
            vec_free_mbuf_at_index(v, i);
        else
            break;
    }
    vec_compact(v);

    return count;
}

void
vec_print(FILE *f, const char *msg, struct vec *v)
{
    int i, k;

    if (!f)
        f = stderr;
    if (!msg)
        fprintf(f, "Vector: @ %p: ", v);
    else
        fprintf(f, "Vector: %s @ %p ", msg, v);
    fprintf(f, " flags %04x, len %u, tlen %u\n", v->flags, v->len, v->tlen);
    fprintf(f, "        vpool %p\nList: ", v->vpool);
    for (i = 0, k = 0; i < v->len; i++) {
        fprintf(f, "%p ", v->list[i]);
        if (++k >= 8) {
            k = 0;
            fprintf(f, "\n      ");
        }
    }
    fprintf(f, "\n");
}
