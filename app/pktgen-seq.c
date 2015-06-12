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
#include "pktgen.h"

void
pktgen_send_seq_pkt(port_info_t * info, uint32_t seq_idx)
{
    (void)info;
    (void)seq_idx;
}
/**************************************************************************//**
*
* pktgen_page_seq - Display the sequence port data on the screen.
*
* DESCRIPTION
* For a given port display the sequence packet data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_page_seq(uint32_t pid)
{
    uint32_t    i, row, col;
    port_info_t * info;
    pkt_seq_t   * pkt;
    char buff[64];

    display_topline("<Sequence Page>");

    info = &pktgen.info[pid];

    row = PORT_STATE_ROW;
    col = 1;
    wr_scrn_printf(row++, col, "Port: %2d, Sequence Count: %2d of %2d  ", pid, info->seqCnt, NUM_SEQ_PKTS);
    wr_scrn_printf(row++, col, "%*s %*s%*s%*s%*s%*s%*s%*s",
            6, "Seq:",
            COLUMN_WIDTH_0, "Dst MAC",
            COLUMN_WIDTH_0, "Src MAC",
            COLUMN_WIDTH_0, "Dst IP",
            COLUMN_WIDTH_0+2, "Src IP",
            12, "Port S/D",
            15, "Protocol:VLAN",
            5, "Size");
    for(i = 0; i < NUM_SEQ_PKTS; i++) {
        col = 1;
        pkt = &info->seq_pkt[i];

        if ( i >= info->seqCnt ) {
        	wr_scrn_eol_pos(row++, col);
        	continue;
        }

        wr_scrn_printf(row, col, "%5d:", i);
        col += 7;
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
        col += COLUMN_WIDTH_1;
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
        col += COLUMN_WIDTH_1;
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr), 0xFFFFFFFF));
        col += COLUMN_WIDTH_1;
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1+2, inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr), pkt->ip_mask));
        col += COLUMN_WIDTH_1+2;

        snprintf(buff, sizeof(buff), "%d/%d", pkt->sport, pkt->dport);
        wr_scrn_printf(row, col, "%*s", 12, buff);
        col += 12;
        snprintf(buff, sizeof(buff), "%s/%s:%04x", (pkt->ethType == ETHER_TYPE_IPv4)? "IPv4" :
                                                      (pkt->ethType == ETHER_TYPE_IPv6)? "IPv6" : "Other",
                                                      (pkt->ipProto == PG_IPPROTO_TCP)? "TCP" :
                                                      (pkt->ipProto == PG_IPPROTO_ICMP)? "ICMP" : "UDP",
                                                    		  pkt->vlanid);
        wr_scrn_printf(row, col, "%*s", 15, buff);
        col += 15;
        wr_scrn_printf(row, col, "%5d", pkt->pktSize+FCS_SIZE);
        row++;
    }

    display_dashline(row+2);
}
