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

#ifndef _PKTGEN_RANGE_H_
#define _PKTGEN_RANGE_H_

#include <stdint.h>

#include "pktgen-seq.h"


typedef struct range_info_s {
	uint32_t				src_ip_inc;			/**< Source IP increment */
	uint32_t				dst_ip_inc;			/**< Destination increment IP address */
	uint16_t				src_port_inc;		/**< Source port increment */
	uint16_t				dst_port_inc;		/**< Destination port increment */
	uint16_t				vlan_id_inc;		/**< VLAN id increment */
	uint16_t				pkt_size_inc;		/**< PKT size increment */
    uint64_t				src_mac_inc;		/**< Source MAC increment */
    uint64_t				dst_mac_inc;		/**< Destination MAC increment */

	uint32_t				src_ip;				/**< Source starting IP address */
	uint32_t				src_ip_min;			/**< Source IP minimum */
	uint32_t				src_ip_max;			/**< Source IP maximum */

	uint32_t				dst_ip;				/**< Destination starting IP address */
	uint32_t				dst_ip_min;			/**< Destination minimum IP address */
	uint32_t				dst_ip_max;			/**< Destination maximum IP address */

	uint16_t				src_port;			/**< Source port starting */
	uint16_t				src_port_min;		/**< Source port minimum */
	uint16_t				src_port_max;		/**< Source port maximum */

	uint16_t				dst_port;			/**< Destination port starting */
	uint16_t				dst_port_min;		/**< Destination port minimum */
	uint16_t				dst_port_max;		/**< Destination port maximum */

	uint16_t				vlan_id;			/**< VLAN id starting */
	uint16_t				vlan_id_min;		/**< VLAN id minimum */
	uint16_t				vlan_id_max;		/**< VLAN id maximum */

	uint16_t				pkt_size;			/**< PKT Size starting */
	uint16_t				pkt_size_min;		/**< PKT Size minimum */
	uint16_t				pkt_size_max;		/**< PKT Size maximum */

	uint64_t				dst_mac;			/**< Destination starting MAC address */
	uint64_t				dst_mac_min;		/**< Destination minimum MAC address */
	uint64_t				dst_mac_max;		/**< Destination maximum MAC address */

	uint64_t				src_mac;			/**< Source starting MAC address */
	uint64_t				src_mac_min;		/**< Source minimum MAC address */
	uint64_t				src_mac_max;		/**< Source maximum MAC address */
} range_info_t;

struct port_info_s;

extern void pktgen_range_ctor(range_info_t * range, pkt_seq_t * pkt);
extern void pktgen_range_setup(struct port_info_s * info);

extern void pktgen_page_range(void);

#endif  // _PKTGEN_RANGE_H_
