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
 *
 * Created 2016 by Abhinandan Gujjar S (abhinandan.gujjar@intel.com) */

#ifndef _PKTGEN_GUI_STREAMS_H_
#define _PKTGEN_GUI_STREAMS_H_

#include "pktgen-gui.h"

extern GtkWidget    *stream_window;
extern GtkWidget    *window;
static GArray       *packet_info = NULL;

/* pktgen_port_stream data. Supporting multiple streams per port */
pktgen_port_stream str_db[RTE_MAX_ETHPORTS][NUM_SEQ_PKTS + 1] = {{}};

/* Protocol fields */
const char *pktgen_ethernet_fields[] =
{"Destination MAC",
 "Source MAC",
 "Ether Type",
 "Vlan ID"};

const char *pktgen_ipv4_fields[] =
{"Version",
 "IHL",
 "DSCP/ECN",
 "Total Length",
 "Identification",
 "Flags/Fragment Offset",
 "Time to Live",
 "Protocol",
 "Header Checksum",
 "Source IP Address",
 "Destination IP Address", };

const char *pktgen_udp_fields[] =
{"Source port",
 "Destination port",
 "Length",
 "Checksum"};

const char *pktgen_tcp_fields[] =
{"Source port",
 "Destination port",
 "Sequence number",
 "Acknowledgement number",
 "Data Offset/Res/Flags",
 "Window Size",
 "Checksum",
 "Urgent pointer"};

GtkWidget           *stream_l2_vlan;
GtkWidget           *pktsize_entry;
GtkWidget           *ip_proto_entry;

gpointer             udp_reference;
GtkWidget           *udp_treeview;
GtkWidget           *udp_sw;

gpointer             usr_def_reference;
GtkWidget           *l4_text_view;
GtkTextBuffer       *l4_buffer;

GtkTextBuffer       *l4_pl_buffer;

#endif
