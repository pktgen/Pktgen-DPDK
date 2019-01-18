/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation.
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>

#include <rte_ethdev.h>
#include <rte_cycles.h>

#include "rte_portlist.h"
#include "rte_link.h"

static volatile bool lsc_cancel = 0;

static int
get_link_status(uint16_t portid, struct rte_eth_link *link)
{
	struct rte_eth_link _link;

	if (!link)
		link = &_link;

	memset(link, 0, sizeof(struct rte_eth_link));

	rte_eth_link_get_nowait(portid, link);

	return link->link_status;
}

int
rte_link_status_print(FILE *f, uint16_t portid)
{
	struct rte_eth_link link;

	if (portid > RTE_MAX_ETHPORTS)
		return -1;

	if (!f)
		f = stdout;

	get_link_status(portid, &link);

	if (link.link_status)
		fprintf(f, "Port%d Link Up- speed %u Mbps- %s\n",
			portid, link.link_speed,
			(link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
				("full-duplex") : ("half-duplex\n"));
	else
		fprintf(f, "Port %d Link Down\n", portid);
	fflush(f);

	return 0;
}

void
rte_link_status_show(FILE *f, uint64_t port_list)
{
	uint16_t portid;

	RTE_ETH_FOREACH_DEV(portid) {
		if (lsc_cancel)
			break;

		if ((port_list & (1 << portid)) == 0)
			continue;

		rte_link_status_print(f, portid);
	}
}

int
rte_link_status_check(uint16_t portid, struct rte_eth_link *link)
{
	return get_link_status(portid, link);
}

/* Check the link status of all ports in up to 9s, and print them finally */
void
rte_link_status_wait(FILE *f, uint64_t port_list, int secs)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define DEFAULT_WAIT_TIME 9 /* 9 seconds */
	uint16_t portid, all_up;
	uint8_t count;
	char buf[128];

	if (!port_list)
		port_list = -1;

	if (!f)
		f = stdout;

	if (!secs)
		secs = DEFAULT_WAIT_TIME;	/* Default number of Seconds */

	secs *= 1000;		/* Convert to milli-seconds */
	secs /= CHECK_INTERVAL;	/* Convert to 100ms intervals or ticks */

	fprintf(f, "\nChecking portlist %s link status: ",
		rte_print_portlist(NULL, port_list, buf, sizeof(buf)));

	lsc_cancel = 0;

	for (count = 0; count <= secs; count++) {
		all_up = 1;

		for(portid = 0; portid < RTE_MAX_ETHPORTS; portid++) {
			if (lsc_cancel)
				return;

			if ((1LL << portid) & port_list) {
				if (!get_link_status(portid, NULL))
					all_up = 0;
			}
		}
		if (all_up)
			break;

		fprintf(f, ".");
		fflush(f);

		rte_delay_us_sleep(CHECK_INTERVAL * 1000);
	}

	fprintf(f, "done\n");
	fflush(f);

	rte_link_status_show(f, port_list);
}

void
rte_link_status_check_cancel(void)
{
	lsc_cancel = 1;
}
