/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_PCAP_H_
#define _PKTGEN_PCAP_H_

#include <pcap/bpf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCAP_MAGIC_NUMBER  0xa1b2c3d4
#define PCAP_MAJOR_VERSION 2
#define PCAP_MINOR_VERSION 4

typedef struct pcap_hdr_s {
    uint32_t magic_number;  /**< magic number */
    uint16_t version_major; /**< major version number */
    uint16_t version_minor; /**< minor version number */
    int32_t thiszone;       /**< GMT to local correction */
    uint32_t sigfigs;       /**< accuracy of timestamps */
    uint32_t snaplen;       /**< max length of captured packets, in octets */
    uint32_t network;       /**< data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;   /**< timestamp seconds */
    uint32_t ts_usec;  /**< timestamp microseconds */
    uint32_t incl_len; /**< number of octets of packet saved in file */
    uint32_t orig_len; /**< actual length of packet */
} pcaprec_hdr_t;

typedef struct pcap_info_s {
    char *filename;                  /**< allocated string for filename of pcap */
    FILE *fp;                        /**< file pointer for pcap file */
    struct rte_mempool *mp;          /**< Mempool for storing packets */
    uint32_t convert;                /**< Endian flag value if 1 convert to host endian format */
    uint32_t max_pkt_size;           /**< largest packet found in pcap file */
    uint32_t pkt_count;              /**< Number of packets in pcap file */
    uint32_t pkt_index;              /**< Index of current packet in pcap file */
    pcap_hdr_t info;                 /**< information on the PCAP file */
    int32_t pcap_result;             /**< PCAP result of filter compile */
    struct bpf_program pcap_program; /**< PCAP filter program structure */

} pcap_info_t;

pcap_info_t *pktgen_pcap_open(char *filename, uint16_t port);
void pktgen_pcap_close(pcap_info_t *pcap);
void pktgen_page_pcap(uint16_t pid);
void pktgen_pcap_info(pcap_info_t *pcap, uint16_t port, int flag);

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_PCAP_H_ */
