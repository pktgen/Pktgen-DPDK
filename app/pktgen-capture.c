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

#include "pktgen-capture.h"
#include <time.h>

#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_string_fns.h>

#include "pktgen-cmds.h"
#include "pktgen-log.h"
#include "pktgen-display.h"

/**************************************************************************//**
*
* pktgen_packet_capture_init - Initialize memory and data structures for packet
* capture.
*
* DESCRIPTION
* Initialization of memory zones and data structures for packet capture
* storage.
*
* PARAMETERS:
* capture: capture_t struct that will keep a pointer to the allocated memzone
* socket_id: Socket where the memzone will be reserved
*
* RETURNS: N/A
*
* SEE ALSO:
*/
void pktgen_packet_capture_init(capture_t *capture, int socket_id)
{
	char memzone_name[RTE_MEMZONE_NAMESIZE];

	if (!capture)
		return;

	capture->lcore	= RTE_MAX_LCORE;
	capture->port	= RTE_MAX_ETHPORTS;
	capture->used	= 0;

	snprintf(memzone_name, sizeof(memzone_name), "Capture_MZ_%d", socket_id);
	capture->mz = rte_memzone_reserve(memzone_name, 0, socket_id,
			RTE_MEMZONE_1GB | RTE_MEMZONE_SIZE_HINT_ONLY);
}


/**************************************************************************//**
*
* pktgen_set_capture - Enable or disable packet capturing
*
* DESCRIPTION
* Set up packet capturing for the given ports and make sure only 1 port per
* socket is in capture mode.
*
* PARAMETERS:
* info: port to capture from
* onOff: enable or disable capturing?
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_capture(port_info_t * info, uint32_t onOff)
{
	capture_t	* cap;

	if ( onOff == ENABLE_STATE ) {
		/* Enabling an aleady enabled port is a no-op */
		if ( rte_atomic32_read(&info->port_flags) & CAPTURE_PKTS )
			return;

		if (wr_get_port_rxcnt(pktgen.l2p, info->pid) == 0) {
			pktgen_log_warning("Port %d has no RX queue: capture is not possible", info->pid);
			return;
		}

		// Find an lcore that can capture packets for the requested port
		uint8_t lid_idx, lid, rxid;
		for (lid_idx = 0; lid_idx < wr_get_port_nb_lids(pktgen.l2p, info->pid); ++lid_idx) {
			lid = wr_get_port_lid(pktgen.l2p, info->pid, lid_idx);
			for (rxid = 0; rxid < wr_get_lcore_rxcnt(pktgen.l2p, lid); ++rxid)
				if (wr_get_rx_pid(pktgen.l2p, lid, rxid) == info->pid)
					goto found_rx_lid;
		}
		lid = RTE_MAX_LCORE;

found_rx_lid:
		if (lid == RTE_MAX_LCORE) {
			pktgen_log_warning("Port %d has no rx lcore: capture is not possible", info->pid);
			return;
		}

		// Get socket of the selected lcore and check if capturing is possible
		uint8_t sid = pktgen.core_info[lid].s.socket_id;
		if (pktgen.capture[sid].mz == NULL) {
			pktgen_log_warning("No memory allocated for capturing on socket %d, are hugepages allocated on this socket?", sid);
			return;
		}

		cap = &pktgen.capture[sid];

		if (cap->lcore != RTE_MAX_LCORE) {
			pktgen_log_warning("Lcore %d is already capturing on socket %d and only 1 lcore can capture on a socket.\nDisable capturing on the port associated with this lcore first.", cap->lcore, sid);
			return;
		}

		// Everything checks out: enable packet capture
		cap->used	= 0;
		cap->lcore	= lid;
		cap->port	= info->pid;
		cap->tail	= (cap_hdr_t *)cap->mz->addr;
		cap->end	= (cap_hdr_t *)((char *)cap->mz->addr + (cap->mz->len - sizeof(cap_hdr_t)));

		// Write end-of-data sentinel to start of capture memory. This
		// effectively clears previously captured data.
		memset(cap->tail, 0, sizeof(cap_hdr_t));
		memset(cap->end, 0, sizeof(cap_hdr_t));

		pktgen_set_port_flags(info, CAPTURE_PKTS);

		pktgen_log_info("Capturing on port %d, lcore %d, socket %d; buffer size: %.2f MB (~%.2f seconds for 64 byte packets at line rate)",
				info->pid, lid, sid,
				(double)cap->mz->len / (1024 * 1024),
				(double)cap->mz->len /
					(66 /* 64 bytes payload + 2 bytes for payload size */
					* ((double)info->link.link_speed * 1000 * 1000 / 8) /* Xbit -> Xbyte */
					/ 84) /* 64 bytes payload + 20 byte etherrnet frame overhead: 84 bytes per packet */
		);
	}
	else {
		if (!(rte_atomic32_read(&info->port_flags) & CAPTURE_PKTS))
			return;

		// This should never happen: capture cannot have been enabled when
		// this condition is true.
		if (wr_get_port_rxcnt(pktgen.l2p, info->pid) == 0) {
			pktgen_log_warning("Port %d has no RX queue: capture is not possible", info->pid);
			return;
		}

		int sid;
		for (sid = 0; sid < RTE_MAX_NUMA_NODES; ++sid) {
			cap = &pktgen.capture[sid];
			if (cap->mz && (cap->port == info->pid))
				break;
		}

		// This should never happen.
		if (sid == RTE_MAX_NUMA_NODES) {
			pktgen_log_error("Could not find socket for port %d", info->pid);
			return;
		}

		/* If there is previously captured data in the buffer, write it to disk. */
		if (cap->used > 0) {
			pcap_t *pcap;
			pcap_dumper_t *pcap_dumper;
			struct pcap_pkthdr pcap_hdr;
			cap_hdr_t * hdr;
			time_t	t;
			char	filename[64];
			char	str_time[64];
			size_t mem_dumped = 0;
			unsigned int pct = 0;

			char status[256];
			sprintf(status, "\r    Dumping ~%.2fMB of captured data to disk: 0%%",
					(double)cap->used / (1024 * 1024));
			rte_printf_status("\n%s", status);

			pcap = pcap_open_dead(DLT_EN10MB, 65535);

			t = time(NULL);
			strftime(str_time, sizeof(str_time), "%Y%m%d-%H%M%S", localtime(&t));
			snprintf(filename, sizeof(filename), "pktgen-%s-%d.pcap", str_time, cap->port);
			pcap_dumper = pcap_dump_open(pcap, filename);

			hdr = (cap_hdr_t *)cap->mz->addr;

			while ( hdr->pkt_len ) {

				pcap_hdr.ts.tv_sec = 0 ;	// FIXME use real timestamp
				pcap_hdr.ts.tv_usec = 0 ;	// FIXME use real timestamp
				pcap_hdr.len    = hdr->pkt_len;
				pcap_hdr.caplen = hdr->data_len;

				pcap_dump((u_char *)pcap_dumper, &pcap_hdr, (const u_char *)hdr->pkt);

				hdr = (cap_hdr_t *)(hdr->pkt + hdr->data_len);

				mem_dumped = hdr->pkt - (unsigned char *)cap->mz->addr;

				// The amount of data to dump to disk, is potentially very large
				// (a few gigabytes), so print a percentage counter.
				if ( pct < ((mem_dumped * 100) / cap->used) ) {
					pct = (mem_dumped * 100) / cap->used;

					if (pct % 10 == 0)
						strncatf(status, "%d%%", pct);
					else if (pct % 2 == 0)
						strncatf(status, ".");

					rte_printf_status("%s", status);
				}
			}
			rte_printf_status("\r");
			rte_printf_status("\n");	// Clean of the screen a bit

			pcap_dump_close(pcap_dumper);
			pcap_close(pcap);
		}

		cap->used	= 0;
		cap->tail	= (cap_hdr_t *)cap->mz->addr;
		cap->lcore	= RTE_MAX_LCORE;
		cap->port	= RTE_MAX_ETHPORTS;

		pktgen_clr_port_flags(info, CAPTURE_PKTS);
	}
}



/**************************************************************************//**
*
* pktgen_packet_capture_bulk - Capture packets to memory.
*
* DESCRIPTION
* Capture packet contents to memory, so they can be written to disk later.
*
* A captured packet is stored as follows:
* - uint16_t: untruncated packet length
* - uint16_t: size of actual packet contents that are stored
* - unsigned char[]: packet contents (number of bytes stored equals previous
*       uint16_t)
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_packet_capture_bulk(struct rte_mbuf ** pkts, uint32_t nb_dump, capture_t * cap )
{
	uint32_t plen, i;
	struct rte_mbuf *pkt;

	/* Don't capture if buffer is full */
	if (cap->tail == cap->end)
		return;

	for (i = 0; i < nb_dump; i++) {

		pkt = pkts[i];

		/* If the packet is segmented by DPDK, only the contents of the first
		 * segment are captured. Capturing all segments uses too much CPU
		 * cycles, which causes packets to be dropped.
		 * Hence, data_len is used instead of pkt_len. */
		plen = (pkt->data_len + 1) & ~1;

		/* If packet to capture is larger than available buffer size, stop
		 * capturing.
		 * The packet data is prepended by the untruncated packet length and
		 * the amount of captured data (which can be less than the packet size
		 * if DPDK has stored the packet contents in segmented mbufs).
		 */
		if ((cap_hdr_t *)(cap->tail->pkt + plen) > cap->end)
			break;

		/* Write untruncated data length and size of the actually captured
		 * data. */
		cap->tail->pkt_len	= pkt->pkt_len;
		cap->tail->data_len	= plen;

		rte_memcpy(cap->tail->pkt, (uint8_t *)pkt->buf_addr + pkt->data_off, pkt->pkt_len);
		cap->tail = (cap_hdr_t *)(cap->tail->pkt + plen);
	}

	/* Write end-of-data sentinel */
	cap->tail->pkt_len = 0;
	cap->used = (unsigned char *)cap->tail - (unsigned char *)cap->mz->addr;
}
