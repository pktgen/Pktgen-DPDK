/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2024> Intel Corporation.
 */

/**
 * @file
 *
 * Compat file for pktgen
 */

#ifndef PG_COMPAT_H_
#define PG_COMPAT_H_

#include <rte_version.h>
#include <rte_ethdev.h>
#include <rte_ether.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __RTE_VERSION RTE_VERSION_NUM(RTE_VER_YEAR, RTE_VER_MONTH, RTE_VER_MINOR, RTE_VER_RELEASE)

#define PG_JUMBO_ETHER_MTU 9216
#define PG_JUMBO_FRAME_LEN (PG_JUMBO_ETHER_MTU + RTE_ETHER_CRC_LEN + RTE_ETHER_HDR_LEN)

#ifdef __cplusplus
}
#endif

#endif /* PG_COMPAT_H_ */
