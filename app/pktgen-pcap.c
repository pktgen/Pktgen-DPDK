/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <lua_config.h>

#include "pktgen-display.h"
#include "pktgen-log.h"

#include "pktgen.h"

#ifndef MBUF_INVALID_PORT
#define MBUF_INVALID_PORT UINT16_MAX
#endif

void
pktgen_pcap_info(pcap_info_t *pcap, uint16_t port, int flag)
{
    printf("\nPCAP file for port %d: %s\n", port, pcap->filename);
    printf("  magic: %08x,", pcap->info.magic_number);
    printf(" Version: %d.%d,", pcap->info.version_major, pcap->info.version_minor);
    printf(" Zone: %d,", pcap->info.thiszone);
    printf(" snaplen: %d,", pcap->info.snaplen);
    printf(" sigfigs: %d,", pcap->info.sigfigs);
    printf(" network: %d", pcap->info.network);
    printf(" Endian: %s\n", pcap->convert ? "Big" : "Little");
    if (flag)
        printf("  Packet count: %d\n", pcap->pkt_count);
    printf("\n");
    fflush(stdout);
}

static __inline__ void
pcap_convert(pcap_info_t *pcap, pcaprec_hdr_t *pHdr)
{
    if (pcap->convert) {
        pHdr->incl_len = ntohl(pHdr->incl_len);
        pHdr->orig_len = ntohl(pHdr->orig_len);
        pHdr->ts_sec   = ntohl(pHdr->ts_sec);
        pHdr->ts_usec  = ntohl(pHdr->ts_usec);
    }
}

static void
pcap_rewind(pcap_info_t *pcap)
{
    /* Rewind to the beginning */
    rewind(pcap->fp);

    /* Seek past the pcap header */
    (void)fseek(pcap->fp, sizeof(pcap_hdr_t), SEEK_SET);
}

static void
pcap_get_info(pcap_info_t *pcap)
{
    pcaprec_hdr_t hdr;
    if (fread(&pcap->info, 1, sizeof(pcap_hdr_t), pcap->fp) != sizeof(pcap_hdr_t))
        rte_exit(EXIT_FAILURE, "%s: failed to read pcap header\n", __func__);

    /* Make sure we have a valid PCAP file for Big or Little Endian formats. */
    if (pcap->info.magic_number != PCAP_MAGIC_NUMBER)
        pcap->convert = 0;
    else if (pcap->info.magic_number != ntohl(PCAP_MAGIC_NUMBER))
        pcap->convert = 1;
    else
        rte_exit(EXIT_FAILURE, "%s: invalid magic number 0x%08x\n", __func__,
                 pcap->info.magic_number);

    if (pcap->convert) {
        pcap->info.magic_number  = ntohl(pcap->info.magic_number);
        pcap->info.version_major = ntohs(pcap->info.version_major);
        pcap->info.version_minor = ntohs(pcap->info.version_minor);
        pcap->info.thiszone      = ntohl(pcap->info.thiszone);
        pcap->info.sigfigs       = ntohl(pcap->info.sigfigs);
        pcap->info.snaplen       = ntohl(pcap->info.snaplen);
        pcap->info.network       = ntohl(pcap->info.network);
    }

    /* count the number of packets and get the largest size packet */
    for (;;) {
        if (fread(&hdr, 1, sizeof(pcaprec_hdr_t), pcap->fp) != sizeof(hdr))
            break;

        /* Convert the packet header to the correct format if needed */
        pcap_convert(pcap, &hdr);

        if (fseek(pcap->fp, hdr.incl_len, SEEK_CUR) < 0)
            break;

        pcap->pkt_count++;
        if (hdr.incl_len > pcap->max_pkt_size)
            pcap->max_pkt_size = hdr.incl_len;
    }
    pcap_rewind(pcap);
}

static __inline__ void
mbuf_iterate_cb(struct rte_mempool *mp, void *opaque, void *obj, unsigned obj_idx __rte_unused)
{
    pcap_info_t *pcap  = (pcap_info_t *)opaque;
    struct rte_mbuf *m = (struct rte_mbuf *)obj;
    pcaprec_hdr_t hdr;

    if (fread(&hdr, 1, sizeof(pcaprec_hdr_t), pcap->fp) != sizeof(hdr))
        rte_exit(EXIT_FAILURE, "%s: failed to read pcap header\n", __func__);

    pcap_convert(pcap, &hdr); /* Convert the packet header to the correct format. */

    if (fread(rte_pktmbuf_mtod(m, char *), 1, hdr.incl_len, pcap->fp) == 0)
        rte_exit(EXIT_FAILURE, "%s: failed to read packet data from PCAP file\n", __func__);

    m->pool     = mp;
    m->next     = NULL;
    m->data_len = hdr.incl_len;
    m->pkt_len  = hdr.incl_len;
    m->port     = 0;
    m->ol_flags = 0;
}

pcap_info_t *
pktgen_pcap_open(char *filename, uint16_t pid)
{
    pcap_info_t *pcap = NULL;
    struct rte_mempool *mp;
    char name[64] = {0};
    uint16_t sid;

    if (filename == NULL)
        rte_exit(EXIT_FAILURE, "%s: PCAP filename is NULL\n", __func__);

    sid = rte_eth_dev_socket_id(pid);

    snprintf(name, sizeof(name), "PCAP-Info-%d", pid);
    pcap = (pcap_info_t *)rte_zmalloc_socket(name, sizeof(pcap_info_t), RTE_CACHE_LINE_SIZE, sid);
    if (pcap == NULL)
        rte_exit(EXIT_FAILURE, "%s: rte_zmalloc_socket() failed for pcap_info_t structure\n",
                 __func__);

    /* Default to little endian format. */
    pcap->filename = strdup(filename);

    /* Read the pcap file trailer. */
    pcap->fp = fopen((const char *)filename, "r");
    if (pcap->fp == NULL)
        rte_exit(EXIT_FAILURE, "%s: failed for (%s)\n", __func__, filename);

    pcap_get_info(pcap);

    snprintf(name, sizeof(name), "pcap-%d", pid);
    mp = rte_pktmbuf_pool_create(name, pcap->pkt_count, 0, DEFAULT_PRIV_SIZE, pcap->max_pkt_size,
                                 sid);
    if (mp == NULL)
        rte_exit(EXIT_FAILURE,
                 "Cannot create mbuf pool (%s) port %d, nb_mbufs %d, socket_id %d: %s", name, pid,
                 pcap->pkt_count, sid, rte_strerror(rte_errno));

    pcap->mp = mp;

    rte_mempool_obj_iter(mp, mbuf_iterate_cb, pcap);

    return pcap;
}

void
pktgen_pcap_close(pcap_info_t *pcap)
{
    if (pcap == NULL)
        return;

    if (pcap->filename)
        free(pcap->filename);
    if (pcap->fp)
        fclose(pcap->fp);
    if (pcap->mp)
        rte_mempool_free(pcap->mp);
    rte_free(pcap);
}

/**
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
#if 0
    uint32_t i, row, col, max_pkts, len;
    uint16_t type, vlan, skip;
    uint8_t proto;
    port_info_t *pinfo;
    l2p_port_t *port;
    pkt_hdr_t *hdr;
    pcap_info_t *pcap;
    pcaprec_hdr_t pcap_hdr;
    char buff[64];
    char pkt_buff[DEFAULT_MBUF_SIZE];
#endif

    display_topline("<PCAP Page>", pid, pid, pktgen.nb_ports);

#if 0
    pinfo = l2p_get_port_private(pid);
    port  = l2p_get_port(pid);
    pcap  = port->pcap_info;

    row = PORT_FLAGS_ROW;
    col = 1;
    if (pcap == NULL) {
        scrn_cprintf(10, this_scrn->ncols, "** Port does not have a PCAP file assigned **");
        row = 28;
        goto leave;
    }

    pktgen_display_set_color("stats.stat.label");
    scrn_eol_pos(row, col);
    scrn_printf(row++, col, "Port: %d, PCAP Count: %d of %d", pid, pcap->pkt_index, pcap->pkt_count);
    scrn_printf(row++, col, "%*s %*s%*s%*s%*s%*s%*s%*s", 5, "Seq", COLUMN_WIDTH_0, "Dst MAC",
                COLUMN_WIDTH_0, "Src MAC", COLUMN_WIDTH_0, "Dst IP", COLUMN_WIDTH_0 + 2, "Src IP",
                12, "Port S/D", 15, "Protocol:VLAN", 9, "Size-FCS");

    max_pkts = pcap->pkt_index + PCAP_PAGE_SIZE;
    if (max_pkts > pcap->pkt_count)
        max_pkts = pcap->pkt_count;

    pcap_skip(pcap, pcap->pkt_index);

    pktgen_display_set_color("stats.stat.values");
    for (i = pcap->pkt_index; i < max_pkts; i++) {
        col  = 1;
        skip = 0;

        len = pcap_read(pcap, &pcap_hdr, pkt_buff, sizeof(pkt_buff));
        if (len == 0)
            break;

        /* Skip any jumbo packets larger then buffer. */
        if (pcap_hdr.incl_len > sizeof(pkt_buff)) {
            i--;
            skip++;
        }
        /* Skip packets that are not normal IP packets. */
        type = ntohs(((uint16_t *)pkt_buff)[6]);
        if (unlikely(type == RTE_ETHER_TYPE_VLAN))
            type = ntohs(((uint16_t *)pkt_buff)[8]);

        if (unlikely(type < MAX_ETHER_TYPE_SIZE))
            skip++;

        hdr = (pkt_hdr_t *)&pkt_buff[0];

        scrn_eol_pos(row, col);

        scrn_printf(row, col, "%5d:", i);
        col += 7;
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_1,
                    inet_mtoa(buff, sizeof(buff), &hdr->eth.dst_addr));
        col += COLUMN_WIDTH_1;
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_1,
                    inet_mtoa(buff, sizeof(buff), &hdr->eth.src_addr));
        col += COLUMN_WIDTH_1;

        type  = ntohs(hdr->eth.ether_type);
        proto = hdr->u.ipv4.next_proto_id;
        vlan  = 0;
        if (type == RTE_ETHER_TYPE_VLAN) {
            vlan  = ntohs(((uint16_t *)&hdr->eth.ether_type)[1]);
            type  = ntohs(((uint16_t *)&hdr->eth.ether_type)[2]);
            proto = ((struct rte_ipv4_hdr *)((char *)&hdr->u.ipv4 + 4))->next_proto_id;
        }

        if (type == RTE_ETHER_TYPE_IPV4) {
            scrn_printf(row, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop4(buff, sizeof(buff), hdr->u.ipv4.dst_addr, 0xFFFFFFFF));
            col += COLUMN_WIDTH_1;
            scrn_printf(row, col, "%*s", COLUMN_WIDTH_1 + 2,
                        inet_ntop4(buff, sizeof(buff), hdr->u.ipv4.src_addr, 0xFFFFFFFF));
            col += COLUMN_WIDTH_1 + 2;

            snprintf(buff, sizeof(buff), "%d/%d", ntohs(hdr->u.uip.udp.src_port),
                     ntohs(hdr->u.uip.udp.dst_port));
            scrn_printf(row, col, "%*s", 12, buff);
            col += 12;
        } else {
            skip++;
            col += ((2 * COLUMN_WIDTH_1) + 2 + 12);
        }
        snprintf(buff, sizeof(buff), "%s/%s:%4d",
                 (type == RTE_ETHER_TYPE_IPV4)   ? "IPv4"
                 : (type == RTE_ETHER_TYPE_IPV6) ? "IPv6"
                                                 : "Other",
                 (type == PG_IPPROTO_TCP)     ? "TCP"
                 : (proto == PG_IPPROTO_ICMP) ? "ICMP"
                                              : "UDP",
                 (vlan & 0xFFF));
        scrn_printf(row, col, "%*s", 15, buff);
        col += 15;
        scrn_printf(row, col, "%5d", len);

        if (skip && (type < RTE_ETHER_TYPE_IPV4))
            scrn_printf(row, col + 7, "<<< Skip %04x", type);
        else if (skip && (type != RTE_ETHER_TYPE_IPV4))
            scrn_printf(row, col + 7, " EthType %04x", type);
        row++;
    }
leave:
    display_dashline(row + 2);
    pktgen_display_set_color(NULL);
#endif

    pktgen.flags &= ~PRINT_LABELS_FLAG;
}

/**
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

#if 0
/**
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
    uint32_t mbuf_size, buf_len, priv_size = 0;
    pcaprec_hdr_t hdr;
    ssize_t len = -1;
    char buffer[DEFAULT_MBUF_SIZE];
    pcap_info_t *pcap = (pcap_info_t *)opaque_arg;

    priv_size = rte_pktmbuf_priv_size(mp);
    buf_len   = rte_pktmbuf_data_room_size(mp);
    mbuf_size = sizeof(struct rte_mbuf) + priv_size;
    memset(m, 0, mbuf_size);

    /* start of buffer is just after mbuf structure */
    m->priv_size = priv_size;
    m->buf_addr  = (char *)m + mbuf_size;
    m->buf_iova  = rte_mempool_virt2iova(m) + mbuf_size;
    m->buf_len   = (uint16_t)buf_len;

    /* keep some headroom between start of buffer and data */
    m->data_off = RTE_MIN(RTE_PKTMBUF_HEADROOM, m->buf_len);

    /* init some constant fields */
    m->pool    = mp;
    m->nb_segs = 1;
    m->port    = MBUF_INVALID_PORT;
    rte_mbuf_refcnt_set(m, 1);
    m->next = NULL;

    for (;;) {
        if ((i & 0x3ff) == 0) {
            scrn_printf(1, 1, "%c\b", "-\\|/"[(i >> 10) & 3]);
            i++;
        }

        if (unlikely(pcap_read(pcap, &hdr, buffer, sizeof(buffer)) <= 0)) {
            _pcap_rewind(pcap);
            continue;
        }

        len = hdr.incl_len;

        /* Adjust the packet length if not a valid size. */
        if (len < (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN))
            len = (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN);
        else if ((pktgen.flags & JUMBO_PKTS_FLAG) &&
                 len > (RTE_ETHER_MAX_JUMBO_FRAME_LEN - RTE_ETHER_CRC_LEN))
            len = RTE_ETHER_MAX_JUMBO_FRAME_LEN - RTE_ETHER_CRC_LEN;
        else if (len > (RTE_ETHER_MAX_LEN - RTE_ETHER_CRC_LEN))
            len = (RTE_ETHER_MAX_LEN - RTE_ETHER_CRC_LEN);

        m->data_len = len;
        m->pkt_len  = len;

        rte_memcpy((uint8_t *)m->buf_addr + m->data_off, buffer, len);
        break;
    }
}

/**
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
pktgen_pcap_parse(pcap_info_t *pcap, port_info_t *pinfo, unsigned qid)
{
    pcaprec_hdr_t hdr;
    uint32_t elt_count, max_pkt_size, len, i;
    uint64_t data_size, pkt_sizes = 0;
    char buffer[DEFAULT_MBUF_SIZE];
    char name[RTE_MEMZONE_NAMESIZE];

    if ((pcap == NULL) || (pinfo == NULL))
        return -1;

    _pcap_rewind(pcap); /* Rewind the file is needed */

    snprintf(name, sizeof(name), "%-12s%d:%d", "PCAP-", pinfo->pid, qid);
    scrn_printf(0, 0, "    Process: %-*s ", 18, name);

    pkt_sizes = elt_count = max_pkt_size = i = 0;

    /* The pcap_open left the file pointer to the first packet. */
    while (_pcap_read(pcap, &hdr, buffer, sizeof(buffer)) > 0) {
        /* Skip any jumbo packets or packets that are too small */
        len = hdr.incl_len;

        /* Adjust the packet length if not a valid size. */
        if (len < (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN))
            len = (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN);
        else if ((pktgen.flags & JUMBO_PKTS_FLAG) &&
                 len > (RTE_ETHER_MAX_JUMBO_FRAME_LEN - RTE_ETHER_CRC_LEN))
            len = RTE_ETHER_MAX_JUMBO_FRAME_LEN - RTE_ETHER_CRC_LEN;
        else if (len > (RTE_ETHER_MAX_LEN - RTE_ETHER_CRC_LEN))
            len = (RTE_ETHER_MAX_LEN - RTE_ETHER_CRC_LEN);

        elt_count++;

        if ((elt_count & 0x3ff) == 0)
            scrn_printf(1, 1, "%c\b", "-\\|/"[i++ & 3]);

        pkt_sizes += len;

        if (len > max_pkt_size)
            max_pkt_size = len;
    }

    /* If count is greater then zero then we allocate and create the PCAP mbuf pool. */
    if (elt_count > 0) {
        l2p_port_t *port = l2p_get_port(pinfo->pid);

        /* Create the average size packet */
        pcap->pkt_size  = (pkt_sizes / elt_count);
        pcap->pkt_count = elt_count;
        pcap->pkt_idx   = 0;

        _pcap_rewind(pcap);

        /* Repeat small pcaps so mempool size is close to MAX_MBUFS_PER_PORT. */
        if (elt_count < MAX_MBUFS_PER_PORT(pktgen.nb_rxd, pktgen.nb_txd))
            elt_count = (MAX_MBUFS_PER_PORT(pktgen.nb_rxd, pktgen.nb_txd) / elt_count) * elt_count;

        /* Compute final size of each mbuf by adding the structure header and headroom. */
        max_pkt_size += sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM;

        scrn_printf(0, 0, "\r    Create: %-*s   \b", 16, name);
        port->pcap_mp[qid] = rte_mempool_create(
            name, elt_count, max_pkt_size, 0, sizeof(struct rte_pktmbuf_pool_private),
            rte_pktmbuf_pool_init, NULL, pktgen_pcap_mbuf_ctor, (void *)pcap,
            rte_eth_dev_socket_id(pinfo->pid), MEMPOOL_F_DMA);
        scrn_printf(0, 0, "\r");
        if (port->pcap_mp[qid] == NULL)
            pktgen_log_panic("Cannot init port %d for %d PCAP packets", pinfo->pid,
                             pcap->pkt_count);

        data_size = ((uint64_t)pcap->pkt_count * max_pkt_size);
        scrn_printf(
            0, 0,
            "    Create: %-*s - Number of MBUFs %6u for %5d packets                 = %6u KB\n", 16,
            name, elt_count, pcap->pkt_count, (data_size + 1023) / 1024);
        pktgen.mem_used += data_size;
        pktgen.total_mem_used += data_size;

        pktgen_set_port_flags(pinfo, SEND_PCAP_PKTS);
    }
    return 0;
}
#endif
