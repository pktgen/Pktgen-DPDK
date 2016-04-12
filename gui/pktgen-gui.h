/*-
 * Copyright (c) <2016>, Intel Corporation
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
 * Copyright (c) <2016>, Intel. All rights reserved.
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
/* Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com) */

#ifndef _PKTGEN_GUI_H_
#define _PKTGEN_GUI_H_

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pktgen.h"
#include "pktgen-port-cfg.h"
#include "pktgen-cmds.h"
#include "lauxlib.h"

#define PKTGEN_GUI_MAX_STATS    22
#define PKTGEN_GUI_MAX_STATIC   13

//GtkWidget           *window = NULL;
GtkTreeStore        *treestore_stats[RTE_MAX_ETHPORTS];
GtkTreeStore        *treestore_static[RTE_MAX_ETHPORTS];
GtkWidget           *view_static[RTE_MAX_ETHPORTS];
GtkTreeModel        *model_static[RTE_MAX_ETHPORTS];

//GtkWidget           *view_chassis;
GtkWidget           *view_stats[RTE_MAX_ETHPORTS];
GtkTreeModel        *model_stats[RTE_MAX_ETHPORTS];

GtkWidget           *traffic_start_button;
GtkWidget           *traffic_stop_button;
GtkWidget           *capture_start_button;
GtkWidget           *capture_stop_button;
GtkWidget           *about_button;

GtkTextBuffer       *buffer;
GtkTextIter buffer_iter;

typedef struct highlight_stats_s {
	uint64_t __64;		/**< Number of 64 byte packets */
	uint64_t __65_127;	/**< Number of 65-127 byte packets */
	uint64_t __128_255;	/**< Number of 128-255 byte packets */
	uint64_t __256_511;	/**< Number of 256-511 byte packets */
	uint64_t __512_1023;	/**< Number of 512-1023 byte packets */
	uint64_t __1024_1518;	/**< Number of 1024-1518 byte packets */
	uint64_t __broadcast;	/**< Number of broadcast packets */
	uint64_t __multicast;	/**< Number of multicast packets */
	uint64_t __ierrors;	/**< Number of Rx packets */
	uint64_t __oerrors;	/**< Number of Tx packets */
} highlight_stats_t;

static highlight_stats_t prev_stats_val[RTE_MAX_ETHPORTS];

static gboolean gui_created = FALSE;
extern pktgen_t pktgen;
extern cmdline_parse_ctx_t main_ctx[];

enum {
	COL_CHASSIS_PORTS = 0,
	NUM_COLS
};

void pktgen_gui_main(int argc, char *argv[]);
GtkWidget *pktgen_streams(void);

const char *pktgen_static_fields[PKTGEN_GUI_MAX_STATIC] = {
	"Pattern Type",
	"Tx Count",
	"Rate(%)",
	"Packet Size (bytes)",
	"Tx Burst (Pkts/s) ",
	"Src Port",
	"Dest Port",
	"Pkt Type",
	"VLAN ID",
	"Dst  IP Address",
	"Src  IP Address",
	"Dst MAC Address",
	"Src MAC Address"
};

const char *pktgen_stats_fields[PKTGEN_GUI_MAX_STATS] =
{"Rx (Pkts/s)",
 "Tx (Pkts/s)",
 "Rx (MBits/s)",
 "Tx (MBits/s)",
 "Broadcast",
 "Multicast",
 "     64 Bytes",
 "     65-127",
 "     128-255",
 "     256-511",
 "     512-1023",
 "     1024-1518",
 "Runts",
 "Jumbos",
 "Errors Rx",
 "Errors Tx",
 "Total Rx (Pkts)",
 "Total Tx (Pkts)",
 "Total Rx (MBs)",
 "Total Tx (MBs)",
 "ARP Pkts",
 "ICMP Pkts"};

const char intel_copyright[] = {
	"\n"
	"   BSD LICENSE\n"
	"\n"
	"   Copyright(c) 2010-2016 Intel Corporation. All rights reserved.\n"
	"   All rights reserved.\n"
	"\n"
	"   Redistribution and use in source and binary forms, with or without\n"
	"   modification, are permitted provided that the following conditions\n"
	"   are met:\n"
	"\n"
	"     * Redistributions of source code must retain the above copyright\n"
	"       notice, this list of conditions and the following disclaimer.\n"
	"     * Redistributions in binary form must reproduce the above copyright\n"
	"       notice, this list of conditions and the following disclaimer in\n"
	"       the documentation and/or other materials provided with the\n"
	"       distribution.\n"
	"     * Neither the name of Intel Corporation nor the names of its\n"
	"       contributors may be used to endorse or promote products derived\n"
	"       from this software without specific prior written permission.\n"
	"\n"
	"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
	"   \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
	"   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
	"   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
	"   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
	"   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
	"   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
	"   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
	"   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
	"   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
	"   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
	""
};

const gchar *authors[] = {"Created by:\n"
	                  "Keith Wiles (keith.wiles@intel.com)\n\n"
	                  "GUI Designed by:\n"
	                  "Abhinandan Gujjar S(abhinandan.gujjar@intel.com)"};

#define COPYRIGHT_MSG \
        "Copyright (c) <2010-2016>, Wind River Systems, Inc. All rights reserved."
#define POWERED_BY_DPDK         "GUI Version 0.1.0\n Powered by Intel® DPDK"

#define PKTGEN_GUI_APP_NAME "PktGen GUI (Version 0.1.0)"

#endif
