/*-
 * Copyright (c) <2010>, Intel Corporation
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
 * 4) The screens displayed by the application must contain the copyright notice as defined
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
/* Created 2010 by Keith Wiles @ intel.com */

#include "pktgen-display.h"
#include "pktgen-log.h"

#include "pktgen.h"

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
    uint32_t    i, row, col, max_pkts, len;
    uint16_t	type, vlan, skip;
    uint8_t		proto;
    port_info_t * info;
    pkt_hdr_t   * hdr;
    pcap_info_t * pcap;
    pcaprec_hdr_t    pcap_hdr;
    char buff[64];
    char pkt_buff[2048];

    display_topline("<PCAP Page>");
    wr_scrn_printf(1, 3, "Port %d of %d", pid, pktgen.nb_ports);

    info = &pktgen.info[pid];
    pcap = info->pcap;

    row = PORT_STATE_ROW;
    col = 1;
    if ( pcap == NULL ) {
    	wr_scrn_center(10, pktgen.scrn->ncols, "** Port does not have a PCAP file assigned **");
    	row = 28;
    	goto leave;
    }

    wr_scrn_eol_pos(row, col);
    wr_scrn_printf(row++, col, "Port: %d, PCAP Count: %d of %d",
    		pid, pcap->pkt_idx, pcap->pkt_count);
    wr_scrn_printf(row++, col, "%*s %*s%*s%*s%*s%*s%*s%*s",
            5, "Seq",
            COLUMN_WIDTH_0, "Dst MAC",
            COLUMN_WIDTH_0, "Src MAC",
            COLUMN_WIDTH_0, "Dst IP",
            COLUMN_WIDTH_0+2, "Src IP",
            12, "Port S/D",
            15, "Protocol:VLAN",
            9, "Size-FCS");

    max_pkts = pcap->pkt_idx + PCAP_PAGE_SIZE;
    if ( max_pkts > pcap->pkt_count )
    	max_pkts = pcap->pkt_count;

    wr_pcap_skip(pcap, pcap->pkt_idx);

    for(i = pcap->pkt_idx; i < max_pkts; i++) {
        col = 1;
        skip = 0;

        len = wr_pcap_read(pcap, &pcap_hdr, pkt_buff, sizeof(pkt_buff));
        if ( len == 0 )
        	break;

    	// Skip any jumbo packets larger then buffer.
        if ( pcap_hdr.incl_len > sizeof(pkt_buff) ) {
    		i--;
    		skip++;
        }
    	// Skip packets that are not normal IP packets.
    	type = ntohs( ((uint16_t *)pkt_buff)[6] );
    	if ( unlikely(type == ETHER_TYPE_VLAN) )
    		type = ntohs( ((uint16_t *)pkt_buff)[8] );

        if ( unlikely(type < MAX_ETHER_TYPE_SIZE) )
            skip++;

        hdr = (pkt_hdr_t *)&pkt_buff[0];

       	wr_scrn_eol_pos(row, col);

        wr_scrn_printf(row, col, "%5d:", i);
        col += 7;
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, inet_mtoa(buff, sizeof(buff), &hdr->eth.d_addr));
        col += COLUMN_WIDTH_1;
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, inet_mtoa(buff, sizeof(buff), &hdr->eth.s_addr));
        col += COLUMN_WIDTH_1;

        type = ntohs(hdr->eth.ether_type);
        proto = hdr->u.ipv4.proto;
        vlan = 0;
        if ( type == ETHER_TYPE_VLAN ) {
        	vlan = ntohs( ((uint16_t *)&hdr->eth.ether_type)[1] );
        	type = ntohs( ((uint16_t *)&hdr->eth.ether_type)[2] );
        	proto = ((ipHdr_t *)((char *)&hdr->u.ipv4 + 4))->proto;
        }

        if ( type == ETHER_TYPE_IPv4 ) {
			wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, inet_ntop4(buff, sizeof(buff), hdr->u.ipv4.dst, 0xFFFFFFFF));
			col += COLUMN_WIDTH_1;
			wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1+2, inet_ntop4(buff, sizeof(buff), hdr->u.ipv4.src, 0xFFFFFFFF));
			col += COLUMN_WIDTH_1+2;

	        snprintf(buff, sizeof(buff), "%d/%d", ntohs(hdr->u.uip.udp.sport), ntohs(hdr->u.uip.udp.dport));
	        wr_scrn_printf(row, col, "%*s", 12, buff);
	        col += 12;
        } else {
        	skip++;
        	col += ((2 * COLUMN_WIDTH_1) + 2 + 12);
        }
        snprintf(buff, sizeof(buff), "%s/%s:%4d", (type == ETHER_TYPE_IPv4)? "IPv4" :
                                                   (type == ETHER_TYPE_IPv6)? "IPv6" : "Other",
                                                   (type == PG_IPPROTO_TCP)? "TCP" :
                                                   (proto == PG_IPPROTO_ICMP)? "ICMP" : "UDP",
                                                   (vlan & 0xFFF));
        wr_scrn_printf(row, col, "%*s", 15, buff);
        col += 15;
        wr_scrn_printf(row, col, "%5d", len);

        if ( skip && (type < ETHER_TYPE_IPv4) )
        	wr_scrn_printf(row, col+7, "<<< Skip %04x", type);
        else if ( skip && (type != ETHER_TYPE_IPv4) )
        	wr_scrn_printf(row, col+7, " EthType %04x", type);
        row++;
    }
leave:
    display_dashline(row+2);

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
    if ( pktgen.flags & PRINT_LABELS_FLAG )
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
pktgen_pcap_mbuf_ctor(struct rte_mempool *mp, void *opaque_arg, void *_m, unsigned i)
{
    struct rte_mbuf *m = _m;
	uint32_t	buf_len = mp->elt_size - sizeof(struct rte_mbuf);
    pcaprec_hdr_t hdr;
    ssize_t	len = -1;
    char buffer[2048];
    pcap_info_t * pcap = (pcap_info_t *)opaque_arg;

	RTE_MBUF_ASSERT(mp->elt_size >= sizeof(struct rte_mbuf));

	memset(m, 0, mp->elt_size);

    /* start of buffer is just after mbuf structure */
    m->buf_addr		= (char *)m + sizeof(struct rte_mbuf);
    m->buf_physaddr = rte_mempool_virt2phy(mp, m->buf_addr);
    m->buf_len		= (uint16_t)buf_len;

    /* keep some headroom between start of buffer and data */
    m->data_off = RTE_MIN(RTE_PKTMBUF_HEADROOM, m->buf_len);

    /* init some constant fields */
    m->pool         = mp;
    m->nb_segs  	= 1;
    m->port			= 0xff;
    m->ol_flags		= 0;

    for(;;) {
        if ( (i & 0x3ff) == 0 ) {
			rte_printf_status("%c\b", "-\\|/"[(i >> 10) & 3]);
        	i++;
        }

    	if ( unlikely(wr_pcap_read(pcap, &hdr, buffer, sizeof(buffer)) <= 0) ) {
    		wr_pcap_rewind(pcap);
    		continue;
    	}

        len = hdr.incl_len;

        // Adjust the packet length if not a valid size.
        if ( len < (ETHER_MIN_LEN - 4) )
    		len = (ETHER_MIN_LEN - 4);
        else if ( len > (ETHER_MAX_LEN - 4) )
    		len = (ETHER_MAX_LEN - 4);

        m->data_len = len;
        m->pkt_len	= len;

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
pktgen_pcap_parse(pcap_info_t * pcap, port_info_t * info, unsigned qid)
{
    pcaprec_hdr_t hdr;
    uint32_t    elt_count, data_size, len, i;
    uint64_t	pkt_sizes = 0;
    char		buffer[2048];
	char		name[RTE_MEMZONE_NAMESIZE];

    if ( (pcap == NULL) || (info == NULL) )
        return -1;

    wr_pcap_rewind(pcap);

	snprintf(name, sizeof(name), "%-12s%d:%d", "PCAP TX", info->pid, 0);
    rte_printf_status("    Process: %-*s ", 18, name);

    pkt_sizes = elt_count = i = 0;

    // The wr_pcap_open left the file pointer to the first packet.
    while( wr_pcap_read(pcap, &hdr, buffer, sizeof(buffer)) > 0 ) {

    	// Skip any jumbo packets or packets that are too small
        len = hdr.incl_len;

        if ( len < (ETHER_MIN_LEN - 4) )
    		len = (ETHER_MIN_LEN - 4);
        else if ( len > (ETHER_MAX_LEN - 4) )
    		len = (ETHER_MAX_LEN - 4);

        elt_count++;

        if ( (elt_count & 0x3ff) == 0 )
			rte_printf_status("%c\b", "-\\|/"[i++ & 3]);

        pkt_sizes += len;
    }

    // If count is greater then zero then we allocate and create the PCAP mbuf pool.
    if ( elt_count > 0 ) {
    	// Create the average size packet
		info->pcap->pkt_size	= (pkt_sizes / elt_count);
        info->pcap->pkt_count	= elt_count;
        info->pcap->pkt_idx		= 0;

        wr_pcap_rewind(pcap);

        // Round up the count and size to allow for TX ring size.
        if ( elt_count < MAX_MBUFS_PER_PORT )
        	elt_count = MAX_MBUFS_PER_PORT;
        elt_count = rte_align32pow2(elt_count);

        rte_printf_status("\r    Create: %-*s   \b", 16, name);
    	info->q[qid].pcap_mp = rte_mempool_create(name, elt_count, MBUF_SIZE, 0,
                   sizeof(struct rte_pktmbuf_pool_private),
                   rte_pktmbuf_pool_init, NULL,
                   pktgen_pcap_mbuf_ctor, (void *)pcap,
                   rte_lcore_to_socket_id(0), MEMPOOL_F_DMA);
        rte_printf_status("\r");
        if ( info->q[qid].pcap_mp == NULL )
            pktgen_log_panic("Cannot init port %d for PCAP packets", info->pid);

        data_size = (info->pcap->pkt_count * MBUF_SIZE);
        rte_printf_status("    Create: %-*s - Number of MBUFs %6u for %5d packets                 = %6u KB\n",
                16, name, elt_count, info->pcap->pkt_count, (data_size + 1023)/1024);
        pktgen.mem_used			+= data_size;
        pktgen.total_mem_used	+= data_size;

        pktgen_set_port_flags(info, SEND_PCAP_PKTS);
    }

    pktgen_packet_rate(info);
    return 0;
}
