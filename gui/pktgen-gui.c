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
#include "pktgen-gui-cfg.h"

#define static_store(p, b) do {                                     \
        gtk_tree_store_set(treestore_static[(p)],                   \
                           &toplevel, COL_CHASSIS_PORTS, (b), -1);  \
    } while((0))

#define chassis_store(t, b) do {                                    \
        gtk_tree_store_set(treestore_chassis,                       \
                           &(t), COL_CHASSIS_PORTS, (b), -1);       \
    } while((0))

#define stats_store(p, v) do {                                      \
        gtk_tree_store_set(treestore_stats[(p)],                    \
                           &toplevel, COL_CHASSIS_PORTS, (v), -1);  \
    } while((0))

#define stats_store_next(p, v) do {                                 \
        gboolean rc;                                                \
        stats_store(p, v);                                          \
        rc = gtk_tree_model_iter_next(model_stats[(p)], &toplevel); \
        if ((rc == FALSE) && (errno != 11)) {                       \
            printf("%s: Pid %d - %s Error %s\n",                    \
                __func__, (p), #v, strerror(errno));                \
            continue;                                               \
        }                                                           \
    } while((0))

GtkTreeModel *
fill_chassis_info(void)
{
	GtkTreeStore  *treestore_chassis;
	GtkTreeIter toplevel, child;
	uint32_t pid;
	rxtx_t cnt;

	char str_port[15];

	treestore_chassis = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING);

	/* Append a second top level row, and fill it with some data */
	gtk_tree_store_append(treestore_chassis, &toplevel, NULL);
    chassis_store(toplevel, "[127.0.0.1]");

	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		cnt.rxtx = wr_get_map(pktgen.l2p, pid, RTE_MAX_LCORE);
		if (cnt.rxtx == 0)
			continue;

		sprintf(str_port, "%s%d", "Port-", pid);

		/* Append a child to the second top level row, and fill in some data */
		gtk_tree_store_append(treestore_chassis, &child, &toplevel);
        chassis_store(child, str_port);
	}

	return GTK_TREE_MODEL(treestore_chassis);
}

GtkTreeModel *
fill_port_info(unsigned int pid, gboolean is_static)
{
	GtkTreeIter toplevel;
	gint i;

	if (is_static == TRUE) {
		treestore_static[pid] = gtk_tree_store_new(NUM_COLS,
		                                           G_TYPE_STRING);
		for (i = 0; i < (PKTGEN_GUI_MAX_STATIC); i++) {
			/* Append a top level row, and fill it with some data */
			gtk_tree_store_append(treestore_static[pid],
			                      &toplevel,
			                      NULL);
            static_store(pid, "0");
		}

		return GTK_TREE_MODEL(treestore_static[pid]);
	} else {
		treestore_stats[pid] = gtk_tree_store_new(NUM_COLS, G_TYPE_INT);

		for (i = 0; i < (PKTGEN_GUI_MAX_STATS); i++) {
			/* Append a top level row, and fill it with some data */
			gtk_tree_store_append(treestore_stats[pid],
			                      &toplevel,
			                      NULL);
            stats_store(pid, 0);
		}

		return GTK_TREE_MODEL(treestore_stats[pid]);
	}
}

void
update_ports_static_stat(unsigned int pid)
{
	uint8_t i = 0;
	GtkTreeIter toplevel;
	port_info_t *info = NULL;
	pkt_seq_t *pkt = NULL;
	gchar buf[20];

	info = &pktgen.info[pid];
	pkt  = &info->seq_pkt[SINGLE_PKT];

	gtk_tree_model_get_iter_first(model_static[pid], &toplevel);

	for (i = 0; i < (PKTGEN_GUI_MAX_STATIC); i++) {
		switch (i) {
		case 0:
            /* Tx Count, Rate(%), Packet Size, Tx Burst */
			g_snprintf(buf, sizeof(buf), "%s",
			        (info->fill_pattern_type == ABC_FILL_PATTERN) ? "abcd..." :
			        (info->fill_pattern_type == NO_FILL_PATTERN) ? "None" :
			        (info->fill_pattern_type == ZERO_FILL_PATTERN) ? "Zero" : info-> user_pattern);
			break;

		case 1:
            /* Tx Count, Rate(%), Packet Size, Tx Burst */
			if (rte_atomic64_read(&info->transmit_count) == 0)
				g_snprintf(buf, sizeof(buf), "%s", "Forever");
			else
				g_snprintf(buf, sizeof(buf), "%lu",
				           rte_atomic64_read(&info->transmit_count));
			break;

		case 2:
			g_snprintf(buf, sizeof(buf), "%d", info->tx_rate);
			break;

		case 3:
			g_snprintf(buf, sizeof(buf), "%d", (pkt->pktSize + FCS_SIZE));
			break;

		case 4:
			g_snprintf(buf, sizeof(buf), "%d", info->tx_burst);
			break;

		case 5:
			g_snprintf(buf, sizeof(buf), "%d", pkt->sport);
			break;

		case 6:
			g_snprintf(buf, sizeof(buf), "%d", pkt->dport);
			break;
		case 7:
			g_snprintf( buf, sizeof(buf), "%s",
			        (pkt->ethType == ETHER_TYPE_IPv4) ? "IPv4" :
			        (pkt->ethType == ETHER_TYPE_IPv6) ? "IPv6" :
			        (pkt->ethType == ETHER_TYPE_ARP) ? "ARP" :
			        (pkt->ipProto == PG_IPPROTO_TCP) ? "TCP" :
			        (pkt->ipProto == PG_IPPROTO_ICMP) ? "ICMP" : "UDP");
			break;

		case 8:
			g_snprintf(buf, sizeof(buf), "%d", pkt->vlanid);
			break;

		case 9:
			strcpy(buf, inet_ntop4(buf, sizeof(buf),
			                       htonl(pkt->ip_dst_addr.addr.ipv4.s_addr),
			                       0xFFFFFFFF));
			break;

		case 10:
			strcpy(buf, inet_ntop4(buf, sizeof(buf),
			                       htonl(pkt->ip_src_addr.addr.ipv4.s_addr),
			                       pkt->ip_mask));
			break;

		case 11:
			strcpy(buf, inet_mtoa(buf, sizeof(buf), &pkt->eth_dst_addr));
			break;

		case 12:
			strcpy(buf, inet_mtoa(buf, sizeof(buf), &pkt->eth_src_addr));
			break;

		default:
            strcpy(buf, "0");
            break;
		}
		static_store(pid, buf);

		if (!gtk_tree_model_iter_next(model_static[pid], &toplevel))
			break;
	}
}

int
update_ports_stat(void *arg)
{
	GtkWidget *window = (GtkWidget *)arg;
	unsigned int pid = 0;
	port_info_t *info = NULL;

	GtkTreeIter toplevel;
	rxtx_t cnt;
	eth_stats_t tot_stats = {0};

	if (gui_created == FALSE)
        return TRUE;

    for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
        cnt.rxtx = wr_get_map(pktgen.l2p, pid, RTE_MAX_LCORE);
        if (cnt.rxtx == 0)
            continue;

        info = &pktgen.info[pid];

        tot_stats.ipackets  += info->rate_stats.ipackets;
        tot_stats.opackets  += info->rate_stats.opackets;
        tot_stats.ibytes    += info->rate_stats.ibytes;
        tot_stats.obytes    += info->rate_stats.obytes;
        tot_stats.ierrors   += info->rate_stats.ierrors;
        tot_stats.oerrors   += info->rate_stats.oerrors;

        tot_stats.imissed   += info->rate_stats.imissed;
#if RTE_VERSION < RTE_VERSION_NUM(2, 2, 0, 0)
        tot_stats.ibadcrc   += info->rate_stats.ibadcrc;
        tot_stats.ibadlen   += info->rate_stats.ibadlen;
#endif
#if RTE_VERSION < RTE_VERSION_NUM(16, 4, 0, 0)
        tot_stats.imcasts   += info->rate_stats.imcasts;
#endif
        tot_stats.rx_nombuf += info->rate_stats.rx_nombuf;

        gtk_tree_model_get_iter_first(model_stats[pid], &toplevel);

        stats_store_next(pid, info->rate_stats.ipackets);
        stats_store_next(pid, info->rate_stats.opackets);
        stats_store_next(pid, iBitsTotal(info->rate_stats) / Million);
        stats_store_next(pid, oBitsTotal(info->rate_stats) / Million);

        /* Packets Sizes */
        stats_store_next(pid, info->sizes.broadcast);
        stats_store_next(pid, info->sizes.multicast);
        stats_store_next(pid, info->sizes._64);
        stats_store_next(pid, info->sizes._65_127);
        stats_store_next(pid, info->sizes._128_255);
        stats_store_next(pid, info->sizes._256_511);
        stats_store_next(pid, info->sizes._512_1023);
        stats_store_next(pid, info->sizes._1024_1518);

        /* Runt & Jumbo pkts */
        stats_store_next(pid, info->sizes.runt);
        stats_store_next(pid, info->sizes.jumbo);

        /* Rx/Tx Errors */
        stats_store_next(pid, info->port_stats.ierrors);
        stats_store_next(pid, info->port_stats.oerrors);

        /* Total Rx/Tx  packets */
        stats_store_next(pid, info->port_stats.ipackets);
        stats_store_next(pid, info->port_stats.opackets);

        /* Total Rx/Tx mbits */
        stats_store_next(pid, iBitsTotal(info->port_stats) / Million);
        stats_store_next(pid, oBitsTotal(info->port_stats) / Million);

        /* ARP & ICMP Pkts */
        stats_store_next(pid, info->stats.arp_pkts);
        stats_store_next(pid, info->stats.echo_pkts);

        prev_stats_val[pid].__broadcast = info->sizes.broadcast;
        prev_stats_val[pid].__multicast = info->sizes.multicast;
        prev_stats_val[pid].__64        = info->sizes._64;
        prev_stats_val[pid].__65_127    = info->sizes._65_127;
        prev_stats_val[pid].__128_255   = info->sizes._128_255;
        prev_stats_val[pid].__256_511   = info->sizes._256_511;
        prev_stats_val[pid].__512_1023  = info->sizes._512_1023;
        prev_stats_val[pid].__1024_1518 = info->sizes._1024_1518;
        prev_stats_val[pid].__ierrors   = info->port_stats.ierrors;
        prev_stats_val[pid].__oerrors   = info->port_stats.oerrors;
    }

    gtk_tree_model_get_iter_first(model_stats[pktgen.ending_port], &toplevel);

    stats_store_next(pktgen.ending_port, tot_stats.ipackets);
    stats_store_next(pktgen.ending_port, tot_stats.opackets);

    stats_store_next(pktgen.ending_port, iBitsTotal(tot_stats) / Million);
    stats_store_next(pktgen.ending_port, oBitsTotal(tot_stats) / Million);

    gtk_widget_queue_draw(GTK_WIDGET(window));
    gtk_widget_show_all(window);

    return TRUE;
}

void
start_stop_traffic(GtkTreeModel  *model,
                   GtkTreePath __attribute__((unused)) *path,
                   GtkTreeIter   *iter,
                   gpointer userdata)
{
	gchar *name;
	port_info_t *info = NULL;
	unsigned int pid;
	guint *flag = (guint *)userdata;

	gtk_tree_model_get(model, iter, COL_CHASSIS_PORTS, &name, -1);

	if (0 != g_strcmp0(name, "[127.0.0.1]")) {
		int offset = strlen(name);
		pid = atoi((name + offset) - 1);
		info = &pktgen.info[pid];
		if ((info != NULL) && (*flag == 1))
			pktgen_start_transmitting(info);
		else if ((info != NULL) && (*flag == 2))
			pktgen_stop_transmitting(info);
	} else {
		if (*flag == 1)
			forall_ports(pktgen_start_transmitting(info));
		else
			forall_ports(pktgen_stop_transmitting(info));
	}
}

void
about_dialog(void)
{
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("gui/icons/logo.png", NULL);

	GtkWidget *about_dialog = gtk_about_dialog_new();

	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about_dialog), PKTGEN_APP_NAME);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), PKTGEN_VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), COPYRIGHT_MSG);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), POWERED_BY_DPDK);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog), "http://dpdk.org");
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_dialog), intel_copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dialog), pixbuf);

	g_object_unref(pixbuf), pixbuf = NULL;
	gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy(about_dialog);

	if (about_dialog) {
		gtk_window_present(GTK_WINDOW(about_dialog));
		return;
	}

	g_signal_connect(GTK_DIALOG(about_dialog), "destroy",
	                 G_CALLBACK(gtk_widget_destroyed), &about_dialog);
}

void
traffic_start_callback(GtkWidget __attribute__(
                               (unused)) *w, gpointer __attribute__(
                               (unused)) data)
{
    guint flag = 1;
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    start_stop_traffic,
	                                    (gpointer) & flag);
}

void
traffic_stop_callback(GtkWidget __attribute__(
                              (unused)) *w, gpointer __attribute__(
                              (unused)) data)
{
	guint flag = 2;
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    start_stop_traffic,
	                                    (gpointer) & flag);
}

void
start_stop_capture(GtkTreeModel  *model,
                   GtkTreePath __attribute__((unused)) *path,
                   GtkTreeIter   *iter,
                   gpointer userdata)
{
	gchar *name;
	port_info_t *info = NULL;
	unsigned int pid;
	guint *flag = (guint *)userdata;

	gtk_tree_model_get(model, iter, COL_CHASSIS_PORTS, &name, -1);

	if (0 != g_strcmp0(name, "[127.0.0.1]")) {
		int offset = strlen(name);
		pid = atoi((name + offset) - 1);
		info = &pktgen.info[pid];
		if ((info != NULL) && (*flag == 1))
			pktgen_set_capture(info, ENABLE_STATE);
		else if ((info != NULL) && (*flag == 2))
			pktgen_set_capture(info, DISABLE_STATE);
	}
}

void
capture_start_callback(GtkWidget __attribute__(
                               (unused)) *w, gpointer __attribute__(
                               (unused)) data)
{
	guint flag = 1;
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    start_stop_capture,
	                                    (gpointer) & flag);
}

void
capture_stop_callback(GtkWidget __attribute__(
                              (unused)) *w, gpointer __attribute__(
                              (unused)) data)
{
	guint flag = 2;
	GtkTreeSelection  *selection = gtk_tree_view_get_selection(
	                GTK_TREE_VIEW(chassis_view));

	gtk_tree_selection_selected_foreach(selection,
	                                    start_stop_capture,
	                                    (gpointer) & flag);
}

void
about_callback(GtkWidget __attribute__(
                       (unused)) *w, gpointer __attribute__(
                       (unused)) data)
{
	about_dialog();
}

GtkWidget *
chassis_tree_view(void)
{
	GtkTreeViewColumn   *col;
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model;

	GtkTreeSelection    *chassis_selection;

	chassis_view = gtk_tree_view_new();

	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(col, "Chassis");
	gtk_tree_view_column_set_expand(col, TRUE);

	/* pack tree view column into tree view */
	gtk_tree_view_append_column(GTK_TREE_VIEW(chassis_view), col);

	renderer = gtk_cell_renderer_text_new();

	/* pack cell renderer into tree view column */
	gtk_tree_view_column_pack_start(col, renderer, TRUE);

	/* connect 'text' property of the cell renderer to
	 *  model column that contains the first name */
	gtk_tree_view_column_add_attribute(col, renderer, "text", COL_CHASSIS_PORTS);

	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(chassis_view), GTK_TREE_VIEW_GRID_LINES_BOTH);

	model = fill_chassis_info();
	gtk_tree_view_set_model(GTK_TREE_VIEW(chassis_view), model);
	g_object_unref(model);

	gtk_tree_view_expand_all(GTK_TREE_VIEW(chassis_view));

	chassis_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(chassis_view));
	gtk_tree_selection_set_mode(chassis_selection, GTK_SELECTION_MULTIPLE);

	return chassis_view;
}

GtkWidget *
port_tree_view(unsigned int port_id, const char *title, gboolean is_static)
{
	GtkTreeViewColumn   *col_stats;
	GtkCellRenderer     *renderer_stats;
	char str_port[15];

	if (port_id != (unsigned int)(pktgen.ending_port))
		sprintf(str_port, "%s%d", title, port_id);
	else
		sprintf(str_port, "%s", title);

	if (is_static == TRUE) {
		view_static[port_id] = gtk_tree_view_new();
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view_static[port_id]), TRUE);
	} else {
		view_stats[port_id] = gtk_tree_view_new();
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view_stats[port_id]), TRUE);
	}

	col_stats = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(col_stats, str_port);

	gtk_tree_view_column_set_alignment(col_stats, 0.5);
	gtk_tree_view_column_set_min_width(col_stats, 30);

	gtk_tree_view_column_set_expand(col_stats, TRUE);

	if (is_static == TRUE)
		/* pack tree view column into tree view */
		gtk_tree_view_append_column(GTK_TREE_VIEW(view_static[port_id]),col_stats);
	else
		/* pack tree view column into tree view */
		gtk_tree_view_append_column(GTK_TREE_VIEW(view_stats[port_id]),col_stats);

	renderer_stats = gtk_cell_renderer_text_new();

	/* pack cell renderer into tree view column */
	gtk_tree_view_column_pack_start(col_stats, renderer_stats, TRUE);

	/* connect 'text' property of the cell renderer to
	 *  model column that contains the first name */
	gtk_tree_view_column_add_attribute(col_stats, renderer_stats, "text", COL_CHASSIS_PORTS);

	if (is_static == TRUE) {
		gtk_tree_view_set_grid_lines(GTK_TREE_VIEW( view_static[port_id]), GTK_TREE_VIEW_GRID_LINES_BOTH);
		model_static[port_id] = fill_port_info(port_id, TRUE);
		gtk_tree_view_set_model(GTK_TREE_VIEW(view_static[port_id]), model_static[port_id]);
		return view_static[port_id];
	} else {
		gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(view_stats[port_id]), GTK_TREE_VIEW_GRID_LINES_BOTH);
		model_stats[port_id] = fill_port_info(port_id, FALSE);
		gtk_tree_view_set_model(GTK_TREE_VIEW(view_stats[port_id]), model_stats[port_id]);
		return view_stats[port_id];
	}
}

/* Create a Button Box with the specified parameters */
static GtkWidget *
button_box(GtkWidget __attribute__(
                   (unused))  *view, const char *title, gint layout)
{
	GtkWidget *frame;
	GtkWidget *bbox;

	GtkWidget *buttonImageStr, *buttonImageStp, *buttonImageCapStr,
	*buttonImageCapStp, *buttonImageAbt;

	GtkSettings *default_settings = gtk_settings_get_default();

	g_object_set(default_settings, "gtk-button-images", TRUE, NULL);

	buttonImageStr = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(buttonImageStr), "gui/icons/traffic_start.png");

	traffic_start_button = gtk_button_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(traffic_start_button), "Start Traffic");
	gtk_button_set_relief((GtkButton *)traffic_start_button, GTK_RELIEF_NONE);
	gtk_button_set_image((GtkButton *)traffic_start_button, buttonImageStr);
	gtk_button_set_image_position((GtkButton *)traffic_start_button, GTK_POS_RIGHT);

	frame = gtk_frame_new(title);
	bbox = gtk_vbutton_box_new();

	gtk_container_add(GTK_CONTAINER(frame), bbox);

	gtk_container_set_border_width(GTK_CONTAINER(traffic_start_button), 5);
	gtk_container_add(GTK_CONTAINER(bbox), traffic_start_button);

	g_signal_connect(GTK_OBJECT(traffic_start_button), "clicked",
	                 GTK_SIGNAL_FUNC(traffic_start_callback), NULL);

	traffic_stop_button = gtk_button_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(traffic_stop_button), "Stop Traffic");

	buttonImageStp = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(buttonImageStp), "gui/icons/traffic_stop.png");
	gtk_button_set_relief((GtkButton *)traffic_stop_button, GTK_RELIEF_NONE);
	gtk_button_set_image((GtkButton *)traffic_stop_button, buttonImageStp);
	gtk_button_set_image_position((GtkButton *)traffic_stop_button, GTK_POS_RIGHT);

	gtk_container_set_border_width(GTK_CONTAINER(traffic_stop_button), 5);
	gtk_container_add(GTK_CONTAINER(bbox), traffic_stop_button);

	g_signal_connect(GTK_OBJECT(traffic_stop_button), "clicked",
	                 GTK_SIGNAL_FUNC(traffic_stop_callback), NULL);

	capture_start_button = gtk_button_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(capture_start_button), "Start Capture");

	buttonImageCapStr = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(buttonImageCapStr),
	                        "gui/icons/capture_start.png");
	gtk_button_set_relief((GtkButton *)capture_start_button, GTK_RELIEF_NONE);
	gtk_button_set_image((GtkButton *)capture_start_button, buttonImageCapStr);
	gtk_button_set_image_position((GtkButton *)capture_start_button, GTK_POS_LEFT);

	gtk_container_set_border_width(GTK_CONTAINER(capture_start_button), 5);
	gtk_container_add(GTK_CONTAINER(bbox), capture_start_button);

	g_signal_connect(GTK_OBJECT(capture_start_button), "clicked",
	                 GTK_SIGNAL_FUNC(capture_start_callback), NULL);

	capture_stop_button = gtk_button_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(capture_stop_button), "Stop Capture");

	buttonImageCapStp = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(buttonImageCapStp),
	                        "gui/icons/capture_stop.png");
	gtk_button_set_relief((GtkButton *)capture_stop_button, GTK_RELIEF_NONE);
	gtk_button_set_image((GtkButton *)capture_stop_button, buttonImageCapStp);
	gtk_button_set_image_position((GtkButton *)capture_stop_button, GTK_POS_RIGHT);

	gtk_container_set_border_width(GTK_CONTAINER(capture_stop_button), 5);
	gtk_container_add(GTK_CONTAINER(bbox), capture_stop_button);

	g_signal_connect(GTK_OBJECT(capture_stop_button), "clicked",
	                 GTK_SIGNAL_FUNC(capture_stop_callback), NULL);

	about_button = gtk_button_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(about_button), "About");

	buttonImageAbt = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(buttonImageAbt), "gui/icons/about.png");
	gtk_button_set_relief((GtkButton *)about_button, GTK_RELIEF_NONE);
	gtk_button_set_image((GtkButton *)about_button, buttonImageAbt);
	gtk_button_set_image_position((GtkButton *)about_button, GTK_POS_RIGHT);

	gtk_container_set_border_width(GTK_CONTAINER(about_button), 5);
	gtk_container_add(GTK_CONTAINER(bbox), about_button);

	g_signal_connect(GTK_OBJECT(about_button), "clicked",
	                 GTK_SIGNAL_FUNC(about_callback), NULL);

	/* Set the appearance of the Button Box */
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), layout);

	return frame;
}

void
console_callback(GtkWidget *widget, GtkWidget *entry)
{
	GtkTextMark *mark;
	GtkAdjustment *vadj;
	const gchar *entry_text = NULL;
	const gchar *cmd_prompt = "Pktgen> ";

	entry_text = gtk_entry_get_text(GTK_ENTRY(widget));

	mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_buffer_get_iter_at_mark(buffer, &buffer_iter, mark);

	/* Insert newline (only if there's already text in the buffer). */
	if (gtk_text_buffer_get_char_count(buffer))
		gtk_text_buffer_insert(buffer, &buffer_iter, "\n", 1);

	/* Set the default buffer text. */
	gtk_text_buffer_insert(buffer, &buffer_iter,
	                       g_strconcat(cmd_prompt, entry_text, NULL), -1);

	cmdline_in(pktgen.cl, entry_text, strlen(entry_text));
	cmdline_in(pktgen.cl, "\r", 1);

	/* scroll to end iter */
	vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(entry));
	gtk_adjustment_set_value(vadj, vadj->upper);
	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(entry), vadj);

	gtk_entry_set_text(GTK_ENTRY(widget), "");
}

GtkWidget *
create_chassis(void)
{
	GtkWidget *frame;

	frame = gtk_frame_new(NULL);

	gtk_widget_set_size_request(frame, 300, 200);

	chassis_view = chassis_tree_view();

	gtk_container_add(GTK_CONTAINER(frame), chassis_view);

	return frame;
}

GtkWidget *
pktgen_console_box(const char *title)
{
	GtkWidget*textArea = gtk_text_view_new();
	GtkWidget*scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget*textEntry = gtk_entry_new();
	GtkWidget*console = gtk_table_new(3, 1, FALSE);

	GtkWidget *frame = gtk_frame_new(title);

	gtk_widget_set_size_request(frame, 300, 100);
	/* Obtaining the buffer associated with the widget. */
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textArea));

	gtk_text_view_set_editable(GTK_TEXT_VIEW(textArea), FALSE);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	gtk_container_add(GTK_CONTAINER(scrolledwindow), textArea);
	gtk_table_set_homogeneous(GTK_TABLE(console), FALSE);

	gtk_table_attach_defaults(GTK_TABLE(console), scrolledwindow, 0, 1, 0, 1);
	gtk_table_attach(GTK_TABLE(console), textEntry, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

	gtk_text_buffer_get_iter_at_offset(buffer, &buffer_iter, 0);
	gtk_entry_set_max_length(GTK_ENTRY(textEntry), 250);
	g_signal_connect(textEntry, "activate",
	                 G_CALLBACK(console_callback),
	                 scrolledwindow);

	gtk_container_add(GTK_CONTAINER(frame), console);

	return frame;
}

GtkTreeModel *
pktgen_stats_header_fill(gboolean is_static)
{
	GtkTreeStore  *stats_label_treestore;
	GtkTreeIter toplevel;
	gint i;

	stats_label_treestore = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING);

	if (is_static == TRUE)
		for (i = 0; i < PKTGEN_GUI_MAX_STATIC; i++) {
			/* Append a top level row, and fill it with some data */
			gtk_tree_store_append(stats_label_treestore, &toplevel, NULL);
			gtk_tree_store_set(stats_label_treestore,
			                   &toplevel, COL_CHASSIS_PORTS,
			                   pktgen_static_fields[i], -1);
		}

	else
		for (i = 0; i < PKTGEN_GUI_MAX_STATS; i++) {
			/* Append a top level row, and fill it with some data */
			gtk_tree_store_append(stats_label_treestore, &toplevel, NULL);
			gtk_tree_store_set(stats_label_treestore,
			                   &toplevel, COL_CHASSIS_PORTS,
			                   pktgen_stats_fields[i], -1);
		}

	return GTK_TREE_MODEL(stats_label_treestore);
}

GtkWidget *
pktgen_stats_header_tree_view(gboolean is_static)
{
	GtkTreeViewColumn   *stats_label_col;
	GtkCellRenderer     *stats_label_renderer;
	GtkWidget           *stats_label_view;
	GtkTreeModel        *stats_label_model;

	GtkTreeSelection    *stats_label_selection;

	char str_port[15] = "Port#";

	stats_label_view = gtk_tree_view_new();

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(stats_label_view), TRUE);

	stats_label_col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(stats_label_col, str_port);

	gtk_tree_view_column_set_alignment(stats_label_col, 0.5);

	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(stats_label_col),
	                                GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_min_width(stats_label_col, 200);
	gtk_tree_view_column_set_max_width(stats_label_col, 200);

	gtk_tree_view_column_set_fixed_width(stats_label_col, 200);
	gtk_tree_view_append_column(GTK_TREE_VIEW(stats_label_view), stats_label_col);

	stats_label_renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_column_pack_start(stats_label_col,
	                                stats_label_renderer,
	                                TRUE);

	gtk_tree_view_column_add_attribute(stats_label_col,
	                                   stats_label_renderer,
	                                   "text",
	                                   COL_CHASSIS_PORTS);

	/* set 'weight' property of the cell renderer to
	 *  bold print */
	g_object_set(stats_label_renderer,
	             "weight", PANGO_WEIGHT_BOLD,
	             "weight-set", TRUE,
	             NULL);

	/* set 'cell-background' property of the cell renderer to light-grey */
	g_object_set(stats_label_renderer,
	             "cell-background", "#D3D3D3",
	             "cell-background-set", TRUE,
	             NULL);

	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(stats_label_view),
	                             GTK_TREE_VIEW_GRID_LINES_BOTH);

	stats_label_model = pktgen_stats_header_fill(is_static);

	gtk_tree_view_set_model(GTK_TREE_VIEW(stats_label_view),
	                        stats_label_model);
	g_object_unref(stats_label_model);

	stats_label_selection =
	        gtk_tree_view_get_selection(GTK_TREE_VIEW(stats_label_view));
	gtk_tree_selection_set_mode(stats_label_selection, GTK_SELECTION_NONE);

	return stats_label_view;
}

GtkWidget *
pktgen_show_static_conf(void)
{
	GtkWidget *hbox_stats;
	GtkWidget *frame_horz_conf;
	GtkWidget *scrolled_window;
	GtkWidget *table;
	uint32_t pid;
	rxtx_t cnt;

	/* create a new scrolled window. */
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);

	gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	frame_horz_conf = gtk_frame_new("Static configuration");
	gtk_widget_set_size_request(frame_horz_conf, -1, 200);

	hbox_stats = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox_stats), 10);

	gtk_box_pack_start(GTK_BOX(hbox_stats), scrolled_window, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame_horz_conf), hbox_stats);

	/* create a table of 1 by 10 squares. */
	table = gtk_table_new(1, (pktgen.ending_port + 1), FALSE);

	/* set the spacing to 5 on x and 5 on y */
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), table);
	gtk_widget_show(table);

	/* Create a column of statistics label on the first column of table */
	gtk_table_attach_defaults(GTK_TABLE(
	                                  table),
	                          pktgen_stats_header_tree_view(
	                                  TRUE), 0, 1, 0, 1);

	/* Create columns of statistics for selected ports on the table */
	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		cnt.rxtx = wr_get_map(pktgen.l2p, pid, RTE_MAX_LCORE);
		if (cnt.rxtx == 0)
			continue;

		gtk_table_attach_defaults(GTK_TABLE(table),
		                          port_tree_view(pid, "Port", TRUE),
		                          (pid + 1), (pid + 2), 0, 1);
		update_ports_static_stat(pid);
	}

	return frame_horz_conf;
}

GtkWidget *
pktgen_show_statistic_data(void)
{
	GtkWidget *frame_horz_stats;
	GtkWidget *scrolled_window;
	GtkWidget *table;
	GtkWidget *hbox_stats;
	uint32_t pid;
	rxtx_t cnt;

	/* create a new scrolled window. */
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);

	gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	frame_horz_stats = gtk_frame_new("Statistics");
	gtk_widget_set_size_request(frame_horz_stats, -1, 400);

	hbox_stats = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox_stats), 10);

	gtk_box_pack_start(GTK_BOX(hbox_stats), scrolled_window, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame_horz_stats), hbox_stats);

	/* create a table of 1 by 10 squares. */
	table = gtk_table_new(1, (pktgen.ending_port + 1), FALSE);

	/* set the spacing to 5 on x and 5 on y */
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), table);
	gtk_widget_show(table);

	/* Create a column of statistics label on the first column of table */
	gtk_table_attach_defaults(GTK_TABLE(table),
	                          pktgen_stats_header_tree_view(FALSE), 0, 1, 0, 1);

	/* Create columns of statistics for selected ports on the table */
	for (pid = 0; pid < RTE_MAX_ETHPORTS; pid++) {
		cnt.rxtx = wr_get_map(pktgen.l2p, pid, RTE_MAX_LCORE);
		if (cnt.rxtx == 0)
			continue;

		gtk_table_attach_defaults(GTK_TABLE(table),
		                          port_tree_view(pid, "Port", FALSE),
		                          (pid + 1), (pid + 2), 0, 1);
	}
    /* Create a column of total statistics on the last column of table */
    gtk_table_attach_defaults(GTK_TABLE(table),
                              port_tree_view(pktgen.ending_port,
                                             "Total Rate", FALSE),
                              (pktgen.ending_port + 1),
                              (pktgen.ending_port + 2), 0, 1);
	return frame_horz_stats;
}

void
pktgen_gui_close(void)
{
	gui_created = FALSE;
	gtk_main_quit();
}

void
pktgen_start_gui(void)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *frame;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(pktgen_gui_close), NULL);

	gtk_window_set_title(GTK_WINDOW(window), PKTGEN_GUI_APP_NAME);

	gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	gtk_container_set_border_width(GTK_CONTAINER(window), 10);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	frame = gtk_frame_new("Configuration");
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 10);

	hbox = gtk_hbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), create_chassis(), FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox),
	                   button_box(chassis_view, "Traffic", GTK_BUTTONBOX_CENTER),
                       FALSE, FALSE, 5);

	gtk_box_pack_start(GTK_BOX(hbox), pktgen_stream_box(), FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox),
	                   pktgen_console_box("Console"), TRUE, TRUE, 5);

	gtk_box_pack_start(GTK_BOX(vbox),
	                   pktgen_show_static_conf(), TRUE, TRUE, 10);

	gtk_box_pack_start(GTK_BOX(vbox),
	                   pktgen_show_statistic_data(), TRUE, TRUE, 10);

    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	gtk_widget_show_all(window);

	gui_created = TRUE;

	g_timeout_add_seconds(3, (GSourceFunc)update_ports_stat, window);

	/* Enter the event loop */
	gtk_main();
}

void
pktgen_gui_main(int argc, char *argv[])
{
	int rc;
	pthread_t inc_x_thread;

	/* Initialize GTK */
	gtk_init(&argc, &argv);

	/* create a second thread which executes inc_x(&x) */
	rc = pthread_create(&inc_x_thread, NULL,
	                    (void *)&pktgen_start_gui, NULL);
	if (rc) {
		printf("Error creating GUI thread\n");
		return;
	}
}
