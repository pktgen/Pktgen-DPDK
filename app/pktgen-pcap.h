/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_PCAP_H_
#define _PKTGEN_PCAP_H_

#include <_pcap.h>

#ifdef __cplusplus
extern "C" {
#endif

struct port_info_s;

int pktgen_pcap_parse(pcap_info_t *pcap,
			     struct port_info_s *info,
			     unsigned qid);

void pktgen_page_pcap(uint16_t pid);

#ifdef __cplusplus
}
#endif

#endif  /* _PKTGEN_PCAP_H_ */
