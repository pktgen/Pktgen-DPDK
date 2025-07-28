/*-
 * Copyright(c) <2025-2025>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2025 by Keith Wiles @ intel.com */

#include <rte_common.h>

#include "pktgen.h"
#include "pktgen-port-cfg.h"
#include "pktgen-workq.h"

static workq_t wq_info;

workq_port_t *
workq_port_get(uint16_t pid)
{
    if (!rte_eth_dev_is_valid_port(pid))
        return NULL;

    return wq_info.ports[pid];
}

int
workq_run(workq_type_t wqt, uint16_t pid, uint16_t qid)
{
    workq_port_t *wqp = workq_port_get(pid);
    workq_entry_t *wqe;
    port_info_t *pinfo;

    if (!wqp || wqt >= WORKQ_MAX)
        return -1;

    wqe   = (wqt == WORKQ_RX) ? &wqp->rx : &wqp->tx;
    pinfo = wqp->arg;

    for (uint16_t w = 0; w < wqe->cnt; w++)
        wqe->func[w](pinfo, qid);

    return 0;
}

int
workq_add(workq_type_t wqt, uint16_t pid, workq_fn func)
{
    workq_port_t *wqp;
    workq_entry_t *wqe;

    if (!func)
        return -1;

    wqp = workq_port_get(pid);
    if (!wqp)
        return -1;

    wqe = (wqt == WORKQ_RX) ? &wqp->rx : &wqp->tx;

    if (wqe->cnt >= MAX_WORKQ_ENTRIES)
        return -1;

    wqe->func[wqe->cnt++] = func;

    return 0;
}

int
workq_del(workq_type_t wqt, uint16_t pid, workq_fn func)
{
    workq_port_t *wqp;
    workq_entry_t *wqe;

    if (!func)
        return -1;

    wqp = workq_port_get(pid);
    if (!wqp)
        return -1;
    wqe = (wqt == WORKQ_RX) ? &wqp->rx : &wqp->tx;

    for (uint16_t w = 0; w < wqe->cnt; w++) {
        if (wqe->func[w] == func) {
            memmove(&wqe->func[w], &wqe->func[w + 1], (wqe->cnt - w - 1) * sizeof(workq_fn));
            wqe->cnt--;
            return 0;
        }
    }
    return -1;
}

int
workq_reset(workq_type_t wqt, uint16_t pid)
{
    workq_port_t *wqp;
    workq_entry_t *wqe;

    wqp = workq_port_get(pid);
    if (!wqp)
        return -1;

    wqe      = (wqt == WORKQ_RX) ? &wqp->rx : &wqp->tx;
    wqe->cnt = 0;

    return 0;
}

int
workq_port_create(uint16_t pid)
{
    workq_port_t *wqp;

    if (!rte_eth_dev_is_valid_port(pid))
        return -1;
    if (wq_info.ports[pid] != NULL)
        return -1;

    wqp = rte_zmalloc_socket("wq_port", sizeof(workq_port_t), 0, rte_eth_dev_socket_id(pid));
    if (!wqp) {
        printf("Failed to allocate memory for workq_port_t\n");
        return -1;
    }

    wq_info.ports[pid] = wqp;

    return 0;
}

int
workq_port_destroy(uint16_t pid)
{
    workq_port_t *wqp;

    if (!rte_eth_dev_is_valid_port(pid))
        return -1;

    if ((wqp = wq_info.ports[pid]) != NULL) {
        wq_info.ports[pid] = NULL;
        rte_free(wqp);
    }

    return 0;
}

int
workq_port_arg_set(uint16_t pid, void *arg)
{
    workq_port_t *wqp;

    if (!rte_eth_dev_is_valid_port(pid))
        return -1;

    wqp = wq_info.ports[pid];
    if (wqp) {
        wqp->arg = arg;
        return 0;
    }
    return -1;
}

int
workq_create(void)
{
    for (uint16_t i = 0; i < RTE_MAX_ETHPORTS; i++) {
        if (rte_eth_dev_is_valid_port(i))
            workq_port_create(i);
    }

    return 0;
}

int
workq_destroy(void)
{
    for (uint16_t i = 0; i < RTE_MAX_ETHPORTS; i++)
        if (rte_eth_dev_is_valid_port(i))
            workq_port_destroy(i);

    return 0;
}

RTE_INIT(workq_constructor) { memset(&wq_info, 0, sizeof(wq_info)); }
