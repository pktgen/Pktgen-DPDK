/*-
 * Copyright(c) <2010-2021>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CONSTANTS_H_
#define _PKTGEN_CONSTANTS_H_

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MAX_PKT_RX_BURST = 128, /* Used to create Max array sizes */
    MAX_PKT_TX_BURST = 128, /* Used to create Max array sizes */
    DEFAULT_PKT_RX_BURST = 64, /* Increasing this number consumes memory very fast */
    DEFAULT_PKT_TX_BURST = 64, /* Increasing this number consumes memory very fast */
    DEFAULT_RX_DESC   = (MAX_PKT_RX_BURST * 8),
    DEFAULT_TX_DESC   = (MAX_PKT_TX_BURST * 16),

    MAX_MBUFS_PER_PORT = ((DEFAULT_RX_DESC + DEFAULT_TX_DESC) * 8), /* number of buffers to support per port */
    MAX_SPECIAL_MBUFS  = 128,
    MBUF_CACHE_SIZE    = 128,

    DEFAULT_PRIV_SIZE = 0,

    NUM_Q = 16, /**< Number of queues per port. */
};
#define DEFAULT_MBUF_SIZE                                                                         \
    (PG_JUMBO_FRAME_LEN + RTE_PKTMBUF_HEADROOM) /* See: http://dpdk.org/dev/patchwork/patch/4479/ \
                                                 */

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CONSTANTS_H_ */
