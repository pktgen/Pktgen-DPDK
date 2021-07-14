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
#if !defined(RTE_LIBRTE_VMXNET3_PMD) && !defined(RTE_NET_VMXNET3)
	DEFAULT_PKT_BURST       = 64,	/* Increasing this number consumes memory very fast */
	DEFAULT_RX_DESC         = (DEFAULT_PKT_BURST * 8 * 2),
	DEFAULT_TX_DESC         = DEFAULT_RX_DESC * 2,
#else
	DEFAULT_PKT_BURST       = 128,	/* Increasing this number consumes memory very fast */
	DEFAULT_RX_DESC         = (DEFAULT_PKT_BURST * 8),
	DEFAULT_TX_DESC         = DEFAULT_RX_DESC * 2,
#endif

	MAX_MBUFS_PER_PORT      = (DEFAULT_TX_DESC * 8),/* number of buffers to support per port */
	MAX_SPECIAL_MBUFS       = 64,
	MBUF_CACHE_SIZE         = (MAX_MBUFS_PER_PORT / 8),

	DEFAULT_PRIV_SIZE       = 0,

	NUM_Q                   = 16,	/**< Number of cores per port. */
};
#define DEFAULT_MBUF_SIZE	(PG_ETHER_MAX_JUMBO_FRAME_LEN + RTE_PKTMBUF_HEADROOM) /* See: http://dpdk.org/dev/patchwork/patch/4479/ */

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_CONSTANTS_H_ */
