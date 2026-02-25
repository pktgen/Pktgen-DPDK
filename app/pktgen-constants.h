/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CONSTANTS_H_
#define _PKTGEN_CONSTANTS_H_

/**
 * @file
 *
 * Memory and burst sizing constants for Pktgen.
 */

#include <rte_mbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MAX_PKT_RX_BURST                  = 256,  /**< Maximum RX burst size (array dimension) */
    MAX_PKT_TX_BURST                  = 256,  /**< Maximum TX burst size (array dimension) */
    DEFAULT_PKT_RX_BURST              = 64,   /**< Default RX burst size */
    DEFAULT_PKT_TX_BURST              = 32,   /**< Default TX burst size */
    DEFAULT_RX_DESC                   = 1024, /**< Default RX descriptor ring size */
    DEFAULT_TX_DESC                   = 1024, /**< Default TX descriptor ring size */
    DEFAULT_MBUFS_PER_PORT_MULTIPLIER = 16,   /**< Multiplier for mbufs-per-port calculation */
    MAX_SPECIAL_MBUFS                 = 1024, /**< Maximum special mbufs per port */
    MBUF_CACHE_SIZE                   = 256,  /**< Per-lcore mempool cache size */
    DEFAULT_PRIV_SIZE                 = 0,    /**< Default mbuf private data size */
};

/** Compute the total mbufs needed for a port given its descriptor ring sizes. */
#define MAX_MBUFS_PER_PORT(rxd, txd) ((rxd + txd) * DEFAULT_MBUFS_PER_PORT_MULTIPLIER)

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CONSTANTS_H_ */
