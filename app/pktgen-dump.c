/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <rte_lua.h>

#include "pktgen.h"
#include "pktgen-log.h"

/**************************************************************************//**
 *
 * pktgen_packet_dump - Dump the contents of a packet
 *
 * DESCRIPTION
 * Dump the contents of a packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_packet_dump(struct rte_mbuf *m, int pid)
{
	port_info_t *info = &pktgen.info[pid];
	int plen = (m->pkt_len + ETHER_CRC_LEN);
	unsigned char *curr_data;
	struct rte_mbuf *curr_mbuf;

	/* Checking if info->dump_tail will not overflow is done in the caller */
	if (info->dump_list[info->dump_tail].data != NULL)
		rte_free(info->dump_list[info->dump_tail].data);

	info->dump_list[info->dump_tail].data = rte_zmalloc_socket("Packet data", plen, 0, rte_socket_id());
	info->dump_list[info->dump_tail].len = plen;

	for (curr_data = info->dump_list[info->dump_tail].data, curr_mbuf = m;
	     curr_mbuf != NULL;
	     curr_data += curr_mbuf->data_len, curr_mbuf = curr_mbuf->next)
		rte_memcpy(curr_data,
			   (uint8_t *)curr_mbuf->buf_addr + m->data_off,
			   curr_mbuf->data_len);

	++info->dump_tail;
}

/**************************************************************************//**
 *
 * pktgen_packet_dump_bulk - Dump packet contents.
 *
 * DESCRIPTION
 * Dump packet contents for later inspection.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_packet_dump_bulk(struct rte_mbuf **pkts, int nb_dump, int pid)
{
	port_info_t *info = &pktgen.info[pid];
	int i;

	/* Don't dump more packets than the user asked */
	if (nb_dump > info->dump_count)
		nb_dump = info->dump_count;

	/* Don't overflow packet array */
	if (nb_dump > MAX_DUMP_PACKETS - info->dump_tail)
		nb_dump = MAX_DUMP_PACKETS - info->dump_tail;

	if (nb_dump == 0)
		return;

	for (i = 0; i < nb_dump; i++)
		pktgen_packet_dump(pkts[i], pid);

	info->dump_count -= nb_dump;
}

/**************************************************************************//**
 *
 * pktgen_print_packet_dump - Print captured packets to the screen
 *
 * DESCRIPTION
 * When some packets are captured on user request, print the packet data to
 * the screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_print_packet_dump(void)
{
	port_info_t *info;

	unsigned int pid;
	unsigned int i, j;
	unsigned char *pdata;
	uint32_t plen;
	char buff[4096];

	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		if (get_map(pktgen.l2p, pid, RTE_MAX_LCORE) == 0)
			continue;

		info = &pktgen.info[pid];
		for (; info->dump_head < info->dump_tail; ++info->dump_head) {
			pdata = (unsigned char *)info->dump_list[info->dump_head].data;
			plen = info->dump_list[info->dump_head].len;

			snprintf(buff, sizeof(buff),
				 "Port %d, packet with length %d:", pid, plen);

			for (i = 0; i < plen; i += 16) {
				strncatf(buff, "\n\t");

				/* Byte counter */
				strncatf(buff, "%06x: ", i);

				for (j = 0; j < 16; ++j) {
					/* Hex. value of character */
					if (i + j < plen)
						strncatf(buff, "%02x ",
							 pdata[i + j]);
					else
						strncatf(buff, "   ");

					/* Extra padding after 8 hex values for readability */
					if ((j + 1) % 8 == 0)
						strncatf(buff, " ");
				}

				/* Separate hex. values and raw characters */
				strncatf(buff, "\t");

				for (j = 0; j < 16; ++j)
					if (i + j < plen)
						strncatf(buff, "%c",
							 isprint(pdata[i +
								       j]) ?
							 pdata[
								 i + j] : '.');
			}
			pktgen_log_info("%s", buff);

			rte_free(info->dump_list[info->dump_head].data);
			info->dump_list[info->dump_head].data = NULL;
		}
	}
}
