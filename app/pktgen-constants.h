/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
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
    MAX_PKT_RX_BURST     = 128, /* Used to create Max array sizes */
    MAX_PKT_TX_BURST     = 128, /* Used to create Max array sizes */
    DEFAULT_PKT_RX_BURST = 64,  /* Increasing this number consumes memory very fast */
    DEFAULT_PKT_TX_BURST = 64,  /* Increasing this number consumes memory very fast */
    DEFAULT_RX_DESC      = (MAX_PKT_RX_BURST * 8),
    DEFAULT_TX_DESC      = (MAX_PKT_TX_BURST * 16),

    DEFAULT_MBUFS_PER_PORT_MULTIPLER = 8, /* Multipler for number of mbufs per port */
    MAX_SPECIAL_MBUFS = 1024,
    MBUF_CACHE_SIZE   = 128,

    DEFAULT_PRIV_SIZE = 0,

    NUM_Q = 64, /**< Number of queues per port. */
};

#define MAX_MBUFS_PER_PORT(rxd, txd) ((rxd + txd) * DEFAULT_MBUFS_PER_PORT_MULTIPLER)

/*
 * Some NICs require >= 2KB buffer as a receive data buffer. DPDK uses 2KB + HEADROOM (128) as
 * the default MBUF buffer size. This would make the pktmbuf buffer 2KB + HEADROOM +
 * sizeof(rte_mbuf) which is 2048 + 128 + 128 = 2304 mempool buffer size.
 *
 * For Jumbo frame buffers lets use MTU 9216 + CRC(4) + L2(14) = 9234, for buffer size we use 10KB
 */
#define _MBUF_LEN         (PG_JUMBO_FRAME_LEN + RTE_PKTMBUF_HEADROOM + sizeof(struct rte_mbuf))
#define DEFAULT_MBUF_SIZE (10 * 1024)

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CONSTANTS_H_ */
