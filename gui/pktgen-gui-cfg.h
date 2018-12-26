/*-
 * Copyright (c) <2016-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com) */

#ifndef _PKTGEN_GUI_CFG_H_
#define _PKTGEN_GUI_CFG_H_

GtkWidget           *window = NULL;

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
	"   Copyright(c) <2010-2019> Intel Corporation. All rights reserved.\n"
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

#define GUI_VERSION         " Version 1.0.0 "
#define COPYRIGHT_MSG \
        "Copyright (c) <2010-2019>, Intel Corporation. All rights reserved."
#define POWERED_BY_DPDK     "GUI" GUI_VERSION "\n Powered by DPDK"
#define PKTGEN_GUI_APP_NAME ("Pktgen GUI" GUI_VERSION)

#endif
