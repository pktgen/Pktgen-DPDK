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

#include "pktgen-gui.h"

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

extern GtkWidget    *stream_window;
static GArray       *packet_info = NULL;

void
fill_proto_field_info(proto_type type, unsigned int pid)
{
	uint i = 0;
	protocol val;

	port_info_t *info = NULL;
	pkt_seq_t *pkt = NULL;

	info = &pktgen.info[pid];
	pkt  = &info->seq_pkt[SINGLE_PKT];
	char buff[50];

	g_return_if_fail(packet_info != NULL);

	if (type == TYPE_ETH) {
		struct ether_addr *eaddr = &pkt->eth_dst_addr;
		val.name = g_strdup(pktgen_ethernet_fields[i++]);
		snprintf(buff, sizeof(buff), "%02x%02x%02x%02x%02x%02x",
		         eaddr->addr_bytes[0], eaddr->addr_bytes[1],
		         eaddr->addr_bytes[2], eaddr->addr_bytes[3],
		         eaddr->addr_bytes[4], eaddr->addr_bytes[5]);

		val.value = g_strdup(buff);
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ethernet_fields[i++]);
		eaddr = &pkt->eth_src_addr;
		snprintf(buff, sizeof(buff), "%02x%02x%02x%02x%02x%02x",
		         eaddr->addr_bytes[0], eaddr->addr_bytes[1],
		         eaddr->addr_bytes[2], eaddr->addr_bytes[3],
		         eaddr->addr_bytes[4], eaddr->addr_bytes[5]);

		val.value = g_strdup(buff);
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ethernet_fields[i++]);
		val.value = g_strdup("IPv4");
		g_array_append_vals(packet_info, &val, 1);

		sprintf(buff, "%d", pkt->vlanid);
		val.name = g_strdup(pktgen_ethernet_fields[i++]);
		val.value = g_strdup(buff);
		g_array_append_vals(packet_info, &val, 1);
	} else if (type == TYPE_IPv4) {
		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("IPv4");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		pkt->ipProto = PG_IPPROTO_UDP;
		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup(
		                (pkt->ipProto ==
		                 PG_IPPROTO_UDP) ? "UDP" : "User Defined");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value =
		        g_strdup(inet_ntop4(buff, sizeof(buff),
		                            ntohl(pkt->ip_src_addr.addr.ipv4.
		                                  s_addr),
		                            0xFFFFFFFF));
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_ipv4_fields[i++]);
		val.value =
		        g_strdup(inet_ntop4(buff, sizeof(buff),
		                            ntohl(pkt->ip_dst_addr.addr.ipv4.
		                                  s_addr),
		                            0xFFFFFFFF));
		g_array_append_vals(packet_info, &val, 1);
	} else if (type == TYPE_UDP) {
		sprintf(buff, "%d", pkt->sport);
		val.name = g_strdup(pktgen_udp_fields[i++]);
		val.value = g_strdup(buff);
		g_array_append_vals(packet_info, &val, 1);

		sprintf(buff, "%d", pkt->dport);
		val.name = g_strdup(pktgen_udp_fields[i++]);
		val.value = g_strdup(buff);
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_udp_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);

		val.name = g_strdup(pktgen_udp_fields[i++]);
		val.value = g_strdup("<auto>");
		g_array_append_vals(packet_info, &val, 1);
	}
}

GtkTreeModel *
create_stream_model(proto_type type, unsigned int pid)
{
	uint i = 0;
	GtkListStore *model;
	GtkTreeIter iter;

	fill_proto_field_info(type, pid);

	/* create list store */
	model = gtk_list_store_new(MAX_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	if (type == TYPE_ETH) i = 0;
	else if (type == TYPE_IPv4) i = 4;
	else if (type == TYPE_UDP) i = 15;

	/* add items */
	for (; i < packet_info->len; i++) {
		gtk_list_store_append(model, &iter);

		gtk_list_store_set(model, &iter,
		                   COLUMN_NAME,
		                   g_array_index(packet_info, protocol, i).name,
		                   COLUMN_VALUE,
		                   g_array_index(packet_info,
		                                 protocol,
		                                 i).value,

		                   -1);
	}

	return GTK_TREE_MODEL(model);
}

int
validate_ip_address(char *st)
{
	int num, i, len;
	char *ch;

	/* counting number of quads present in a given IP address */
	int quadsCnt = 0;

	len = strlen(st);

	/*  Check if the string is valid */
	if (len < 7 || len > 15)
		return 0;

	ch = strtok(st, ".");

	while (ch != NULL) {
		quadsCnt++;

		num = 0;
		i = 0;

		/*  Get the current token and convert to an integer value */
		while (ch[i] != '\0') {
			num = num * 10;
			num = num + (ch[i] - '0');
			i++;
		}

		if (num < 0 || num > 255)
			/* Not a valid ip */
			return 0;

		if ( (quadsCnt == 1 && num == 0) || (quadsCnt == 4 && num == 0))
			/* Not a valid ip */
			return 0;

		ch = strtok(NULL, ".");
	}

	/*  Check the address string, should be n.n.n.n format */
	if (quadsCnt != 4)
		return 0;

	/*  Looks like a valid IP address */
	return 1;
}

void
cell_edited_callback(GtkCellRendererText *cell,
                     const gchar         *path_string,
                     gchar               *new_text,
                     gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *)data;
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	GtkTreeIter iter_name;

	gchar *col_name;
	char new_ip[20];

	strcpy(new_ip, new_text);

	gint column =
	        GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));

	gtk_tree_model_get_iter(model, &iter, path);

	gtk_tree_model_get_iter(model, &iter_name, path);
	gtk_tree_model_get(model, &iter_name, 0, &col_name, -1);

	switch (column) {
	case COLUMN_VALUE:
	{
		uint i = 0, index_offset = 0;
		gchar *old_text;

		gtk_tree_model_get(model, &iter, column, &old_text, -1);

		if (0 == strcmp(col_name, pktgen_ethernet_fields[3]))
			if ((atoi(new_text) > 4096) || (atoi(new_text) < 1)) {
				GtkWidget *dialog;
				dialog =
				        gtk_message_dialog_new(GTK_WINDOW(
				                                       stream_window),
				                               GTK_DIALOG_DESTROY_WITH_PARENT,
				                               GTK_MESSAGE_INFO,
				                               GTK_BUTTONS_OK,
				                               "Invalid VLAN Id! It should be [1-4095]");
				gtk_window_set_title(GTK_WINDOW(
				                             dialog), "Pktgen");
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				return;
			}

		if ((0 ==
		     strcmp(col_name,
		            pktgen_ipv4_fields[9])) ||
		    (0 == strcmp(col_name, pktgen_ipv4_fields[10])) )
			if (!validate_ip_address(new_ip)) {
				GtkWidget *dialog;
				dialog =
				        gtk_message_dialog_new(GTK_WINDOW(
				                                       stream_window),
				                               GTK_DIALOG_DESTROY_WITH_PARENT,
				                               GTK_MESSAGE_INFO,
				                               GTK_BUTTONS_OK,
				                               "The given IP is not a valid IP address!");
				gtk_window_set_title(GTK_WINDOW(
				                             dialog), "Pktgen");
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				return;
			}

		/* Validate the MAC address */
		if ((0 ==
		     strcmp(col_name,
		            pktgen_ethernet_fields[0])) ||
		    (0 == strcmp(col_name, pktgen_ethernet_fields[1])) ) {
			if (12 == strlen(new_text)) {
				gchar *ch;
				for (i = 0; i < strlen(new_text); i++) {
					ch = (new_text + i);

					if ((*ch >= 'a' &&
					     *ch <= 'f') ||
					    (*ch >= 'A' &&
					     *ch <= 'F') ||
					    (*ch >= '0' && *ch <= '9'))
						continue;
					else {
						GtkWidget *dialog;
						dialog = gtk_message_dialog_new(
						                GTK_WINDOW(
						                        stream_window),
						                GTK_DIALOG_DESTROY_WITH_PARENT,
						                GTK_MESSAGE_INFO,
						                GTK_BUTTONS_OK,
						                "Please input only hex values[0-9][a-f], special characters are not allowed!");
						gtk_window_set_title(GTK_WINDOW(
						                             dialog),
						                     "Pktgen");
						gtk_dialog_run(GTK_DIALOG(
						                       dialog));
						gtk_widget_destroy(dialog);

						return;
					}
				}
			} else {
				GtkWidget *dialog;
				dialog =
				        gtk_message_dialog_new(GTK_WINDOW(
				                                       stream_window),
				                               GTK_DIALOG_DESTROY_WITH_PARENT,
				                               GTK_MESSAGE_INFO,
				                               GTK_BUTTONS_OK,
				                               "Please input 6 bytes of MAC address in hex pattern [e.g. 012345abcdef]");
				gtk_window_set_title(GTK_WINDOW(
				                             dialog), "Pktgen");
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);

				return;
			}
		}
		if ((0 !=
		     strcmp(old_text,
		            "<auto>")) &&
		    (0 !=
		     strcmp(old_text,
		            "IPv4")) && (0 != strcmp(old_text, "UDP")) ) {
			g_free(old_text);

			i = gtk_tree_path_get_indices(path)[0];

			switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(
			                                              notebook)))
			{
			case 0:
				index_offset = 0;
				break;

			case 1:
				index_offset = 4;
				break;

			case 2:
				index_offset = 15;
				break;
			}

			g_free(g_array_index(packet_info, protocol,
			                     (i + index_offset)).value);
			g_array_index(packet_info, protocol,
			              (i + index_offset)).value = g_strdup(
			                new_text);

			gtk_list_store_set(GTK_LIST_STORE(model), &iter, column,
			                   g_array_index(packet_info,
			                                 protocol,
			                                 (i +
			                                  index_offset)).value,
			                   -1);
		} else {
			GtkWidget *dialog;
			dialog =
			        gtk_message_dialog_new(GTK_WINDOW(
			                                       stream_window),
			                               GTK_DIALOG_DESTROY_WITH_PARENT,
			                               GTK_MESSAGE_INFO,
			                               GTK_BUTTONS_OK,
			                               "Auto generated values can not be modified!");
			gtk_window_set_title(GTK_WINDOW(dialog), "Pktgen");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
	}
	break;
	}

	gtk_tree_path_free(path);
}

void
add_proto_values_column(GtkTreeView  *treeview,
                        GtkTreeModel *stream_model)
{
	GtkCellRenderer *renderer;

	/* number column */
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer,
	             "editable", FALSE,
	             NULL);
	g_object_set_data(G_OBJECT(renderer), "column",
	                  GINT_TO_POINTER(COLUMN_NAME));

	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
	                                            -1, "Name", renderer,
	                                            "text", COLUMN_NAME,
	                                            NULL);

	/* value column */
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer,
	             "editable", TRUE,
	             NULL);
	g_signal_connect(renderer, "edited",
	                 G_CALLBACK(cell_edited_callback), stream_model);
	g_object_set_data(G_OBJECT(renderer), "column",
	                  GINT_TO_POINTER(COLUMN_VALUE));

	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
	                                            -1, "Value", renderer,
	                                            "text", COLUMN_VALUE,
	                                            NULL);
}

GtkWidget *
pktgen_fill_stream_info(proto_type type, unsigned int pid)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *sw;
	GtkWidget *treeview;
	GtkWidget *next_button, *apply_button, *quit_button;
	GtkTreeModel *stream_model;

	GtkWidget *frame;

	frame = gtk_frame_new(NULL);
	gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
	gtk_widget_set_size_request(frame, 300, 200);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	gtk_box_pack_start(GTK_BOX(vbox),
	                   gtk_label_new("Edit Protocol fields"),
	                   FALSE, FALSE, 0);

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
	                                    GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
	                               GTK_POLICY_AUTOMATIC,
	                               GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

	/* create models */
	stream_model = create_stream_model(type, pid);

	/* create tree view */
	treeview = gtk_tree_view_new_with_model(stream_model);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(
	                                                                treeview)),
	                            GTK_SELECTION_SINGLE);

	add_proto_values_column(GTK_TREE_VIEW(treeview), stream_model);

	g_object_unref(stream_model);

	gtk_container_add(GTK_CONTAINER(sw), treeview);

	/* some buttons */
	hbox = gtk_hbox_new(TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	next_button = gtk_button_new_with_label("Next");
	g_signal_connect(next_button, "clicked",
	                 G_CALLBACK(switch_stream_editor_page), notebook);

	gtk_box_pack_start(GTK_BOX(hbox), next_button, TRUE, TRUE, 0);

	apply_button = gtk_button_new_with_label("Apply");
	g_signal_connect(apply_button, "clicked",
	                 G_CALLBACK(pktgen_port_stream_apply_callback), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), apply_button, TRUE, TRUE, 0);

	quit_button = gtk_button_new_with_label("Close");

	g_signal_connect(quit_button,
	                 "clicked",
	                 G_CALLBACK(close_window_callback),
	                 G_OBJECT(stream_window));

	gtk_box_pack_start(GTK_BOX(hbox), quit_button, TRUE, TRUE, 0);

	/* put everything into a scrolled window */
	return GTK_WIDGET(frame);
}

void
close_window_callback(GtkWidget __attribute__(
                              (unused)) *widget, gpointer window)
{
	gtk_widget_destroy(GTK_WIDGET(window));
}

/*
 * vlan_enable_callback
 *
 * Handle a checkbox signal
 */
void
vlan_enable_callback(GtkWidget *widget, gpointer *data)
{
	gboolean active;
	unsigned int *pid = (unsigned int *)data;
	port_info_t *info = NULL;

	info = &pktgen.info[*pid];

	g_object_get(G_OBJECT(widget), "active", &active, NULL);
	pktgen_set_vlan(info, (uint32_t)active);
}

void
pktgen_display_stream_editor(GtkWidget *window, unsigned int pid)
{
	GtkWidget *stream_l2_vbox, *stream_l3_vbox, *stream_l4_vbox;
	GtkWidget *stream_l2_vlan;

	GtkWidget *label2, *label3, *label4;
	char window_name[50];

	sprintf(window_name, "Edit streams for port %d", pid);
	gtk_window_set_title(GTK_WINDOW(window), window_name);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_widget_set_size_request(window, 400, 200);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(window), "destroy",
	                 G_CALLBACK(gtk_main_quit), NULL);

	notebook = gtk_notebook_new();
	label2 = gtk_label_new("Ethernet");
	label3 = gtk_label_new("IPv4");
	label4 = gtk_label_new("UDP");

	/* create array for store protocol info */
	packet_info =
	        g_array_sized_new(FALSE, FALSE, sizeof(protocol),
	                          (1 + 3 + 11 + 4));

	stream_l2_vbox = gtk_vbox_new(FALSE, 0);
	stream_l2_vlan = gtk_check_button_new_with_label("VLAN");

	gtk_signal_connect(GTK_OBJECT(stream_l2_vlan), "toggled",
	                   GTK_SIGNAL_FUNC(
	                           vlan_enable_callback), (gpointer) & pid);

	gtk_box_pack_start(GTK_BOX(
	                           stream_l2_vbox), stream_l2_vlan, FALSE,
	                   FALSE, 0);
	gtk_box_pack_start(GTK_BOX(stream_l2_vbox),
	                   pktgen_fill_stream_info(TYPE_ETH,
	                                           pid), TRUE, TRUE, 0);

	stream_l3_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(stream_l3_vbox),
	                   pktgen_fill_stream_info(TYPE_IPv4,
	                                           pid), TRUE, TRUE, 0);

	stream_l4_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(stream_l4_vbox),
	                   pktgen_fill_stream_info(TYPE_UDP,
	                                           pid), TRUE, TRUE, 0);

	/* Append to pages to the notebook container. */
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stream_l2_vbox, label2);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stream_l3_vbox, label3);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stream_l4_vbox, label4);

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);

	gtk_container_add(GTK_CONTAINER(window), notebook);
	gtk_widget_show_all(window);

	gtk_main();
}

/* Switch between the current GtkNotebook page. */
void
switch_stream_editor_page(GtkButton __attribute__((unused)) *button,
                          GtkNotebook *notebook)
{
	gint page = gtk_notebook_get_current_page(notebook);

	if (page != 2)
		gtk_notebook_set_current_page(notebook, ++page);
	else
		gtk_notebook_set_current_page(notebook, 0);
}

void
pktgen_port_stream_apply_callback(void)
{
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    pktgen_conf_traffic_stream,
	                                    NULL);
}

void
pktgen_conf_traffic_stream(GtkTreeModel  *model,
                           GtkTreePath __attribute__((unused)) *path,
                           GtkTreeIter   *iter,
                           gpointer __attribute__((unused)) userdata)
{
	gchar *name;
	unsigned int pid;

	gtk_tree_model_get(model, iter, COL_CHASSIS_PORTS, &name, -1);

	if (0 != g_strcmp0(name, "[127.0.0.1]")) {
		int offset = strlen(name);
		pid = atoi((name + offset) - 1);
		pktgen_set_stream_info(pid);
	}
}

int
hex_to_number(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int
ascii_to_mac(const char *txt, unsigned int *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;
		a = hex_to_number(*txt++);
		if (a < 0)
			return -1;
		b = hex_to_number(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
	}
	return 0;
}

void
pktgen_set_stream_info(unsigned int pid)
{
	port_info_t *info = NULL;
	pkt_seq_t *pkt = NULL;

	info = &pktgen.info[pid];
	pkt  = &info->seq_pkt[SINGLE_PKT];
	uint i = 0;

	/* set values */
	for (i = 0; i < packet_info->len; i++) {
		int j = 0;
		char str[13] = {0};
		unsigned int mac[6] = {0};
		struct sockaddr_in new_src_ip, new_dst_ip;

		switch (i) {
		case 0:	/* Destination MAC */
			strcpy(str, g_array_index(packet_info,
			                          protocol,
			                          i).value);
			ascii_to_mac(str, mac);

			for (j = 0; j < 6; j++)
				pkt->eth_dst_addr.addr_bytes[j] = mac[j];
			break;

		case 1:	/* Source MAC */
			strcpy(str, g_array_index(packet_info,
			                          protocol,
			                          i).value);
			ascii_to_mac(str, mac);

			for (j = 0; j < 6; j++)
				pkt->eth_src_addr.addr_bytes[j] = mac[j];
			break;

		case 3:	/* Vlan ID */
			pkt->vlanid =
			        atoi(g_array_index(packet_info,
			                           protocol,
			                           i).value);
			break;

		case 13:/* Source IP Address */
			inet_aton(g_array_index(packet_info,
			                        protocol,
			                        i).value, &new_src_ip.sin_addr);
			pkt->ip_src_addr.addr.ipv4.s_addr = ntohl(
			                new_src_ip.sin_addr.s_addr);
			break;

		case 14:/* Destination IP Address */
			inet_aton(g_array_index(packet_info,
			                        protocol,
			                        i).value, &new_dst_ip.sin_addr);
			pkt->ip_dst_addr.addr.ipv4.s_addr = ntohl(
			                new_dst_ip.sin_addr.s_addr);
			break;

		case 15:/* Source Port */
			pkt->sport =
			        atoi(g_array_index(packet_info,
			                           protocol,
			                           i).value);
			break;

		case 16:/* Destination Port */
			pkt->dport =
			        atoi(g_array_index(packet_info,
			                           protocol,
			                           i).value);
			break;
		}
	}

	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}
