/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2013 by Keith Wiles @ intel.com */

#ifndef _PORT_CONFIG_H
#define _PORT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

uint32_t get_portdesc(struct rte_pci_addr *pciAddr,
			     uint8_t **portdesc,
			     uint32_t num,
			     int verbose);
void free_portdesc(uint8_t **portdesc, uint32_t num);
uint32_t create_blacklist(uint64_t portmask,
				 struct rte_pci_addr *portlist,
				 uint32_t port_cnt,
				 uint8_t * desc[]);

#ifdef __cplusplus
}
#endif

#endif /* _PORT_CONFIG_H */
