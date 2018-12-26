/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

/**
 * @file
 *
 * String-related utility functions
 */

#ifndef _RTE_LINK_H_
#define _RTE_LINK_H_

#include <rte_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * print the port link status to the given file pointer.
 *
 * @param f
 *   File pointer for output, if null use stdout.
 * @param portid
 *   Port ID number.
 * @return
 *   -1 if portid is greater then RTE_MAX_ETHPORTS or 0 if OK
 */
int __rte_experimental rte_link_status_print(FILE *f, uint16_t portid);

/**
 * Show the link status of all ports in the port_list variable.
 *
 * @param f
 *   File pointer for output, if null use stdout.
 * @param port_list
 *   A 64bit mask of ports to display link status information.
 */
void __rte_experimental rte_link_status_show(FILE *f, uint64_t port_list);

/**
 * Given the list of port return true is all ports are UP.
 *
 * @param port_list
 *   A port list used to test all ports are up.
 * @return
 *   If all of the ports in the port_list are up return 1 or 0 if not
 */
int __rte_experimental rte_link_status_check(uint16_t portid, struct rte_eth_link *link);

/**
 * Wait for ports to be up and then display link status information. Using
 * the number of seconds as a timeout.
 *
 * @param f
 *   The file pointer to send the output or if NULL use stdout.
 * @param port_list
 *   A list of ports to test waiting for all port to have link status of UP.
 * @param secs
 *   Number of seconds to wait for all port, if zero then use 9 seconds.
 */
void __rte_experimental rte_link_status_wait(FILE *f, uint64_t port_list, int secs);

/**
 * Attempt to set the link status cancel flag to force rte_link_status_wait()
 * break out of the waiting loop. Normally used inside a signal hander routine.
 */
void __rte_experimental rte_link_status_check_cancel(void);

#ifdef __cplusplus
}
#endif

#endif /* _RTE_LINK_H */
