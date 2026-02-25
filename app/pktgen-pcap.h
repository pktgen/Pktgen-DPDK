/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_PCAP_H_
#define _PKTGEN_PCAP_H_

/**
 * @file
 *
 * PCAP file read/write support for Pktgen.
 *
 * Provides structures mirroring the libpcap file format, plus functions for
 * opening, replaying, and writing PCAP files from mbufs.
 */

#include <pcap/bpf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCAP_MAGIC_NUMBER  0xa1b2c3d4 /**< PCAP global header magic (little-endian) */
#define PCAP_MAJOR_VERSION 2          /**< PCAP file format major version */
#define PCAP_MINOR_VERSION 4          /**< PCAP file format minor version */

/** PCAP global file header. */
typedef struct pcap_hdr_s {
    uint32_t magic_number;  /**< magic number */
    uint16_t version_major; /**< major version number */
    uint16_t version_minor; /**< minor version number */
    int32_t thiszone;       /**< GMT to local correction */
    uint32_t sigfigs;       /**< accuracy of timestamps */
    uint32_t snaplen;       /**< max length of captured packets, in octets */
    uint32_t network;       /**< data link type */
} pcap_hdr_t;

/** PCAP per-packet record header. */
typedef struct pcap_record_hdr_s {
    uint32_t ts_sec;   /**< timestamp seconds */
    uint32_t ts_usec;  /**< timestamp microseconds */
    uint32_t incl_len; /**< number of octets of packet saved in file */
    uint32_t orig_len; /**< actual length of packet */
} pcap_record_hdr_t;

/** Pktgen PCAP replay state for one port. */
typedef struct pcap_info_s {
    char *filename;                  /**< allocated string for filename of pcap */
    FILE *fp;                        /**< file pointer for pcap file */
    struct rte_mempool *mp;          /**< Mempool for storing packets */
    uint32_t convert;                /**< Endian flag value if 1 convert to host endian format */
    uint32_t max_pkt_size;           /**< largest packet found in pcap file */
    uint32_t avg_pkt_size;           /**< average packet size in pcap file */
    uint32_t pkt_count;              /**< Number of packets in pcap file */
    uint32_t pkt_index;              /**< Index of current packet in pcap file */
    pcap_hdr_t info;                 /**< information on the PCAP file */
    int32_t pcap_result;             /**< PCAP result of filter compile */
    struct bpf_program pcap_program; /**< PCAP filter program structure */

} pcap_info_t;

/**
 * Register a PCAP file for replay on a port.
 *
 * @param filename  Path to the PCAP file.
 * @param port      Port ID to associate with this PCAP.
 * @return
 *   0 on success, negative on error.
 */
int pktgen_pcap_add(char *filename, uint16_t port);

/**
 * Open all registered PCAP files and load packets into mempools.
 *
 * @return
 *   0 on success, negative on error.
 */
int pktgen_pcap_open(void);

/** Close all open PCAP file handles. */
void pktgen_pcap_close(void);

/**
 * Print PCAP file information to the display.
 *
 * @param pcap  PCAP info structure to display.
 * @param port  Port ID.
 * @param flag  Display verbosity flag.
 */
void pktgen_pcap_info(pcap_info_t *pcap, uint16_t port, int flag);

/**
 * Create a new PCAP output file for packet capture.
 *
 * @param filename  Path to the output PCAP file.
 * @return
 *   Open FILE pointer, or NULL on error.
 */
FILE *pktgen_create_pcap_file(char *filename);

/**
 * Close and finalise a PCAP output file.
 *
 * @param fp  File pointer returned by pktgen_create_pcap_file().
 */
void pktgen_close_pcap_file(FILE *fp);

/**
 * Append one mbuf as a packet record to an open PCAP file.
 *
 * @param fp    Open PCAP file handle.
 * @param mbuf  mbuf containing the packet to write.
 * @return
 *   0 on success, negative on write error.
 */
int pktgen_write_mbuf_to_pcap_file(FILE *fp, struct rte_mbuf *mbuf);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_PCAP_H_ */
