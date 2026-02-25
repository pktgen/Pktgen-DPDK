/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
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

static pcap_info_t *pcap_info_list[RTE_MAX_ETHPORTS];

void
pktgen_pcap_info(pcap_info_t *pcap, uint16_t port, int flag)
{
    printf("PCAP file for port %d: %s\n", port, pcap->filename);
    printf("  magic: %08x,", pcap->info.magic_number);
    printf(" Version: %d.%d,", pcap->info.version_major, pcap->info.version_minor);
    printf(" Zone: %d,", pcap->info.thiszone);
    printf(" snaplen: %d,", pcap->info.snaplen);
    printf(" sigfigs: %d,", pcap->info.sigfigs);
    printf(" network: %d", pcap->info.network);
    printf(" Convert Endian: %s\n", pcap->convert ? "Yes" : "No");
    if (flag)
        printf("  Packet count: %d, max size %d\n", pcap->pkt_count, pcap->max_pkt_size);
    fflush(stdout);
}

static __inline__ void
pcap_convert(pcap_info_t *pcap, pcap_record_hdr_t *pHdr)
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
    pcap_record_hdr_t hdr;

    if (fread(&pcap->info, 1, sizeof(pcap_hdr_t), pcap->fp) != sizeof(pcap_hdr_t))
        rte_exit(EXIT_FAILURE, "%s: failed to read pcap header\n", __func__);

    /* Make sure we have a valid PCAP file for Big or Little Endian formats. */
    if (pcap->info.magic_number == PCAP_MAGIC_NUMBER)
        pcap->convert = 0;
    else if (pcap->info.magic_number == ntohl(PCAP_MAGIC_NUMBER))
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

    pcap->max_pkt_size  = 0;
    pcap->avg_pkt_size  = 0;
    uint64_t total_size = 0;
    /* count the number of packets and get the largest size packet */
    for (;;) {
        if (fread(&hdr, 1, sizeof(pcap_record_hdr_t), pcap->fp) != sizeof(hdr))
            break;

        /* Convert the packet header to the correct format if needed */
        pcap_convert(pcap, &hdr);

        if (fseek(pcap->fp, hdr.incl_len, SEEK_CUR) < 0)
            break;

        pcap->pkt_count++;
        if (hdr.incl_len > pcap->max_pkt_size)
            pcap->max_pkt_size = hdr.incl_len;

        total_size += hdr.incl_len;
    }
    printf("PCAP: Max Packet Size: %d\n", pcap->max_pkt_size);

    pcap->avg_pkt_size = total_size / pcap->pkt_count;

    printf("PCAP: Avg Packet Size: %d\n", pcap->avg_pkt_size);

    pcap_rewind(pcap);
}

static __inline__ void
mbuf_iterate_cb(struct rte_mempool *mp, void *opaque, void *obj, unsigned obj_idx __rte_unused)
{
    pcap_info_t *pcap     = (pcap_info_t *)opaque;
    struct rte_mbuf *m    = (struct rte_mbuf *)obj;
    pcap_record_hdr_t hdr = {0};

    if (fread(&hdr, 1, sizeof(pcap_record_hdr_t), pcap->fp) != sizeof(hdr)) {
        pcap_rewind(pcap);
        if (fread(&hdr, 1, sizeof(pcap_record_hdr_t), pcap->fp) != sizeof(hdr))
            rte_exit(EXIT_FAILURE, "%s: failed to read pcap header\n", __func__);
    }

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

int
pktgen_pcap_add(char *filename, uint16_t pid)
{
    pcap_info_t *pcap = NULL;
    char name[64]     = {0};
    uint16_t sid;

    if (filename == NULL)
        rte_exit(EXIT_FAILURE, "%s: PCAP filename is NULL\n", __func__);

    sid = pg_eth_dev_socket_id(pid);

    snprintf(name, sizeof(name), "PCAP-Info-%d", pid);
    pcap = (pcap_info_t *)rte_zmalloc_socket(name, sizeof(pcap_info_t), RTE_CACHE_LINE_SIZE, sid);
    if (pcap == NULL)
        rte_exit(EXIT_FAILURE, "%s: rte_zmalloc_socket() failed for pcap_info_t structure\n",
                 __func__);

    /* Default to little endian format. */
    pcap->filename = strdup(filename);

    pcap_info_list[pid] = pcap;

    return 0;
}

int
pktgen_pcap_open(void)
{
    pcap_info_t *pcap = NULL;
    struct rte_mempool *mp;
    char name[64] = {0};
    uint16_t sid;
    uint32_t pkt_count;

    for (int pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
        if ((pcap = pcap_info_list[pid]) == NULL)
            continue;

        pcap = pcap_info_list[pid];

        sid = pg_eth_dev_socket_id(pid);

        /* Read the pcap file trailer. */
        pcap->fp = fopen((const char *)pcap->filename, "r");
        if (pcap->fp == NULL)
            rte_exit(EXIT_FAILURE, "%s: failed for (%s)\n", __func__, pcap->filename);

        pcap_get_info(pcap);

        pkt_count = pcap->pkt_count;
        if (pkt_count == 0) {
            fclose(pcap->fp);
            rte_exit(EXIT_FAILURE, "%s: PCAP file is empty: %s\n", __func__, pcap->filename);
        }
        if (pkt_count < (DEFAULT_TX_DESC * 4))
            pkt_count = (DEFAULT_TX_DESC * 4);

        snprintf(name, sizeof(name), "pcap-%d", pid);
        uint32_t dataroom =
            RTE_ALIGN_CEIL(pcap->max_pkt_size + RTE_PKTMBUF_HEADROOM, RTE_CACHE_LINE_SIZE);
        mp = rte_pktmbuf_pool_create(name, pkt_count, 0, DEFAULT_PRIV_SIZE, dataroom, sid);
        if (mp == NULL)
            rte_exit(EXIT_FAILURE,
                     "Cannot create mbuf pool (%s) port %d, nb_mbufs %d, socket_id %d: %s", name,
                     pid, pcap->pkt_count, sid, rte_strerror(rte_errno));

        pcap->mp = mp;

        rte_mempool_obj_iter(mp, mbuf_iterate_cb, pcap);

        if (l2p_set_pcap_info(pid, pcap) < 0)
            pktgen_log_error("Error opening PCAP file: %s", pcap->filename);
    }
    return 0;
}

void
pktgen_pcap_close(void)
{
    pcap_info_t *pcap = NULL;

    for (int pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
        pcap = pcap_info_list[pid];
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
}

FILE *
pktgen_create_pcap_file(char *filename)
{
    struct pcap_file_header file_header;
    file_header.magic         = 0xa1b2c3d4;
    file_header.version_major = 2;
    file_header.version_minor = 4;
    file_header.thiszone      = 0;
    file_header.sigfigs       = 0;
    file_header.snaplen       = 65535;
    file_header.linktype      = 1;        // LINKTYPE_ETHERNET

    printf("Creating PCAP file: %s, %lu, %lu\n", filename, sizeof(struct pcap_file_header),
           sizeof(struct pcap_pkthdr));

    // Open the output file
    FILE *file = fopen(filename, "wb");
    if (!file)
        return NULL;

    // Write the file header
    fwrite(&file_header, sizeof(uint8_t), sizeof(file_header), file);
    fflush(file);

    return file;
}

void
pktgen_close_pcap_file(FILE *fp)
{
    if (fp)
        fclose(fp);
}

int
pktgen_write_mbuf_to_pcap_file(FILE *fp, struct rte_mbuf *mbuf)
{
    // Packet header
    pcap_record_hdr_t packet_header;
    size_t size;

    if (fp == NULL)
        return 0;

    packet_header.ts_sec   = 0;
    packet_header.ts_usec  = 0;
    packet_header.incl_len = rte_pktmbuf_pkt_len(mbuf);
    packet_header.orig_len = rte_pktmbuf_pkt_len(mbuf);

    // Write the packet header
    if ((size = fwrite(&packet_header, sizeof(uint8_t), sizeof(packet_header), fp)) != 16)
        printf("Error writing packet header %ld\n", size);

    // Write the packet data
    fwrite(rte_pktmbuf_mtod(mbuf, char *), sizeof(uint8_t), rte_pktmbuf_pkt_len(mbuf), fp);
    fflush(fp);

    return 0;
}
