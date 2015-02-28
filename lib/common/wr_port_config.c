/*-
 * Copyright (c) <2010-2014>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the applcation must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include "wr_scrn.h"
#include "wr_port_config.h"
#include "wr_core_info.h"

#define PORT_STRING_SIZE	256

/**************************************************************************//**
*
* wr_get_portdesc - Parse the lspci command output to find ports.
*
* DESCRIPTION
* Parse the lspci command output to find valid ports to use.
*
* RETURNS: Number of ports found.
*
* SEE ALSO:
*/

uint32_t
wr_get_portdesc(struct rte_pci_addr * pciAddr, uint8_t ** portdesc, uint32_t num, int verbose )
{
	FILE * fd;
	uint32_t	idx;
	char buff[PORT_STRING_SIZE], *p;

	if ( (num <= 0) || (pciAddr == NULL) || (portdesc == NULL) )
		return 0;

	// Only parse the Ethernet cards on the PCI bus.
	fd = popen("lspci -D | grep Ethernet", "r");
	if ( fd == NULL )
		rte_panic("*** Unable to do lspci may need to be installed");

	if ( verbose )
		fprintf(stdout, "\n** Bit Mask: All ports in system\n");

	idx = 0;
	while( fgets(buff, sizeof(buff), fd) ) {
		p = &buff[0];

		// add a null at the end of the string.
		p[strlen(buff)-1] = 0;

		// Decode the 0000:00:00.0 PCI device address.
		pciAddr[idx].domain		= strtol(  p, &p, 16);
		pciAddr[idx].bus		= strtol(++p, &p, 16);
		pciAddr[idx].devid		= strtol(++p, &p, 16);
		pciAddr[idx].function	= strtol(++p, &p, 16);

		if ( verbose )
			fprintf(stdout, " 0x%016llx: %s\n", (1ULL << idx), buff);

		// Save the port description for later if asked to do so.
		if ( portdesc )
			portdesc[idx] = (uint8_t *)strdup(buff);		// portdesc[idx] needs to be NULL or we lose memory.

		if ( ++idx >= num )
			break;
	}

	pclose(fd);
	if ( verbose )
		fprintf(stdout, "\nFound %d ports\n", idx);

	return idx;
}

/**************************************************************************//**
*
* wr_free_portdesc - Free the allocated memory for port descriptions.
*
* DESCRIPTION
* Free the allocated memory for the port descriptions
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
wr_free_portdesc( uint8_t ** portdesc, uint32_t num )
{
	uint32_t		i;

	for( i = 0; i < num; i++ ) {
		if ( portdesc[i] )
			free((char *)portdesc[i]);
		portdesc[i] = NULL;
	}
}

/**************************************************************************//**
*
* wr_create_blacklist - Create a port blacklist.
*
* DESCRIPTION
* Create a port blacklist from the port and port descriptions.
*
* RETURNS: Number of ports in list.
*
* SEE ALSO:
*/

uint32_t
wr_create_blacklist(uint64_t portmask, struct rte_pci_addr * portlist, uint32_t port_cnt, uint8_t * desc[]) {
    uint32_t i, idx;
    char pci_addr_str[32];

    if ( (portmask == 0) || (portlist == NULL) || (port_cnt == 0) || (desc == NULL) )
    	return 0;

	fprintf(stdout, "Ports: Port Mask: %016lx blacklisted = --, not-blacklisted = ++\n", portmask);
	idx = 0;
    for(i = 0; i < port_cnt; i++) {
		memset(pci_addr_str, 0, sizeof(pci_addr_str));
		if ( (portmask & (1ULL << i)) == 0 ) {
			fprintf(stdout, "-- %s\n", desc[i]);
			strncpy(pci_addr_str, (void *)desc[i], 12);
			rte_eal_devargs_add(RTE_DEVTYPE_BLACKLISTED_PCI, pci_addr_str);
			idx++;
		} else {
			strncpy(pci_addr_str, (void *)desc[i], 12);
			rte_eal_devargs_add(RTE_DEVTYPE_WHITELISTED_PCI, pci_addr_str);
			fprintf(stdout, "++ %s\n", desc[i]);
		}
	}
    if ( desc )
    	fprintf(stdout, "\n");

	return idx;
}
