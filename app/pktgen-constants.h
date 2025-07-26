/*-
 * Copyright(c) <2010-2025>, Intel Corporation. All rights reserved.
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
    MAX_PKT_RX_BURST                  = 128,  /* Used to create Max array sizes */
    MAX_PKT_TX_BURST                  = 128,  /* Used to create Max array sizes */
    DEFAULT_PKT_RX_BURST              = 64,   /* Increasing this number consumes memory very fast */
    DEFAULT_PKT_TX_BURST              = 32,   /* Increasing this number consumes memory very fast */
    DEFAULT_RX_DESC                   = 1024, /* Default Rx/Tx ring descriptor size */
    DEFAULT_TX_DESC                   = 1024, /* Default Rx/Tx ring descriptor size */
    DEFAULT_MBUFS_PER_PORT_MULTIPLIER = 16,   /* Multiplier for number of mbufs per port */
    MAX_SPECIAL_MBUFS                 = 1024,
    MBUF_CACHE_SIZE                   = 256,
    DEFAULT_PRIV_SIZE                 = 0,
};

#define MAX_MBUFS_PER_PORT(rxd, txd) ((rxd + txd) * DEFAULT_MBUFS_PER_PORT_MULTIPLIER)

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CONSTANTS_H_ */
