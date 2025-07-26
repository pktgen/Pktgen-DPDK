/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) <2020-2025> Intel Corporation.
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

#define PG_JUMBO_ETHER_MTU     9216        // 9K total size of the Ethernet jumbo frame
#define PG_JUMBO_DATAROOM_SIZE 9000        // 9K data room size in the Ethernet jumbo frame
#define PG_JUMBO_HEADROOM_SIZE \
    (PG_JUMBO_ETHER_MTU -      \
     PG_JUMBO_DATAROOM_SIZE)        // 9K headroom size in the Ethernet jumbo frame

static __inline__ int
pg_socket_id(void)
{
    int sid = rte_socket_id();

    return (sid == -1) ? 0 : sid;
}

static __inline__ int
pg_eth_dev_socket_id(int pid)
{
    int sid = rte_eth_dev_socket_id(pid);

    return (sid == -1) ? 0 : sid;
}

#ifdef __cplusplus
}
#endif

#endif /* PG_COMPAT_H_ */
