/*-
 *   BSD LICENSE
 *
 *   Copyright 2016 6WIND S.A. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdint.h>

#include <rte_version.h>
#include <rte_mbuf.h>
#if RTE_VERSION >= RTE_VERSION_NUM(17,2,0,0)
#include <rte_mbuf_ptype.h>
#endif
#include <rte_byteorder.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>

#include "pg_inet.h"

