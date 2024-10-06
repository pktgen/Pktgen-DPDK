/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <lua_config.h>

#include "pktgen-display.h"
#include "pktgen.h"

void
pktgen_send_seq_pkt(port_info_t *pinfo, uint32_t seq_idx)
{
    (void)pinfo;
    (void)seq_idx;
}

/**
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
    uint32_t i, row, col, sav;
    port_info_t *pinfo;
    pkt_seq_t *pkt;
    char buff[128];

    display_topline("<Sequence Page>", 0, 0, 0);

    pinfo = l2p_get_port_pinfo(pid);
    if (pinfo == NULL)
        return;

    pktgen_display_set_color("top.ports");
    row = PORT_FLAGS_ROW;
    col = 1;
    scrn_printf(row, col, "Port: %2d, Sequence Count: %2d of %2d  ", pid, pinfo->seqCnt,
                NUM_SEQ_PKTS);
    pktgen_display_set_color("stats.stat.label");
    scrn_printf(row++, col + 102, "GTP-u");
    scrn_printf(row++, col, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s", 3, "Seq", 15, "Dst MAC", 15,
                "Src MAC", 16, "Dst IP", 18, "Src IP", 13, "TTL/Port S/D", 20,
                "Protocol:VLAN:Flags", 8, "CoS/ToS", 5, "TEID", 17, "Flag/Group/vxlan", 6, "Size");

    pktgen_display_set_color("stats.stat.values");
    scrn_fgcolor(SCRN_DEFAULT_FG, SCRN_NO_ATTR);
    sav = row;
    for (i = 0; i <= NUM_SEQ_PKTS; i++) {
        if (i >= pinfo->seqCnt)
            scrn_eol_pos(row++, col);
    }
    row = sav;
    for (i = 0; i < pinfo->seqCnt; i++) {
        pkt = &pinfo->seq_pkt[i];

        col = 1;
        scrn_printf(row, col, "%2d:", i);

        col += 3;
        snprintf(buff, sizeof(buff), "%02x%02x:%02x%02x:%02x%02x", pkt->eth_dst_addr.addr_bytes[0],
                 pkt->eth_dst_addr.addr_bytes[1], pkt->eth_dst_addr.addr_bytes[2],
                 pkt->eth_dst_addr.addr_bytes[3], pkt->eth_dst_addr.addr_bytes[4],
                 pkt->eth_dst_addr.addr_bytes[5]);
        scrn_printf(row, col, "%*s", 15, buff);

        col += 15;
        snprintf(buff, sizeof(buff), "%02x%02x:%02x%02x:%02x%02x", pkt->eth_src_addr.addr_bytes[0],
                 pkt->eth_src_addr.addr_bytes[1], pkt->eth_src_addr.addr_bytes[2],
                 pkt->eth_src_addr.addr_bytes[3], pkt->eth_src_addr.addr_bytes[4],
                 pkt->eth_src_addr.addr_bytes[5]);
        scrn_printf(row, col, "%*s", 15, buff);

        col += 15;
        scrn_printf(
            row, col, "%*s", 16,
            inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr.addr.ipv4.s_addr), 0xFFFFFFFF));

        col += 16;
        scrn_printf(
            row, col, "%*s", 18,
            inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr.addr.ipv4.s_addr), pkt->ip_mask));

        col += 18;
        snprintf(buff, sizeof(buff), "%d/%d/%d", pkt->ttl, pkt->sport, pkt->dport);
        scrn_printf(row, col, "%*s", 13, buff);

        col += 13;
        snprintf(buff, sizeof(buff), "%s/%s:%04x:%04x",
                 (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "IPv4"
                 : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "IPv6"
                                                         : "....",
                 (pkt->ipProto == PG_IPPROTO_TCP)    ? "TCP"
                 : (pkt->ipProto == PG_IPPROTO_ICMP) ? "ICMP"
                                                     : "UDP",
                 pkt->vlanid, pkt->tcp_flags);
        scrn_printf(row, col, "%*s", 20, buff);

        col += 20;
        snprintf(buff, sizeof(buff), "%3d/%3d", pkt->cos, pkt->tos);
        scrn_printf(row, col, "%*s", 8, buff);

        col += 8;
        scrn_printf(row, col, "%5d", pkt->gtpu_teid);

        col += 6;
        snprintf(buff, sizeof(buff), "%04x/%5d/%5d", pkt->vni_flags, pkt->group_id, pkt->vxlan_id);
        scrn_printf(row, col, "%*s", 16, buff);

        col += 16;
        scrn_printf(row, col, "%6d", pkt->pkt_size + RTE_ETHER_CRC_LEN);

        row++;
    }

    display_dashline(row + 2);
    pktgen_display_set_color(NULL);
}
