/*-
 * Copyright (c) <2014-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2014 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <rte_version.h>
#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_pci.h>
#include <rte_devargs.h>
#include <rte_debug.h>

#include "port_config.h"
#include "core_info.h"

#define PORT_STRING_SIZE    256

#if RTE_VERSION >= RTE_VERSION_NUM(18, 5, 0, 0)
#define rte_eal_devargs_add	rte_devargs_add
#endif

/**************************************************************************//**
 *
 * get_portdesc - Parse the lspci command output to find ports.
 *
 * DESCRIPTION
 * Parse the lspci command output to find valid ports to use.
 *
 * RETURNS: Number of ports found.
 *
 * SEE ALSO:
 */

uint32_t
get_portdesc(struct rte_pci_addr *pciAddr,
	     uint8_t **portdesc,
	     uint32_t num,
	     int verbose)
{
	FILE *fd;
	uint32_t idx;
	char buff[PORT_STRING_SIZE], *p;

	if ( (num <= 0) || (pciAddr == NULL) || (portdesc == NULL) )
		return 0;

	/* Only parse the Ethernet cards on the PCI bus. */
	fd = popen("lspci -D | grep Ethernet", "r");
	if (fd == NULL)
		rte_panic("*** Unable to do lspci may need to be installed");

	if (verbose)
		fprintf(stdout, "\n** Bit Mask: All ports in system\n");

	idx = 0;
	while (fgets(buff, sizeof(buff), fd) ) {
		p = &buff[0];

		/* add a null at the end of the string. */
		p[strlen(buff) - 1] = 0;

		/* Decode the 0000:00:00.0 PCI device address. */
		pciAddr[idx].domain     = strtol(p, &p, 16);
		p++;
		pciAddr[idx].bus        = strtol(p, &p, 16);
		p++;
		pciAddr[idx].devid      = strtol(p, &p, 16);
		p++;
		pciAddr[idx].function   = strtol(p, &p, 16);

		if (verbose)
			fprintf(stdout, " 0x%016llx: %s\n", (1ULL << idx),
				buff);

		/* Save the port description for later if asked to do so. */
		if (portdesc) {
			const char *s = "ethernet controller";
			int n = strlen(s);

			p = strcasestr(buff, s);
			if (p)
				memmove(p, p + n, (strlen(buff) - ((p - buff) + n)) + 1);
			p = strcasestr(buff, s);
			n++;
			if (p)	/* Try to remove the two strings to make the line shorter */
				memmove(p, p + n, (strlen(buff) - ((p - buff) + n)) + 1);
			portdesc[idx] = (uint8_t *)strdup(buff);/* portdesc[idx] needs to be NULL or we lose memory. */
		}
		if (++idx >= num)
			break;
	}

	pclose(fd);
	if (verbose)
		fprintf(stdout, "\nFound %d ports\n", idx);

	return idx;
}

/**************************************************************************//**
 *
 * free_portdesc - Free the allocated memory for port descriptions.
 *
 * DESCRIPTION
 * Free the allocated memory for the port descriptions
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
free_portdesc(uint8_t **portdesc, uint32_t num)
{
	uint32_t i;

	for (i = 0; i < num; i++) {
		if (portdesc[i])
			free((char *)portdesc[i]);
		portdesc[i] = NULL;
	}
}

/**************************************************************************//**
 *
 * create_blacklist - Create a port blacklist.
 *
 * DESCRIPTION
 * Create a port blacklist from the port and port descriptions.
 *
 * RETURNS: Number of ports in list.
 *
 * SEE ALSO:
 */

uint32_t
create_blacklist(uint64_t portmask,
		 struct rte_pci_addr *portlist,
		 uint32_t port_cnt,
		 uint8_t *desc[]) {
	uint32_t i, idx;
	char pci_addr_str[32];

	if ( (portmask == 0) || (portlist == NULL) || (port_cnt == 0) ||
	     (desc == NULL) )
		return 0;

	fprintf(stdout,
		"Ports: Port Mask: %016" PRIx64 " blacklisted = --, not-blacklisted = ++\n",
		portmask);
	idx = 0;
	for (i = 0; i < port_cnt; i++) {
		memset(pci_addr_str, 0, sizeof(pci_addr_str));
		if ( (portmask & (1ULL << i)) == 0) {
			fprintf(stdout, "-- %s\n", desc[i]);
			snprintf(pci_addr_str, sizeof(pci_addr_str), "%s", desc[i]);
			rte_eal_devargs_add(RTE_DEVTYPE_BLACKLISTED_PCI, pci_addr_str);
			idx++;
		} else {
			snprintf(pci_addr_str, sizeof(pci_addr_str), "%s", desc[i]);
			rte_eal_devargs_add(RTE_DEVTYPE_WHITELISTED_PCI, pci_addr_str);
			fprintf(stdout, "++ %s\n", desc[i]);
		}
	}
	if (desc)
		fprintf(stdout, "\n");

	return idx;
}
