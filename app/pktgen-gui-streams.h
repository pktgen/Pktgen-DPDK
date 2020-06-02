/*-
 * Copyright (c) <2016>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
