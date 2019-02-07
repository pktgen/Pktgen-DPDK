/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <rte_lua.h>

#include "pktgen-display.h"
#include "pktgen-log.h"

#include "pktgen.h"

#ifndef MBUF_INVALID_PORT
#if RTE_VERSION >= RTE_VERSION_NUM(17, 11, 0, 0)
#define MBUF_INVALID_PORT	UINT16_MAX
#else
#define MBUF_INVALID_PORT	UINT8_MAX
#endif
#endif

/**************************************************************************//**
 *
 * pktgen_print_pcap - Display the pcap data page.
 *
 * DESCRIPTION
 * Display the pcap data page on the screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static void
pktgen_print_pcap(uint16_t pid)
{
	uint32_t i, row, col, max_pkts, len;
	uint16_t type, vlan, skip;
	uint8_t proto;
	port_info_t *info;
	pkt_hdr_t   *hdr;
	pcap_info_t *pcap;
	pcaprec_hdr_t pcap_hdr;
	char buff[64];
	char pkt_buff[DEFAULT_MBUF_SIZE];

	pktgen_display_set_color("top.page");
	display_topline("<PCAP Page>");
	scrn_printf(1, 3, "Port %d of %d", pid, pktgen.nb_ports);

	info = &pktgen.info[pid];
	pcap = info->pcap;

	row = PORT_STATE_ROW;
	col = 1;
	if (pcap == NULL) {
		scrn_cprintf(10,
		               this_scrn->ncols,
		               "** Port does not have a PCAP file assigned **");
		row = 28;
		goto leave;
	}

	pktgen_display_set_color("stats.stat.label");
	scrn_eol_pos(row, col);
	scrn_printf(row++, col, "Port: %d, PCAP Count: %d of %d",
	               pid, pcap->pkt_idx, pcap->pkt_count);
	scrn_printf(row++, col, "%*s %*s%*s%*s%*s%*s%*s%*s",
	               5, "Seq",
	               COLUMN_WIDTH_0, "Dst MAC",
	               COLUMN_WIDTH_0, "Src MAC",
	               COLUMN_WIDTH_0, "Dst IP",
	               COLUMN_WIDTH_0 + 2, "Src IP",
	               12, "Port S/D",
	               15, "Protocol:VLAN",
	               9, "Size-FCS");

	max_pkts = pcap->pkt_idx + PCAP_PAGE_SIZE;
	if (max_pkts > pcap->pkt_count)
		max_pkts = pcap->pkt_count;

	_pcap_skip(pcap, pcap->pkt_idx);

	pktgen_display_set_color("stats.stat.values");
	for (i = pcap->pkt_idx; i < max_pkts; i++) {
		col = 1;
		skip = 0;

		len = _pcap_read(pcap, &pcap_hdr, pkt_buff, sizeof(pkt_buff));
		if (len == 0)
			break;

		/* Skip any jumbo packets larger then buffer. */
		if (pcap_hdr.incl_len > sizeof(pkt_buff) ) {
			i--;
			skip++;
		}
		/* Skip packets that are not normal IP packets. */
		type = ntohs( ((uint16_t *)pkt_buff)[6]);
		if (unlikely(type == ETHER_TYPE_VLAN) )
			type = ntohs( ((uint16_t *)pkt_buff)[8]);

		if (unlikely(type < MAX_ETHER_TYPE_SIZE) )
			skip++;

		hdr = (pkt_hdr_t *)&pkt_buff[0];

		scrn_eol_pos(row, col);

		scrn_printf(row, col, "%5d:", i);
		col += 7;
		scrn_printf(row, col, "%*s", COLUMN_WIDTH_1,
		               inet_mtoa(buff, sizeof(buff), &hdr->eth.d_addr));
		col += COLUMN_WIDTH_1;
		scrn_printf(row, col, "%*s", COLUMN_WIDTH_1,
		               inet_mtoa(buff, sizeof(buff), &hdr->eth.s_addr));
		col += COLUMN_WIDTH_1;

		type = ntohs(hdr->eth.ether_type);
		proto = hdr->u.ipv4.next_proto_id;
		vlan = 0;
		if (type == ETHER_TYPE_VLAN) {
			vlan = ntohs( ((uint16_t *)&hdr->eth.ether_type)[1]);
			type = ntohs( ((uint16_t *)&hdr->eth.ether_type)[2]);
			proto = ((struct ipv4_hdr *)((char *)&hdr->u.ipv4 + 4))->next_proto_id;
		}

		if (type == ETHER_TYPE_IPv4) {
			scrn_printf(row,
			               col,
			               "%*s",
			               COLUMN_WIDTH_1,
			               inet_ntop4(buff, sizeof(buff),
			                          hdr->u.ipv4.dst_addr,
			                          0xFFFFFFFF));
			col += COLUMN_WIDTH_1;
			scrn_printf(row,
			               col,
			               "%*s",
			               COLUMN_WIDTH_1 + 2,
			               inet_ntop4(buff, sizeof(buff),
			                          hdr->u.ipv4.src_addr,
			                          0xFFFFFFFF));
			col += COLUMN_WIDTH_1 + 2;

			snprintf(buff, sizeof(buff), "%d/%d",
			         ntohs(hdr->u.uip.udp.src_port),
			         ntohs(hdr->u.uip.udp.dst_port));
			scrn_printf(row, col, "%*s", 12, buff);
			col += 12;
		} else {
			skip++;
			col += ((2 * COLUMN_WIDTH_1) + 2 + 12);
		}
		snprintf(buff, sizeof(buff), "%s/%s:%4d",
		         (type == ETHER_TYPE_IPv4) ? "IPv4" :
		         (type == ETHER_TYPE_IPv6) ? "IPv6" : "Other",
		         (type == PG_IPPROTO_TCP) ? "TCP" :
		         (proto == PG_IPPROTO_ICMP) ? "ICMP" : "UDP",
		         (vlan & 0xFFF));
		scrn_printf(row, col, "%*s", 15, buff);
		col += 15;
		scrn_printf(row, col, "%5d", len);

		if (skip && (type < ETHER_TYPE_IPv4) )
			scrn_printf(row, col + 7, "<<< Skip %04x", type);
		else if (skip && (type != ETHER_TYPE_IPv4) )
			scrn_printf(row, col + 7, " EthType %04x", type);
		row++;
	}
leave:
	display_dashline(row + 2);
	pktgen_display_set_color(NULL);

	pktgen.flags &= ~PRINT_LABELS_FLAG;
}

/**************************************************************************//**
 *
 * pktgen_page_pcap - Display the PCAP data page.
 *
 * DESCRIPTION
 * Display the PCAP data page for a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_page_pcap(uint16_t pid)
{
	if (pktgen.flags & PRINT_LABELS_FLAG)
		pktgen_print_pcap(pid);
}

/**************************************************************************//**
 *
 * pktgen_pcap_mbuf_ctor - Callback routine to construct PCAP packets.
 *
 * DESCRIPTION
 * Callback routine to construct a set of PCAP packet buffers.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_pcap_mbuf_ctor(struct rte_mempool *mp,
		      void *opaque_arg,
		      void *_m,
		      unsigned i)
{
	struct rte_mbuf *m = _m;
	uint32_t mbuf_size, buf_len, priv_size = 0;
	pcaprec_hdr_t hdr;
	ssize_t len = -1;
	char buffer[DEFAULT_MBUF_SIZE];
	pcap_info_t *pcap = (pcap_info_t *)opaque_arg;

#if RTE_VERSION >= RTE_VERSION_NUM(16, 7, 0, 0)
	priv_size = rte_pktmbuf_priv_size(mp);
	buf_len = rte_pktmbuf_data_room_size(mp);
#else
	buf_len = mp->elt_size - sizeof(struct rte_mbuf);
#endif
	mbuf_size = sizeof(struct rte_mbuf) + priv_size;
	memset(m, 0, mbuf_size);

#if RTE_VERSION >= RTE_VERSION_NUM(17, 11, 0, 0)
	/* start of buffer is just after mbuf structure */
	m->priv_size    = priv_size;
	m->buf_addr     = (char *)m + mbuf_size;
	m->buf_iova     = rte_mempool_virt2iova(m) + mbuf_size;
	m->buf_len      = (uint16_t)buf_len;
#elif RTE_VERSION >= RTE_VERSION_NUM(16, 7, 0, 0)
	/* start of buffer is after mbuf structure and priv data */
	m->priv_size = priv_size;
	m->buf_addr = (char *)m + mbuf_size;
	m->buf_physaddr = rte_mempool_virt2phy(mp, m) + mbuf_size;
	m->buf_len = (uint16_t)buf_len;
#else
	/* start of buffer is just after mbuf structure */
	m->buf_addr     = (char *)m + sizeof(struct rte_mbuf);
	m->buf_physaddr = rte_mempool_virt2phy(mp, m->buf_addr);
	m->buf_len      = (uint16_t)buf_len;
#endif

	/* keep some headroom between start of buffer and data */
	m->data_off = RTE_MIN(RTE_PKTMBUF_HEADROOM, m->buf_len);

	/* init some constant fields */
	m->pool         = mp;
	m->nb_segs      = 1;
	m->port         = MBUF_INVALID_PORT;
	rte_mbuf_refcnt_set(m, 1);
	m->next		= NULL;

	for (;; ) {
		union pktgen_data *d = (union pktgen_data *)&m->udata64;

		if ( (i & 0x3ff) == 0) {
			scrn_printf(1, 1, "%c\b", "-\\|/"[(i >> 10) & 3]);
			i++;
		}

		if (unlikely(_pcap_read(pcap, &hdr, buffer,
					sizeof(buffer)) <= 0) ) {
			_pcap_rewind(pcap);
			continue;
		}

		len = hdr.incl_len;

		/* Adjust the packet length if not a valid size. */
		if (len < MIN_PKT_SIZE)
			len = MIN_PKT_SIZE;
		else if (len > MAX_PKT_SIZE)
			len = MAX_PKT_SIZE;

		m->data_len = len;
		m->pkt_len  = len;

		d->pkt_len = len;
		d->data_len = len;
		d->buf_len = m->buf_len;

		rte_memcpy((uint8_t *)m->buf_addr + m->data_off, buffer, len);
		break;
	}
}

/**************************************************************************//**
 *
 * pktgen_pcap_parse - Parse a PCAP file.
 *
 * DESCRIPTION
 * Parse a pcap file into packet buffers.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

int
pktgen_pcap_parse(pcap_info_t *pcap, port_info_t *info, unsigned qid)
{
	pcaprec_hdr_t hdr;
	uint32_t elt_count, data_size, len, i;
	uint64_t pkt_sizes = 0;
	char buffer[DEFAULT_MBUF_SIZE];
	char name[RTE_MEMZONE_NAMESIZE];

	if ( (pcap == NULL) || (info == NULL) )
		return -1;

	_pcap_rewind(pcap);

	snprintf(name, sizeof(name), "%-12s%d:%d", "PCAP TX", info->pid, qid);
	scrn_printf(0, 0, "    Process: %-*s ", 18, name);

	pkt_sizes = elt_count = i = 0;

	/* The pcap_open left the file pointer to the first packet. */
	while (_pcap_read(pcap, &hdr, buffer, sizeof(buffer)) > 0) {
		/* Skip any jumbo packets or packets that are too small */
		len = hdr.incl_len;

		if (len < MIN_PKT_SIZE)
			len = MIN_PKT_SIZE;
		else if (len > MAX_PKT_SIZE)
			len = MAX_PKT_SIZE;

		elt_count++;

		if ( (elt_count & 0x3ff) == 0)
			scrn_printf(1, 1, "%c\b", "-\\|/"[i++ & 3]);

		pkt_sizes += len;
	}

	/* If count is greater then zero then we allocate and create the PCAP mbuf pool. */
	if (elt_count > 0) {
		/* Create the average size packet */
		pcap->pkt_size    = (pkt_sizes / elt_count);
		pcap->pkt_count   = elt_count;
		pcap->pkt_idx     = 0;

		_pcap_rewind(pcap);

		/* Removed to allow for all of the PCAP file to be replayed */
#if 0
		/* Round up the count and size to allow for TX ring size. */
		if (elt_count < MAX_MBUFS_PER_PORT)
			elt_count = MAX_MBUFS_PER_PORT;
		elt_count = rte_align32pow2(elt_count);
#endif

		scrn_printf(0, 0, "\r    Create: %-*s   \b", 16, name);
		info->q[qid].pcap_mp = rte_mempool_create(
		                name,
		                elt_count,
		                DEFAULT_MBUF_SIZE,
		                0,
		                sizeof(struct rte_pktmbuf_pool_private),
		                rte_pktmbuf_pool_init,
		                NULL,
		                pktgen_pcap_mbuf_ctor,
		                (void *)pcap,
		                rte_lcore_to_socket_id(
		                        0),
		                MEMPOOL_F_DMA);
		scrn_printf(0, 0, "\r");
		if (info->q[qid].pcap_mp == NULL)
			pktgen_log_panic("Cannot init port %d for %d PCAP packets",
					 info->pid, pcap->pkt_count);

		data_size = (pcap->pkt_count * DEFAULT_MBUF_SIZE);
		scrn_printf(0, 0,
			"    Create: %-*s - Number of MBUFs %6u for %5d packets                 = %6u KB\n",
			16,
			name,
			elt_count,
			pcap->pkt_count,
			(data_size + 1023) / 1024);
		pktgen.mem_used         += data_size;
		pktgen.total_mem_used   += data_size;

		pktgen_set_port_flags(info, SEND_PCAP_PKTS);
	}

	pktgen_packet_rate(info);
	return 0;
}
